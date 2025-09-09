#pragma once

#include "NetworkResourceAllocation.h"

namespace Heuristics {
    namespace {
        /**
         * @brief A simple heuristic that allocates each covered device to a random available server.
         * @details This function iterates through a randomly shuffled list of all covered
         * devices. For each device, it randomly selects one of its potential covering
         * servers. If the chosen server has sufficient available capacity, the device is
         * allocated to it. Otherwise, the device remains unserved by this heuristic.
         *
         * @param[in,out] state A reference to the Result object. It provides the initial
         * state and is updated in-place with the allocation results and calculated metrics.
         */
        inline void randomHeuristic(Result& state) {
            Devices& devices = state.devices;
            Servers& servers = state.servers;
            iVec coveredDevicesIdx = state.coveredDevicesIdx;

            auto startChrono = std::chrono::high_resolution_clock::now();
            
            std::shuffle(coveredDevicesIdx.begin(), coveredDevicesIdx.end(), utils::getEngine());
            
            for (const auto& d_idx : coveredDevicesIdx) {
                Device& device = devices.at(d_idx);
                if (device.servers.empty()) continue;
                
                int s_idx = utils::randomNumber(0, (int)device.servers.size());
                if (s_idx == device.servers.size()) continue; // rejects the device
                
                server_covering& potential_server = device.servers.at(s_idx);
                Server& server = servers.at(potential_server.id);
                
                if (server.canServe(device)) {
                    server.addServed(device);
                    device.server = potential_server;
                }
            }
            
            auto endChrono = std::chrono::high_resolution_clock::now();
            state.metrics->outputs.execution_time_sec += std::chrono::duration<double>(endChrono - startChrono).count();

            NetworkResourceAllocation::calculateMetrics(devices, servers, *state.metrics);
        }
        
        /**
         * @brief A greedy heuristic that allocates devices based on sorted criteria.
         * @details This algorithm first sorts the list of covered devices based on their
         * cost of non-service (cnd). It then iterates through this sorted list. For each
         * device, it sorts its list of potential servers by response time. Finally, it
         * attempts to allocate the device to the first server in the sorted list that
         * has enough capacity. Once an allocation is made, it moves to the next device.
         *
         * @param[in,out] state A reference to the Result object, which is updated
         * in-place with the greedy allocation and final metrics.
         * @param[in] sortDevicesAsc If true, sorts devices by CND in ascending order;
         * otherwise, sorts in descending order.
         * @param[in] sortServersAsc If true, sorts servers by response time in
         * ascending order (best first); otherwise, descending.
         */
        inline void greedyHeuristic(Result& state, bool sortDevicesAsc, bool sortServersAsc) {
            Devices& devices = state.devices;
            Servers& servers = state.servers;

            auto startChrono = std::chrono::high_resolution_clock::now();
            
            iVec sortedCoveredIdx;
            if (sortDevicesAsc) {
                sortedCoveredIdx = utils::sortEntities<true, Device, const double, &Device::cnd>(devices, state.coveredDevicesIdx);
            } else {
                sortedCoveredIdx = utils::sortEntities<false, Device, const double, &Device::cnd>(devices, state.coveredDevicesIdx);
            }

            for (const auto& d_idx : sortedCoveredIdx) {
                Device& device = devices.at(d_idx);
                if (sortServersAsc) {
                    std::sort(device.servers.begin(), device.servers.end(), [](const server_covering& a, const server_covering& b) {
                        return a.responseTime < b.responseTime;
                    });
                } else {
                    std::sort(device.servers.begin(), device.servers.end(), [](const server_covering& a, const server_covering& b) {
                        return a.responseTime > b.responseTime;
                    });
                }
                
                for (const auto& potential_server : device.servers) {
                    Server& server = servers.at(potential_server.id);
                    
                    if (server.canServe(device)) {
                        server.addServed(device);
                        device.server = potential_server;   
                        break; 
                    }
                }
            }
            
            auto endChrono = std::chrono::high_resolution_clock::now();
            state.metrics->outputs.execution_time_sec += std::chrono::duration<double>(endChrono - startChrono).count();

            NetworkResourceAllocation::calculateMetrics(devices, servers, *state.metrics);
        }
    }
    
    /**
     * @brief Serves as the main entry point for running a specific heuristic algorithm.
     * @details This function acts as a dispatcher. It parses the `algorithm` string to
     * determine which heuristic to execute. For "Greedy" variants, it also parses the
     * name to set the sorting direction for both devices and servers before invoking
     * the `greedyHeuristic` function. After the heuristic runs, it finalizes,
     * displays, and saves the resulting metrics.
     *
     * @param[in] algorithm The specific algorithm name (e.g., "Random", "Greedy_DescAsc").
     * @param[in,out] state The Result object containing the initial simulation state, which will
     * be modified by the selected heuristic.
     */
    inline void bootup(const std::string& algorithm, Result& state) {
        if (algorithm == "Random") {
            randomHeuristic(state);
        } else if (algorithm.rfind("Greedy", 0) == 0) {
            bool sortDevicesAsc = (algorithm == "Greedy_AscAsc" || algorithm == "Greedy_AscDesc");
            bool sortServersAsc = (algorithm == "Greedy_AscAsc" || algorithm == "Greedy_DescAsc");
            greedyHeuristic(state, sortDevicesAsc, sortServersAsc);
        } else {
            std::cerr << "Error: Unknown heuristic algorithm type." << std::endl;
            return;
        }

        if (state.metrics) {
            auto metrics = std::make_unique<HeuristicMetrics>("Heuristic", algorithm, state.metrics);
            showStructs::showMetrics(*metrics);
            metrics->saveResultsToFile(); // Use the final server count
        }
    }
}