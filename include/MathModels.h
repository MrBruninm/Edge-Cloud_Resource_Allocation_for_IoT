#pragma once

#include "NetworkResourceAllocation.h"

#include <ilcplex/ilocplex.h>

typedef IloArray<IloNumVarArray> NumVar2D;
typedef IloArray<NumVar2D>       NumVar3D;

namespace MathModels {
    namespace {
        /**
         * @brief Solves the resource allocation problem as an Integer Linear Programming (ILP) model using CPLEX.
         * @details This function formulates and solves the optimization model.
         * The objective is to minimize the total cost, defined as the sum of activation
         * costs for used servers and penalty costs for devices that are covered but not
         * served. It defines three primary binary decision variables:
         * - w_d: 1 if device d is NOT served, 0 otherwise.
         * - x_i^d: 1 if device d is allocated to server i, 0 otherwise.
         * - z_i: 1 if server i is active, 0 otherwise.
         * The model includes constraints for device assignment uniqueness, server resource
         * capacities (BW, MEM, PCN, PCC, STO), and linking device assignments to server
         * activation. The function also configures the solver, exports the model to a
         * .lp file for analysis, and saves a solver log.
         *
         * @param[in,out] state A reference to the Result object. It provides the initial
         * problem data and is updated in-place with the optimal allocation found by the solver.
         * @param[in,out] metrics A reference to the MathMetrics object to be populated with
         * solver results, including objective value, MIP gap, and execution time.
         * @note This function is intended for internal use within the MathModels namespace.
         * @exception IloException Catches and reports CPLEX-specific errors.
         */
        inline void minimizeCost(Result& state, MathMetrics& metrics) {
            Devices& devices = state.devices;
            Servers& servers = state.servers;
            const iVec& coveredDevicesIdx = state.coveredDevicesIdx;

            IloEnv env;
            try {
                IloModel model(env);

                //=========================================================================
                // 1. VARIABLE DECLARATION
                //=========================================================================

                // w^{d}: 1 if device d is NOT served, 0 otherwise.
                IloNumVarArray w(env, devices.size(), 0, 1, ILOBOOL);
                for (size_t d_idx = 1; d_idx < devices.size(); ++d_idx) {
                    std::string name = "w_d(" + std::to_string(d_idx) + ")";
                    w[d_idx].setName(name.c_str());
                }

                // x_{i}^{d}: 1 if device d is allocated to server i, 0 otherwise.
                NumVar2D x(env, servers.size());
                for (size_t i = 1; i < servers.size(); ++i) {
                    x[i] = IloNumVarArray(env, devices.size(), 0, 1, ILOBOOL);
                    for (size_t d_idx = 1; d_idx < devices.size(); ++d_idx) {
                        std::string name = "x_s(" + std::to_string(i) + ")_d(" + std::to_string(d_idx) + ")";
                        x[i][d_idx].setName(name.c_str());
                    }
                }

                // z_i: 1 if server i is active, 0 otherwise.
                IloNumVarArray z(env, servers.size(), 0, 1, ILOBOOL);
                for (size_t i = 1; i < servers.size(); ++i) {
                    std::string name = "z_s(" + std::to_string(i) + ")";
                    z[i].setName(name.c_str());
                }

                //=========================================================================
                // 2. OBJECTIVE FUNCTION
                // Minimize total cost: server activation costs + non-service penalties.
                //=========================================================================

                IloExpr obj(env);
                for (size_t i = 1; i < servers.size(); ++i) {
                    obj += servers[i].csc * z[i];
                }
                for (int d_idx : coveredDevicesIdx) {
                    obj += devices[d_idx].cnd * w[d_idx];
                }
                model.add(IloMinimize(env, obj));
                obj.end();

                //=========================================================================
                // 3. CONSTRAINTS
                //=========================================================================

                for (int d_idx : coveredDevicesIdx) {
                    Device& device = devices[d_idx];
                    // Constraint (1): Each device is served by at most one server.
                    IloExpr c1(env);
                    for (const auto& s_info : device.servers) {
                        c1 += x[s_info.id][d_idx];
                    }
                    model.add(c1 == 1 - w[d_idx]);
                    c1.end();
                    
                    // Link x and z: A device can only be assigned to an active server (z_i=1).
                    for (const auto& s_info : device.servers) {
                        model.add(x[s_info.id][d_idx] <= z[s_info.id]);
                    }
                    
                }

                // Constraints (2-6): Server resource capacity limits.
                for (size_t i = 1; i < servers.size(); ++i) {
                    IloExpr c2_bw(env), c3_mem(env), c4_pcn(env), c5_pcc(env), c6_sto(env);
                    for (int d_idx : coveredDevicesIdx) {
                        // Check if server 'i' is a potential server for device 'd_idx'
                        bool is_potential = false;
                        for (const auto& s_info : devices[d_idx].servers) {
                            if (s_info.id == i) {
                                is_potential = true;
                                break;
                            }
                        }
                        if (is_potential) {
                            c2_bw  += devices[d_idx].bw  * x[i][d_idx]; // Bandwidth
                            c3_mem += devices[d_idx].mem * x[i][d_idx]; // Memory
                            c4_pcn += devices[d_idx].pcn * x[i][d_idx]; // Num. Cores
                            c5_pcc += devices[d_idx].pcc * x[i][d_idx]; // Proc. Capacity
                            c6_sto += devices[d_idx].sto * x[i][d_idx]; // Storage
                        }
                    }
                    model.add(c2_bw  <= z[i] * servers[i].bw); 
                    model.add(c3_mem <= z[i] * servers[i].mem); 
                    model.add(c4_pcn <= z[i] * servers[i].pcn); 
                    model.add(c5_pcc <= z[i] * servers[i].pcc_total); 
                    model.add(c6_sto <= z[i] * servers[i].sto); 
                    c2_bw.end(); c3_mem.end(); c4_pcn.end(); c5_pcc.end(); c6_sto.end();
                }

                //=========================================================================
                // 4. SOLVER CONFIGURATION AND EXECUTION
                //=========================================================================

                IloCplex cplex(model);
                cplex.setParam(IloCplex::Param::Threads, 1);
                cplex.setParam(IloCplex::Param::TimeLimit, 1200);
                //cplex.setParam(IloCplex::Param::MIP::Tolerances::Integrality, 1e-9);

                std::filesystem::path baseDir = metrics.getBaseDirectoryPath();
                std::string baseName = metrics.getBaseFileName();
                std::filesystem::path logDir = baseDir / "logs";
                std::filesystem::path modelDir = baseDir / "models";
                std::filesystem::create_directories(logDir);
                std::filesystem::create_directories(modelDir);
                std::filesystem::path logPath = logDir / (baseName + ".log");
                std::filesystem::path modelPath = modelDir / (baseName + ".lp");

                std::ofstream logFile(logPath);
                if (logFile.is_open()) {
                    cplex.setOut(logFile);
                } else {
                    cplex.setOut(env.getNullStream());
                }

                cplex.exportModel(modelPath.c_str());

                auto startChrono = std::chrono::high_resolution_clock::now();
                cplex.solve();
                auto endChrono = std::chrono::high_resolution_clock::now();
                metrics.outputs.execution_time_sec = std::chrono::duration<double>(endChrono - startChrono).count();

                std::stringstream status;
                status << cplex.getStatus();
                metrics.status = status.str();
                metrics.OF = (double)cplex.getObjValue() + metrics.outputs.cost_of_non_coverage;
                metrics.gap = cplex.getMIPRelativeGap();

                logFile.close();

                //=========================================================================
                // 5. PARSE RESULTS
                //=========================================================================

                for (int d_idx : coveredDevicesIdx) {
                    if (cplex.getValue(w[d_idx]) < 0.5) {
                        for (const auto& s_info : devices[d_idx].servers) {
                            if (cplex.getValue(x[s_info.id][d_idx]) > 0.5) {
                                servers[s_info.id].addServed(devices[d_idx]);
                                devices[d_idx].server = s_info;
                                break; // Move to the next device
                            }
                        }
                    }
                }

                NetworkResourceAllocation::calculateMetrics(devices, servers, metrics);

            } catch (const IloException& e) {
                std::cerr << "CPLEX Error: " << e.getMessage() << std::endl;
            } catch (...) {
                std::cerr << "An unknown error occurred in the CPLEX model." << std::endl;
            }
            env.end();
        }
    }

    /**
     * @brief Serves as the main entry point for running a mathematical optimization model.
     * @details This function orchestrates the execution of a specific mathematical model.
     * It creates a dedicated `MathMetrics` object to store results, calls the
     * appropriate solver function (e.g., `minimizeCost`) based on the algorithm name,
     * and, upon completion, triggers the display and saving of the final metrics.
     *
     * @param[in] algorithm The name of the mathematical model to execute (e.g., "Minimize_Cost").
     * @param[in,out] state The `Result` object containing the initial simulation state.
     * This object will be updated by the solver with the optimal solution.
     */
    inline void bootup(const std::string& algorithm, Result& state) {
        auto metrics = std::make_unique<MathMetrics>("Mathematical", algorithm, state.metrics);
        
        if (algorithm == "Minimize_Cost") {
            minimizeCost(state, *metrics);
        } else {
            std::cerr << "Error: Unknown mathematical model algorithm type." << std::endl;
            return;
        }

        if (state.metrics) {
            showStructs::showMetrics(*metrics);
            metrics->saveResultsToFile();
        }
    }
}