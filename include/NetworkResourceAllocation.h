#pragma once

#include "DataGenerator.h"
#include "structs.h"
#include "utils.h"

#include <chrono>

namespace NetworkResourceAllocation {

    /**
     * @brief Loads or generates device data and parses it into a vector of Device structs.
     * @details This function calls `DataGenerator::devicesData` to get the raw string data,
     * then parses each row into a `Device` object. The returned vector is 1-indexed,
     * with the element at index 0 being a default-constructed placeholder. It includes
     * error handling to skip rows that cannot be parsed correctly.
     *
     * @param[in] length The number of devices to load or generate.
     * @return A 1-indexed `std::vector<Device>`. Returns an empty vector on failure
     * (e.g., if the data file cannot be found or is empty).
     */
    inline Devices loadDevices(int length) {
        std::vector<std::vector<std::string>> strDevices = DataGenerator::devicesData(length);
        if (strDevices.size() <= 1) {
            std::cerr << "Error: Device data file not found or is empty." << std::endl;
            return {};
        }

        Devices devices;
        devices.reserve(length + 1);
        devices.emplace_back(); // Placeholder for 1-based indexing.
        
        for (const auto& row : strDevices) {
            if (row.at(0) == "#") continue;
            try {
                devices.emplace_back(
                    std::stoi(row.at(0)),  // #: id
                    std::stod(row.at(1)),  // LAT: lat
                    std::stod(row.at(2)),  // LON: lon
                    std::stod(row.at(3)),  // CND: cnd
                    std::stod(row.at(4)),  // PCC: pcc
                    std::stoi(row.at(5)),  // PCN: pcn
                    std::stod(row.at(6)),  // MEM: mem
                    std::stod(row.at(7)),  // STO: sto
                    std::stod(row.at(8)),  // S_d: s_d
                    std::stoi(row.at(9))   // SVC: svc
                );
            } catch (const std::exception& e) {
                std::cerr << "Error parsing device data: " << e.what() << std::endl;
                // Continue to next row
            }
        }
        return devices;
    }
    
    /**
     * @brief Loads or generates server data (EC and CC) into a single vector of Server structs.
     * @details Fetches data for both Edge and Cloud servers using the `DataGenerator`. It then
     * parses and combines them into a single 1-indexed `Servers` vector, assigning a `type`
     * character ('E' for Edge, 'C' for Cloud) to each server during parsing.
     *
     * @param[in] ecLength The number of EC servers to load.
     * @param[in] ccLength The number of CC servers to load.
     * @return A 1-indexed `std::vector<Server>` containing both server types. Returns
     * an empty vector on failure.
     */
    inline Servers loadServers(int ecLength, int ccLength = 5) {
        std::vector<std::vector<std::string>> strServerEC = DataGenerator::ecData(ecLength);
        std::vector<std::vector<std::string>> strServerCC = DataGenerator::ccData(ccLength);
        if (strServerEC.size() <= 1 || strServerCC.size() <= 1) {
            std::cerr << "Error: Server data file(s) not found or are empty." << std::endl;
            return {};
        }

        Servers servers;
        servers.reserve(ecLength + ccLength + 1);
        servers.emplace_back(); // Placeholder for 1-based indexing.

        auto parseAndAdd = [&](const std::vector<std::vector<std::string>>& matrix, char type) {
            for (const auto& row : matrix) {
                if (row.at(0) == "#") continue;
                try {
                    servers.emplace_back(
                        std::stoi(row.at(0)),  // #: id
                        std::stod(row.at(1)),  // LAT: lat
                        std::stod(row.at(2)),  // LON: lon
                        std::stod(row.at(3)),  // CSC: csc
                        std::stod(row.at(4)),  // PCC: pcc
                        std::stoi(row.at(5)),  // PCN: pcn
                        std::stod(row.at(6)),  // MEM: mem
                        std::stod(row.at(7)),  // STO: sto
                        std::stod(row.at(8)),  // T_p: t_p
                        type
                    );
                } catch (const std::exception& e) {
                    std::cerr << "Error parsing server data: " << e.what() << std::endl;
                }
            }
        };

        parseAndAdd(strServerEC, 'E');
        parseAndAdd(strServerCC, 'C');
        
        return servers;
    }    

    /**
     * @brief Sorts device indices by their condition (cnd) in ascending order.
     * @details A convenience wrapper around `utils::sortEntities`.
     * @param[in] devices The vector of devices whose indices are to be sorted.
     * @return A `std::vector<int>` containing the sorted indices.
     */
    inline iVec devicesAsc(const Devices& devices) {
        return utils::sortEntities<true, Device, const double, &Device::cnd>(devices);
    }

    /**
     * @brief Sorts device indices by their condition (cnd) in descending order.
     * @details A convenience wrapper around `utils::sortEntities`.
     * @param[in] devices The vector of devices whose indices are to be sorted.
     * @return A `std::vector<int>` containing the sorted indices.
     */
    inline iVec devicesDesc(const Devices& devices) {
        return utils::sortEntities<false, Device, const double, &Device::cnd>(devices);
    }

    /**
     * @brief Sorts server indices by their cost (csc) in ascending order.
     * @details A convenience wrapper around `utils::sortEntities`.
     * @param[in] servers The vector of servers whose indices are to be sorted.
     * @return A `std::vector<int>` containing the sorted indices.
     */
    inline iVec serversAsc(const Servers& servers) {
        return utils::sortEntities<true, Server, const double, &Server::csc>(servers);
    }

    /**
     * @brief Sorts server indices by their cost (csc) in descending order.
     * @details A convenience wrapper around `utils::sortEntities`.
     * @param[in] servers The vector of servers whose indices are to be sorted.
     * @return A `std::vector<int>` containing the sorted indices.
     */
    inline iVec serversDesc(const Servers& servers) {
        return utils::sortEntities<false, Server, const double, &Server::csc>(servers);
    }

    /**
     * @brief Calculates the geographic distance between two entities.
     * @details This templated wrapper extracts the latitude and longitude from two
     * entity objects and passes them to the core `utils::calculateDistance` function.
     * @tparam T1 The type of the first entity (e.g., Device, Server).
     * @tparam T2 The type of the second entity (e.g., Device, Server).
     * @param[in] pointA The first entity object.
     * @param[in] pointB The second entity object.
     * @return The distance in kilometers.
     */
    template <typename T1, typename T2>
    inline double calculateDistance(const T1& pointA, const T2& pointB) {
        return utils::calculateDistance(pointA.lat, pointA.lon, pointB.lat, pointB.lon);
    }

    /**
     * @brief Retrieves network technology parameters based on an ID.
     * @details Acts as a lookup table for mobile network generations, returning their
     * typical coverage radius and data rate.
     * @param[in] tech The technology identifier (1=1G+, 2=2G, ..., 6=6G).
     * @return A `std::pair<double, double>` containing `{radius_km, data_rate_mbps}`.
     * Returns `{-1.0, -1.0}` for an unknown ID.
     */
    inline std::pair<double, double> techParams(int tech) {
        switch (tech) {
            case 1: return {20.0, 0.0024};   // 1G+
            case 2: return {10.0, 0.064};    // 2G
            case 3: return {5.0, 2.0};       // 3G
            case 4: return {3.0, 100.0};     // 4G
            case 5: return {0.6, 1000.0};    // 5G
            case 6: return {0.32, 10000.0};  // 6G
            default:
                std::cerr << "Error: Unknown technology type: " << tech << std::endl;
                return {-1.0, -1.0};
        }
    }

    /**
     * @brief Assigns bandwidth values to all devices and servers.
     * @details Sets the `bw` member for all devices based on the provided data rate.
     * Servers are assigned a constant, high-speed backbone data rate.
     * @param[in,out] devices The vector of devices to be updated.
     * @param[in,out] servers The vector of servers to be updated.
     * @param[in] dataRate The data rate in Mbps for devices.
     */
    inline void bandwidth(Devices& devices, Servers& servers, double dataRate) {
        constexpr double DATA_RATE_EC_TO_CC_MBPS = 100000.0; // High-speed backbone
        for (size_t i = 1; i < devices.size(); ++i) {
            devices[i].bw = dataRate;
        }
        for (size_t i = 1; i < servers.size(); ++i) {
            servers[i].bw = DATA_RATE_EC_TO_CC_MBPS;
        }
    }

    /**
     * @brief Identifies which devices are within coverage range of edge servers.
     * @details Iterates through all device-server pairs. If a device is within the
     * coverage radius of an edge server, it is marked as `covered` and that server
     * is added to its list of potential servers. Cloud servers are added as potential
     * servers for all covered devices. The cost of non-coverage is calculated for
     * devices that remain out of range.
     *
     * @param[in,out] devices The vector of devices, to be updated with coverage status.
     * @param[in] servers The vector of all servers.
     * @param[in] coverageRadius The maximum distance (km) for a device to be covered.
     * @param[in,out] metrics The metrics object, updated with the cost of non-coverage.
     * @return A vector of indices for all covered devices.
     */
    inline iVec findCovering(Devices& devices, Servers& servers, double coverageRadius, Metrics& metrics) {
        iVec coveredDeviceIds;
        for (size_t i = 1; i < devices.size(); ++i) {
            Device& device = devices[i];
            
            for (size_t j = 1; j < servers.size(); ++j) {
                const Server& server = servers[j];
                if (server.type != 'E') continue;

                double distance = calculateDistance(device, server);
                if (distance <= coverageRadius) {
                    device.servers.emplace_back((int) j, distance);
                    device.covered = true;
                }
            }

            if (device.covered) {
                coveredDeviceIds.push_back(device.id);
                for (size_t j = 1; j < servers.size(); ++j) {
                    const Server& server = servers[j];
                    if (server.type == 'C') {
                        device.servers.emplace_back((int) j);
                    }
                }
            } else {
                metrics.outputs.cost_of_non_coverage += device.cnd;
            }
        }
        metrics.outputs.devices_covered_count = coveredDeviceIds.size();
        return coveredDeviceIds;
    }

    constexpr long double SPEED_OF_LIGHT = 299792.458L; // Speed of light in km/s
    // Latency from Milan to Ohio CC in ms, measured on 2024-12-18.
    constexpr double INTER_DC_LATENCY_MS = 111.86;

    /**
     * @brief Calculates connection, processing, and response times for each potential device-server pair.
     * @details For each covered device, this function iterates through its list of potential
     * servers and calculates timing metrics. The connection time for a cloud server
     * is calculated as a two-hop path (device -> closest edge -> cloud) and includes
     * a fixed inter-datacenter latency. These values are stored in the `server_covering`
     * structs within each device.
     *
     * @param[in,out] devices The vector of devices to be updated with timing data.
     * @param[in,out] servers The vector of servers.
     */
    inline void timeCalculation(Devices& devices, Servers& servers) {
        for (size_t i = 1; i < devices.size(); ++i) {
            Device& device = devices[i];
            if (!device.covered) continue;

            std::pair<int, double> closestEdge = {0, utils::EARTH_RADIUS_KM};
            for (const auto& s_info : device.servers) {
                if (servers.at(s_info.id).type == 'E' && s_info.distance < closestEdge.second) {
                    closestEdge = {s_info.id, s_info.distance};
                }
            }

            for (auto& s : device.servers) {
                Server& server = servers.at(s.id);
                s.processingTime = device.s_d * server.t_p;
                double transmission_time_ms = (device.s_d / device.bw) * 1000.0;
                
                if (server.type == 'C') {
                    s.id_routing = closestEdge.first;
                    double propagation_dist = closestEdge.second + calculateDistance(servers.at(closestEdge.first), server);
                    double propagation_delay_ms = (propagation_dist / SPEED_OF_LIGHT) * 1000.0;
                    s.connectionTime = transmission_time_ms + propagation_delay_ms + INTER_DC_LATENCY_MS;
                } else {
                    double propagation_delay_ms = (s.distance / SPEED_OF_LIGHT) * 1000.0;
                    s.connectionTime = transmission_time_ms + propagation_delay_ms;
                }
                s.responseTime = s.connectionTime + s.processingTime;
            }
        }
    }

    /**
     * @brief Runs the complete coverage and routing time calculation phase.
     * @details This function orchestrates the pre-calculation steps. It retrieves tech
     * parameters, assigns bandwidth, finds covered devices, and calculates all
     * timing metrics for potential device-server connections.
     *
     * @param[in,out] devices The main vector of devices.
     * @param[in,out] servers The main vector of servers.
     * @param[in,out] metrics The metrics object, to be updated during the process.
     * @return A vector of IDs for the devices that were successfully covered.
     */
    inline iVec coverage(Devices& devices, Servers& servers, Metrics& metrics) {
        std::pair<double, double> techProps = techParams(metrics.inputs.tech);
        if (techProps.first < 0) {
            std::cerr << "Error: Invalid technology ID provided." << std::endl;
            return {};
        }
        bandwidth(devices, servers, techProps.second);
        iVec coveredDevices = findCovering(devices, servers, techProps.first, metrics);
        timeCalculation(devices, servers);
        return coveredDevices;
    }

    /**
     * @brief Calculates and populates the metrics object based on a final allocation state.
     * @details This function should be called *after* an allocation algorithm has run. It
     * resets all output metric counters and then iterates through the final state of
     * all devices and servers to calculate aggregate statistics like counts of served
     * devices, used servers, total costs, and average response time.
     *
     * @param[in] devices The state of all devices after an allocation attempt.
     * @param[in] servers The state of all servers after an allocation attempt.
     * @param[in,out] metrics The metrics object to be reset and populated.
     */
    inline void calculateMetrics(const Devices& devices, const Servers& servers, Metrics& metrics) {
        metrics.outputs.devices_served_count = 0;
        metrics.outputs.devices_served_ec_count = 0;
        metrics.outputs.devices_served_cc_count = 0;
        metrics.outputs.servers_used_count = 0;
        metrics.outputs.servers_used_ec_count = 0;
        metrics.outputs.servers_used_cc_count = 0;
        metrics.outputs.cost_of_servers_used = 0.0;
        metrics.outputs.cost_of_non_service = 0.0;
        metrics.outputs.total_cost = 0.0;
        metrics.outputs.average_response_time = 0.0;        
        
        for (const auto& device : devices) {
            if (device.id == 0) continue;
            if (device.served) {
                metrics.outputs.devices_served_count++;
                metrics.outputs.average_response_time += device.server.responseTime;
                if (servers.at(device.server.id).type == 'E') {
                    metrics.outputs.devices_served_ec_count++;
                } else {
                    metrics.outputs.devices_served_cc_count++;
                }
            } else if (device.covered) {
                metrics.outputs.cost_of_non_service += device.cnd;
            }
        }

        for (const auto& server : servers) {
            if (server.id == 0) continue;
            if (server.on) {
                metrics.outputs.cost_of_servers_used += server.csc;
                if (server.type == 'E') {
                    metrics.outputs.servers_used_ec_count++;
                } else { // 'C'
                    metrics.outputs.servers_used_cc_count++;
                }
            }
        }

        if (metrics.outputs.devices_served_count > 0) {
            metrics.outputs.average_response_time /= metrics.outputs.devices_served_count;
        }

        metrics.outputs.servers_used_count = metrics.outputs.servers_used_ec_count + metrics.outputs.servers_used_cc_count;
        metrics.outputs.total_cost = metrics.outputs.cost_of_non_coverage + metrics.outputs.cost_of_non_service +  metrics.outputs.cost_of_servers_used;
    }

    /**
     * @brief Prepares the complete initial state for any simulation.
     * @details This is the main entry point for the setup phase. It loads all data,
     * creates the base metrics object, performs the full coverage and time calculation,
     * and bundles all resulting data into a `Result` struct, which is ready to be
     * passed to an allocation algorithm.
     *
     * @param[in] simulation_type The category of the simulation (e.g., "Heuristic").
     * @param[in] algorithm_name The specific name of the algorithm (e.g., "Random").
     * @param[in] numDevices The number of devices to load.
     * @param[in] numServersEC The number of edge servers to load.
     * @param[in] numServersCC The number of cloud servers to load.
     * @param[in] tech The network technology ID.
     * @return An `std::optional<Result>` containing the initial state, or `std::nullopt` on failure.
     */
    inline std::optional<Result> pre_calculation(const std::string& simulation_type, const std::string& algorithm_name, int numDevices, int numServersEC, int numServersCC, int tech) {
        if (numDevices <= 0 || numServersEC <= 0 || numServersCC <= 0) {
            std::cerr << "Error: Number of devices and servers must be positive." << std::endl;
            return std::nullopt;
        }
        
        Devices devices = loadDevices(numDevices);
        Servers servers = loadServers(numServersEC, numServersCC);

        if (devices.empty() || servers.empty()) {
            std::cerr << "Error: Failed to load device or server data." << std::endl;
            return std::nullopt;
        }

        auto metrics = std::make_unique<Metrics>(simulation_type, algorithm_name, numDevices, numServersEC, numServersCC, tech);

        iVec coveredDevicesIdx = coverage(devices, servers, *metrics);
        
        return Result{std::move(devices), std::move(servers), std::move(coveredDevicesIdx), std::move(metrics)};
    }

    /**
     * @brief Modifies existing devices to create a resource bottleneck on Cloud servers.
     * @details This function implements a specific test scenario designed to highlight the
     * weaknesses of a greedy allocation algorithm. It operates by targeting all available
     * Cloud Computing (CC) servers. For each CC server, it selects one device
     * from the pool of already covered devices and modifies its attributes. This selection
     * can be either deterministic or randomized. The device's memory and storage demands
     * are increased to nearly 100% of the target server's capacity, effectively
     * saturating it. Simultaneously, the device's non-service cost (cnd) is set to
     * the maximum possible value, ensuring it gets prioritized by greedy heuristics.
     * 
     * @param[in,out] state An std::optional<Result> containing the entire simulation
     * state. The function will directly modify the Device objects within the state's
     * `devices` vector.
     * @param[in] randomBottleneck If true, the devices to be modified are selected
     * randomly from the covered devices pool. If false, the selection is deterministic,
     * picking the first devices from the list. Defaults to false.
     * @note This function should be called after the `pre_calculation` step, as it
     * depends on the list of `coveredDevicesIdx`.
     */
    inline void createBottleneck(std::optional<Result>& state, bool randomBottleneck = false) {
        Devices& devices = state->devices;
        Servers& servers = state->servers;
        
        iiPVec bottleneckValues;
        int startCC = state->metrics->inputs.servers_ec + 1;
        for (int i = startCC; i < servers.size(); ++i) {
            bottleneckValues.push_back({servers[i].mem, servers[i].sto});
        }
        
        int loop = state->metrics->inputs.servers_cc;
        iVec idx = state->coveredDevicesIdx;
        if (randomBottleneck) shuffle(idx.begin(), idx.end(), utils::getEngine());
        int i = 0;
        while (loop--) {
            //showStructs::showDevice(devices[idx[i]]);
            devices[idx[i]].mem = bottleneckValues[i].first  * 0.999999;
            devices[idx[i]].sto = bottleneckValues[i].second * 0.999999;
            devices[idx[i]].cnd = 9.9;
            //showStructs::showDevice(devices[idx[i]]);
            i++;
        }
    }
}