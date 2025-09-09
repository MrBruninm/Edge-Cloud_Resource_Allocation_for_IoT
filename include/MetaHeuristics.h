#pragma once

#include "NetworkResourceAllocation.h"

namespace MetaHeuristics {
    namespace {
        /**
         * @brief Generates a neighbor solution by attempting to move one device to a different server.
         * @details This function defines the neighborhood structure for the search. It randomly
         * selects a covered device and tries to reallocate it to a different potential
         * server. If a valid move is found (i.e., the new server has capacity), the
         * solution's state (devices, servers) is updated directly. The function also
         * updates the cost components (`cns`, `csu`) by reference to reflect the move's
         * impact, such as activating a new server or deactivating an old one.
         *
         * @param[in,out] devices The device vector of the solution to modify.
         * @param[in,out] servers The server vector of the solution to modify.
         * @param[in] coveredDevicesIdx A vector of indices for all covered devices.
         * @param[in,out] cns Reference to the current solution's cost of non-service, updated on change.
         * @param[in,out] csu Reference to the current solution's cost of servers used, updated on change.
         */
        inline void generateNeighbor(Devices& devices, Servers& servers, const iVec& coveredDevicesIdx, double& cns, double& csu) {
            int idx = utils::randomNumber(0, (int) coveredDevicesIdx.size() - 1);
            Device& device = devices.at(coveredDevicesIdx.at(idx));
            
            iVec serversShuffled = utils::shuffledRange(0, (int) device.servers.size() - 1);
            
            int tries = 0;
            for (auto idxServers : serversShuffled) {
                if (tries++ >= 5) break;
                auto& potential_server = device.servers.at(idxServers);

                if (potential_server.id == device.server.id) continue;

                Server& old_server = servers.at(device.server.id);
                Server& new_server = servers.at(potential_server.id);

                if (new_server.canServe(device)) {
                    if (!new_server.on) {
                        csu += new_server.csc;
                    }

                    if (device.served) {
                        old_server.rmvServed(device);
                        if (!old_server.on) {
                            csu -= old_server.csc;
                        }
                    } else {
                        cns -= device.cnd;
                    }

                    new_server.addServed(device);
                    device.server = potential_server;
                    return;
                }
            }
        }
    
        /**
         * @brief Performs resource allocation using the Simulated Annealing (SA) meta-heuristic.
         * @details This function implements the SA algorithm to find a near-optimal allocation.
         * It starts with an initial solution and iteratively explores neighbor solutions.
         * A neighbor solution is always accepted if it's better (lower cost). A worse
         * solution can also be accepted with a certain probability, which depends on the
         * cost difference and the current temperature `T`. This allows the search to
         * escape local optima. The temperature is gradually decreased according to the
         * cooling rate `alpha`. The process stops when the temperature falls below a
         * minimum threshold.
         *
         * @param[in,out] state The Result object containing the initial solution. It is
         * updated in-place to hold the best solution found by the algorithm.
         * @param[in] T The initial temperature for the annealing process.
         * @param[in] alpha The cooling rate (e.g., 0.95), used to decrease the temperature.
         */
        inline void simulatedAnnealing(Result& state, double T, double alpha) {
            iVec& coveredDevicesIdx = state.coveredDevicesIdx;

            Devices bestDevices = state.devices, currentDevices = state.devices;
            Servers bestServers = state.servers, currentServers = state.servers;

            double bestCNS = state.metrics->outputs.cost_of_non_service, currentCNS = bestCNS;
            double bestCSU = state.metrics->outputs.cost_of_servers_used, currentCSU = bestCSU;
            double bestCost = bestCNS + bestCSU, currentCost = bestCost;
          
            auto startChrono = std::chrono::high_resolution_clock::now();

            while (T > 1e-3) {
                for (int i = 0; i < 10; ++i) {
                    Devices neighborDevices = currentDevices;
                    Servers neighborServers = currentServers;
                    double neighborCNS = currentCNS;
                    double neighborCSU = currentCSU;
                    
                    generateNeighbor(neighborDevices, neighborServers, coveredDevicesIdx, neighborCNS, neighborCSU);
                    
                    double neighborCost = neighborCNS + neighborCSU;
                    double delta = neighborCost - currentCost;

                    if (delta < 0) {
                        i = 0; // if accepted, persist in this interval
                        currentCNS = neighborCNS;
                        currentCSU = neighborCSU;
                        currentCost = neighborCost;
                        currentDevices = std::move(neighborDevices);
                        currentServers = std::move(neighborServers);

                        if (currentCost < bestCost) {
                            bestCNS = currentCNS;
                            bestCSU = currentCSU;
                            bestCost = currentCost;
                            bestDevices = currentDevices;
                            bestServers = currentServers;
                        }

                    } else if (utils::randomNumber(0.0, 1.0) < std::exp(-delta / T)) {
                        currentCNS = neighborCNS;
                        currentCSU = neighborCSU;
                        currentCost = neighborCost;
                        currentDevices = std::move(neighborDevices);
                        currentServers = std::move(neighborServers);
                    }
                } 
                T *= alpha;
            }
            auto endChrono = std::chrono::high_resolution_clock::now();
            state.metrics->outputs.execution_time_sec += std::chrono::duration<double>(endChrono - startChrono).count();

            NetworkResourceAllocation::calculateMetrics(bestDevices, bestServers, *state.metrics);
        }
    }   

    /**
     * @brief Manages the execution of a meta-heuristic algorithm for a specified number of runs.
     * @details This function orchestrates the entire meta-heuristic process. It first
     * generates an initial solution using a specified heuristic (e.g., "Random" or
     * "Greedy"). It then runs the selected meta-heuristic (e.g., "SA") starting from
     * that solution. The entire process (initial solution generation + meta-heuristic
     * optimization) is repeated `loopTest` times to gather statistical data.
     *
     * @param[in] algorithm_name The meta-heuristic algorithm to run (e.g., "SA").
     * @param[in] state The initial state from the pre-calculation phase. This object is
     * copied for each run and is NOT modified by this function.
     * @param[in] T The initial temperature for Simulated Annealing.
     * @param[in] alpha The cooling rate for Simulated Annealing.
     * @param[in] heuristic_used The heuristic to generate the initial solution.
     * @param[in] loopTest The number of independent times to run the full process.
     */
    inline void bootup(const std::string& algorithm_name, const Result& state, double T, double alpha, const std::string& heuristic_used, int loopTest) {
        
        Result baseState = state;
        
        if (heuristic_used == "Random") {
            for (int i = 0; i < loopTest; ++i) {
                Result iteration = baseState;
                Heuristics::bootup(heuristic_used, iteration);

                if (algorithm_name == "SA") {
                    simulatedAnnealing(iteration, T, alpha);
                } else {
                    std::cerr << "Error: Unknown simulation or algorithm type." << std::endl;
                    return;
                }

                if (state.metrics) {
                    auto metrics = std::make_unique<MetaHeuristicMetrics>("MetaHeuristic", algorithm_name, iteration.metrics, T, alpha, heuristic_used);
                    showStructs::showMetrics(*metrics);
                    metrics->saveResultsToFile();
                } else {
                    std::cerr << "Error: Metrics not available." << std::endl;
                    return;
                }
            }
        } else {
            Heuristics::bootup(heuristic_used, baseState);
            
            for (int i = 0; i < loopTest; ++i) {
                Result iteration = baseState;

                if (algorithm_name == "SA") {
                    simulatedAnnealing(iteration, T, alpha);
                } else {
                    std::cerr << "Error: Unknown simulation or algorithm type." << std::endl;
                    return;
                }

                if (state.metrics) {
                    auto metrics = std::make_unique<MetaHeuristicMetrics>("MetaHeuristic", algorithm_name, iteration.metrics, T, alpha, heuristic_used);                    
                    showStructs::showMetrics(*metrics);
                    metrics->saveResultsToFile();
                } else {
                    std::cerr << "Error: Metrics not available." << std::endl;
                    return;
                }
            }
        }
    }
}