#pragma once

#include "utils.h"

#include <filesystem>
#include <memory>
#include <set>
#include <utility>

#define Devices std::vector<Device> ///< A type alias for a vector of Device objects.
#define Servers std::vector<Server> ///< A type alias for a vector of Server objects.
#define iiP std::pair<int, int>     ///< A type alias for a pair of integers, typically used for index pairs.
#define iVec std::vector<int>       ///< A type alias for a vector of integers, typically used for indices.
#define iiPVec std::vector<iiP>     ///< A type alias for a vector of integer pairs, typically used for index pairs.

/**
 * @struct server_covering
 * @brief Holds pre-calculated data about a potential or assigned server for a device.
 * @details This struct represents a potential connection between a device and a server.
 * It stores not only the server's ID but also key performance indicators like
 * distance and response time, which are pre-calculated to speed up decision-making
 * in allocation algorithms.
 */
struct server_covering {
    int id = 0;                  ///< The unique identifier of the covering server.
    int id_routing = 0;          ///< The ID of the edge server for routing if this is a cloud server.
    double distance = 0.0;       ///< Geographic distance from the device to the server (in km).
    double connectionTime = 0.0; ///< Total network time (propagation + transmission) in ms.
    double processingTime = 0.0; ///< Time for the server to process the device's task (in ms).
    double responseTime = 0.0;   ///< Total time: connectionTime + processingTime.

    server_covering() = default;
    explicit server_covering(int id_) : id(id_) {}
    server_covering(int id_, double distance_) : id(id_), distance(distance_) {}
};

/**
 * @struct server_supply
 * @brief Aggregates the current resource demand on a single server.
 * @details This struct acts as a state tracker for a server's resource consumption.
 * It sums the requirements of all devices currently allocated to the server,
 * allowing for efficient capacity checks when considering a new device allocation.
 */
struct server_supply {
    int pcnD = 0;        ///< Demand: Total number of processing cores required.
    double cndD = 0.0;   ///< Demand: Sum of non-service costs for all served devices.
    double pccD = 0.0;   ///< Demand: Total processing core capacity required.
    double memD = 0.0;   ///< Demand: Total memory required.
    double stoD = 0.0;   ///< Demand: Total storage required.
    double bwD = 0.0;    ///< Demand: Total bandwidth required.
    std::set<int> devices_served; ///< Set of unique IDs of devices currently served.
};

/**
 * @struct Device
 * @brief Represents a user device with its requirements and simulation state.
 * @details This struct holds both the static attributes of a device (ID, location,
 * service requirements) and its dynamic state during the simulation, such as whether
 * it is covered, whether it has been served, and to which server it is assigned.
 */
struct Device {
    int id, pcn, svc;
    double lat, lon, cnd, pcc, mem, sto, s_d;
    double bw = 0.0;                      ///< Assigned bandwidth based on network technology.
    bool covered = false;                 ///< True if within range of at least one edge server.
    bool served = false;                  ///< True if allocated to a server for processing.
    server_covering server;               ///< The server that is ultimately assigned to this device.
    std::vector<server_covering> servers; ///< List of all potential servers that can serve this device.

    Device() : id(0), pcn(0), svc(0), lat(0.0), lon(0.0), cnd(0.0), pcc(0.0), mem(0.0), sto(0.0), s_d(0.0) {}
    Device(int id_, double lat_, double lon_, double cnd_, double pcc_, int pcn_, double mem_, double sto_, double s_d_, int svc_)
        : id(id_), lat(lat_), lon(lon_), cnd(cnd_), pcc(pcc_), pcn(pcn_), mem(mem_), sto(sto_), s_d(s_d_), svc(svc_) {}
};

/**
 * @struct Server
 * @brief Represents a server (Edge or Cloud) with its capacity and current state.
 * @details Holds a server's static capacities (location, cost, resources) and its
 * dynamic state during the simulation, including whether it is active (`on`) and
 * the current aggregated demand on its resources (`supply`).
 */
struct Server {
    int id, pcn;
    char type;
    double lat, lon, csc, pcc_per_core, pcc_total, mem, sto, t_p;
    double bw = 0.0;      ///< Maximum bandwidth capacity of the server.
    bool on = false;      ///< True if the server is active (serving at least one device).
    server_supply supply; ///< Current aggregated demand on the server's resources.

    Server() : id(0), pcn(0), type(' '), lat(0.0), lon(0.0), csc(0.0), pcc_per_core(0.0), pcc_total(0.0), mem(0.0), sto(0.0), t_p(0.0) {}
    Server(int id_, double lat_, double lon_, double csc_, double pcc_, int pcn_, double mem_, double sto_, double t_p_, char type_)
        : id(id_), pcn(pcn_), type(type_), lat(lat_), lon(lon_), csc(csc_), pcc_per_core(pcc_), pcc_total(pcc_ * pcn_), mem(mem_), sto(sto_), t_p(t_p_) {}

    /**
     * @brief Checks if the server has enough available resources to serve a given device.
     * @details Performs a multi-dimensional capacity check, comparing the server's
     * remaining resources against the device's requirements for PCC, PCN, MEM,
     * STO, and BW.
     * @param[in] device The device to check against the server's available capacity.
     * @return Returns `true` if the device can be served, `false` otherwise.
     */
    inline bool canServe(const Device& device) const {
    return (supply.pccD + device.pcc <= this->pcc_total &&
            supply.pcnD + device.pcn <= this->pcn &&
            supply.memD + device.mem <= this->mem &&
            supply.stoD + device.sto <= this->sto &&
            supply.bwD  + device.bw  <= this->bw);
    }

    /**
     * @brief Allocates a device to this server and consumes its resources.
     * @details This function updates the server's state by marking it as active (`on`),
     * updating the device's state to `served`, and adding the device's resource
     * requirements to the server's total demand (`supply`).
     * @param[in,out] device The device to be served. Its `served` status is set to true.
     * @return `true` if the device was added successfully, `false` if it was already present.
     */
    inline bool addServed(Device& device) { 
        auto result = supply.devices_served.insert(device.id);
        if (!result.second) return false;
        
        this->on = true;
        device.served = true;
        supply.cndD += device.cnd;
        supply.pccD += device.pcc;
        supply.pcnD += device.pcn;
        supply.memD += device.mem;
        supply.stoD += device.sto;
        supply.bwD  += device.bw;
        return true;
    }

    /**
     * @brief Deallocates a device from this server, freeing up its resources.
     * @details Reverses the allocation process. It removes the device's requirements
     * from the server's demand, updates the device's state to not `served`, and if
     * the server becomes empty, it is marked as inactive (`on = false`).
     * @param[in,out] device The device to be removed. Its `served` status is set to false.
     * @return `true` if the device was found and removed, `false` otherwise.
     */
    inline bool rmvServed(Device& device) {
    if (supply.devices_served.find(device.id) == supply.devices_served.end()) return false;
    
    supply.devices_served.erase(device.id);
    device.served = false;
    
    supply.cndD -= device.cnd;
    supply.pccD -= device.pcc;
    supply.pcnD -= device.pcn;
    supply.memD -= device.mem;
    supply.stoD -= device.sto;
    supply.bwD  -= device.bw;

    if (supply.devices_served.empty()) {
        this->on = false;
    }
    return true;
    }
};

/**
 * @struct Metrics
 * @brief A polymorphic base structure for collecting and managing all simulation metrics.
 * @details This class stores both the input parameters and the output results of a
 * simulation run. It provides common functionalities for all metric types, such as
 * generating file paths and serializing data for output files.
 */
struct Metrics {
    std::string simulation_type;
    std::string algorithm_name;

    struct CommonInputs {
        int devices = 0;
        int servers_ec = 0;
        int servers_cc = 0;
        int tech = 0;
    } inputs;

    struct CommonOutputs {
        double execution_time_sec = 0.0;
        int    devices_covered_count = 0;
        int    devices_served_count = 0;
        int    devices_served_ec_count = 0;
        int    devices_served_cc_count = 0;
        int    servers_used_count = 0;
        int    servers_used_ec_count = 0;
        int    servers_used_cc_count = 0;
        double cost_of_servers_used = 0.0;
        double cost_of_non_coverage = 0.0;
        double cost_of_non_service = 0.0;
        double total_cost = 0.0;
        double average_response_time = 0.0;
    } outputs;

    Metrics(std::string simulation, std::string algorithm, int d, int s_ec, int s_cc, int t) : simulation_type(std::move(simulation)), algorithm_name(std::move(algorithm)), inputs({d, s_ec, s_cc, t}) {}
    Metrics(std::string simulation, std::string algorithm, const std::unique_ptr<Metrics>& base) : simulation_type(std::move(simulation)), algorithm_name(std::move(algorithm)), inputs(base->inputs), outputs(base->outputs) {}
    virtual ~Metrics() = default;
    
    /**
     * @brief Creates a deep copy of the concrete Metrics object.
     * @return A `std::unique_ptr<Metrics>` holding the new cloned object.
     */
    virtual std::unique_ptr<Metrics> clone() const {
        return std::make_unique<Metrics>(*this);
    }

    /**
     * @brief Constructs the base directory path for result files.
     * @return A `std::filesystem::path` like `Results/{sim_type}/{algo_name}`.
     */
    virtual std::filesystem::path getBaseDirectoryPath() const {
        return std::filesystem::path("Results") / this->simulation_type / this->algorithm_name;
    }

    /**
     * @brief Constructs the base file name for a specific simulation run.
     * @return A string like `D{devices}_S{servers}_{tech}G`.
     */
    inline std::string getBaseFileName() const {
        return "D" + std::to_string(this->inputs.devices) + "_S" + std::to_string(this->inputs.servers_ec + this->inputs.servers_cc) + "_" + (std::to_string(this->inputs.tech) + "G");
    }

    /**
     * @brief Gets the header (column names) for the results file.
     * @return A `std::vector<std::string>` containing the column names.
     */
    virtual std::vector<std::string> getHeader() const {
        return {"Devices", "Servers", "Tech", "ExeTime", "DCovered", "DServed", "DServedEC", "DServedCC", "SUsed", "SUsedEC", "SUsedCC", "TotalCost", "CostNCoverage", "CostNService", "CostS", "Avg.RTime"};
    }

    /**
     * @brief Serializes the metrics data into a row of strings for file output.
     * @return A `std::vector<std::string>` containing the metric values as strings.
     */
    virtual std::vector<std::string> data() const {
        return {
            utils::toString(inputs.devices),
            utils::toString(inputs.servers_ec + inputs.servers_cc),
            utils::toString(inputs.tech),
            utils::toString(outputs.execution_time_sec),
            utils::toPercentageString(outputs.devices_covered_count, inputs.devices),
            utils::toPercentageString(outputs.devices_served_count, inputs.devices),
            utils::toPercentageString(outputs.devices_served_ec_count, outputs.devices_served_count),
            utils::toPercentageString(outputs.devices_served_cc_count, outputs.devices_served_count),
            utils::toPercentageString(outputs.servers_used_count, inputs.servers_ec + inputs.servers_cc),
            utils::toPercentageString(outputs.servers_used_ec_count, inputs.servers_ec),
            utils::toPercentageString(outputs.servers_used_cc_count, inputs.servers_cc),
            utils::toString(outputs.total_cost),
            utils::toString(outputs.cost_of_non_coverage),
            utils::toString(outputs.cost_of_non_service),
            utils::toString(outputs.cost_of_servers_used),
            utils::toString(outputs.average_response_time)};
    }

    /**
     * @brief Appends the current metrics data to the appropriate results file.
     * @details Constructs the file path and name, creates the directory if needed,
     * and appends the serialized data. Adds a header row if the file is new.
     */
    inline void saveResultsToFile() const {
        std::filesystem::path result_path = this->getBaseDirectoryPath() / (this->getBaseFileName() + ".txt");

        std::vector<std::vector<std::string>> content;
        if (!std::filesystem::exists(result_path)) {
            content.push_back(this->getHeader());
        }

        content.push_back(this->data());

        // FileManager::append will create the directory if it doesn't exist.
        FileManager::append(result_path.string(), content, ';');
    }
};

/**
 * @struct MathMetrics
 * @brief Extends base Metrics to include results from mathematical solvers like CPLEX.
 */
struct MathMetrics : public Metrics {
    std::string status = "Unknown";
    double OF = 0.0;
    double gap = 1.0;

    MathMetrics(std::string simulation, std::string algorithm, int d, int s_ec, int s_cc, int t)
        : Metrics(std::move(simulation), std::move(algorithm), d, s_ec, s_cc, t) {}
    MathMetrics(std::string simulation, std::string algorithm, const std::unique_ptr<Metrics>& base)
        : Metrics(std::move(simulation), std::move(algorithm), base) {}
    
    std::unique_ptr<Metrics> clone() const override {
        return std::make_unique<MathMetrics>(*this);
    }

    std::vector<std::string> getHeader() const override {
        auto header = Metrics::getHeader();
        header.push_back("Status");
        header.push_back("OF");
        header.push_back("GAP");
        return header;
    }

    std::vector<std::string> data() const override {
        auto row = Metrics::data();
        row.push_back(status);
        row.push_back(utils::toString(OF));
        row.push_back(utils::toString(gap));
        return row;
    }
};

/**
 * @struct HeuristicMetrics
 * @brief Extends base Metrics for simple heuristics (e.g., Random, Greedy).
 */
struct HeuristicMetrics : public Metrics {
    HeuristicMetrics(std::string simulation, std::string algorithm, int d, int s_ec, int s_cc, int t)
        : Metrics(std::move(simulation), std::move(algorithm), d, s_ec, s_cc, t) {}
    HeuristicMetrics(std::string simulation, std::string algorithm, const std::unique_ptr<Metrics>& base)
        : Metrics(std::move(simulation), std::move(algorithm), base) {}
    
    std::unique_ptr<Metrics> clone() const override {
        return std::make_unique<HeuristicMetrics>(*this);
    }
};

/**
 * @struct MetaHeuristicMetrics
 * @brief Extends base Metrics to include parameters for meta-heuristics like SA.
 */
struct MetaHeuristicMetrics : public Metrics {
    std::string heuristic_used;
    double temperature = 0.0;
    double alpha = 0.0;

    MetaHeuristicMetrics(std::string simulation, std::string algorithm, int d, int s_ec, int s_cc, int t, double temp, double alph, std::string heuristic)
        : Metrics(std::move(simulation), std::move(algorithm), d, s_ec, s_cc, t), temperature(temp), alpha(alph), heuristic_used(std::move(heuristic)) {}
    MetaHeuristicMetrics(std::string simulation, std::string algorithm, const std::unique_ptr<Metrics>& base, double temp, double alph, std::string heuristic)
        : Metrics(std::move(simulation), std::move(algorithm), base), temperature(temp), alpha(alph), heuristic_used(std::move(heuristic)) {}
    
    std::unique_ptr<Metrics> clone() const override {
        return std::make_unique<MetaHeuristicMetrics>(*this);
    }

    inline std::filesystem::path getBaseDirectoryPath() const override {
        return Metrics::getBaseDirectoryPath() / this->heuristic_used;
    }

    std::vector<std::string> getHeader() const override {
        auto header = Metrics::getHeader();
        header.push_back("Temperature");
        header.push_back("Alpha");
        header.push_back("Heuristic");
        return header;
    }

    std::vector<std::string> data() const override {
        auto row = Metrics::data();
        row.push_back(utils::toString(temperature));
        row.push_back(utils::toString(alpha));
        row.push_back(heuristic_used);
        return row;
    }
};

/**
 * @struct Result
 * @brief A container for the complete state of a single simulation instance.
 * @details This struct bundles all necessary data for a simulation run: the vectors of
 * devices and servers, the list of covered device indices, and the metrics
 * object. This design facilitates passing the entire simulation state between
 * functions and creating copies for independent runs or neighborhood exploration.
 */
struct Result {
    Devices devices;
    Servers servers;
    iVec coveredDevicesIdx;
    std::unique_ptr<Metrics> metrics;

    Result(Devices d, Servers s, iVec c, std::unique_ptr<Metrics> m)
        : devices(std::move(d)), servers(std::move(s)), coveredDevicesIdx(std::move(c)), metrics(std::move(m)) {}

    // Custom copy constructor to correctly clone the unique_ptr
    Result(const Result& other)
        : devices(other.devices), servers(other.servers), coveredDevicesIdx(other.coveredDevicesIdx), metrics(other.metrics ? other.metrics->clone() : nullptr) {}
    
    // Custom copy assignment operator
    Result& operator=(const Result& other) {
        if (this != &other) {
            devices = other.devices;
            servers = other.servers;
            coveredDevicesIdx = other.coveredDevicesIdx;
            metrics = other.metrics ? other.metrics->clone() : nullptr;
        }
        return *this;
    }
};

namespace showStructs {
    /**
     * @brief Displays the detailed information of a single Device instance to the console.
     * @param[in] device The device object to display.
     */
    inline void showDevice(const Device& device) {
        std::cout << "========== Device ID: " << device.id << " ==========\n"
                  << "  - Location (Lat, Lon):  (" << device.lat << ", " << device.lon << ")\n"
                  << "  - Service ID:           " << device.svc << "\n"
                  << "  - Requirements (CND):   " << device.cnd << "\n"
                  << "  - Requirements (PCC):   " << device.pcc << "\n"
                  << "  - Requirements (PCN):   " << device.pcn << "\n"
                  << "  - Requirements (MEM):   " << device.mem << "\n"
                  << "  - Requirements (STO):   " << device.sto << "\n"
                  << "  - Requirements (S_d):   " << device.s_d << "\n"
                  << "  - State (Bandwidth):    " << device.bw << " Mbps\n"
                  << "  - State (Covered):      " << (device.covered ? "Yes" : "No") << "\n"
                  << "  - State (Served):       " << (device.served ? "Yes" : "No") << "\n";

        if (device.served) {
            std::cout << "  - Assigned Server ID:   " << device.server.id
                      << " (Response Time: " << device.server.responseTime << " ms)\n";
        } else {
            std::cout << "  - Assigned Server ID:   None\n";
        }

        std::cout << "  - Potential Servers (" << device.servers.size() << "):\n";
        if (device.servers.empty()) {
            std::cout << "    - None\n";
        } else {
            for (const auto& s : device.servers) {
                std::cout << "    - Server ID: " << std::setw(3) << s.id
                          << " | Response Time: " << std::fixed << std::setprecision(4) << s.responseTime << " ms\n";
            }
        }
        std::cout << "====================================\n" << std::endl;
    }

    /**
     * @brief Iterates through and displays a vector of Device objects.
     * @param[in] devices The vector of devices to display.
     */
    inline void showDevice(const Devices& devices) {
        std::cout << "\n--- Displaying " << devices.size() -1 << " Devices ---\n";
        for (const auto& device : devices) {
            if (device.id == 0) continue; // Skip placeholder at index 0
            showDevice(device);
        }
    }

    /**
     * @brief Displays the detailed information of a single Server instance to the console.
     * @param[in] server The server object to display.
     */
    inline void showServer(const Server& server) {
        std::cout << "========== Server ID: " << server.id << " (Type: " << server.type << ") ==========\n"
                  << "  - Location (Lat, Lon): " << server.lat << ", " << server.lon << "\n"
                  << "  - Status (ON):         " << (server.on ? "Yes" : "No") << "\n\n"
                  << "  --- Capacity ---\n"
                  << "  - Cost (CSC):          " << server.csc << "\n"
                  << "  - PCC per Core:        " << server.pcc_per_core << "\n"
                  << "  - Total PCC:           " << server.pcc_total << "\n"
                  << "  - Core Count (PCN):    " << server.pcn << "\n"
                  << "  - Memory (MEM):        " << server.mem << "\n"
                  << "  - Storage (STO):       " << server.sto << "\n"
                  << "  - Bandwidth (BW):      " << server.bw << " Mbps\n"
                  << "  - Proc. Time (T_p):    " << server.t_p << "\n\n"
                  << "  --- Current Demand ---\n"
                  << "  - Demand (PCC):        " << server.supply.pccD << "\n"
                  << "  - Demand (PCN):        " << server.supply.pcnD << "\n"
                  << "  - Demand (MEM):        " << server.supply.memD << "\n"
                  << "  - Demand (STO):        " << server.supply.stoD << "\n"
                  << "  - Demand (BW):         " << server.supply.bwD << "\n"
                  << "  - Devices Served (" << server.supply.devices_served.size() << "): ";

        if (server.supply.devices_served.empty()) {
            std::cout << "None\n";
        } else {
            std::string device_list;
            for (int device_id : server.supply.devices_served) {
                device_list += std::to_string(device_id) + ", ";
            }
            // Remove trailing comma and space
            std::cout << device_list.substr(0, device_list.length() - 2) << "\n";
        }
        std::cout << "===============================================\n" << std::endl;
    }

     /**
     * @brief Iterates through and displays a vector of Server objects.
     * @param[in] servers The vector of servers to display.
     */
    inline void showServer(const Servers& servers) {
        std::cout << "\n--- Displaying " << servers.size() - 1 << " Servers ---\n";
        for (const auto& server : servers) {
            if (server.id == 0) continue; // Skip placeholder at index 0
            showServer(server);
        }
    }

    namespace {
        const int total_width = 62;
        const int label_width = 26; 
       
        inline void print_row (const std::string& label, const std::string& value) {
            std::cout << "| " << std::left << std::setw(label_width) << label
                        << " | "<< std::left << std::setw(total_width - label_width - 7) << value
                        << " |" << std::endl;
        }

        inline void print_header() {
            std::cout << "+" << std::string(total_width - 2, '=') << "+" << std::endl;
        };

        inline void print_midle() {
            std::cout << "+" << std::string(label_width + 2, '-') << "+"
                    << std::string(total_width - label_width - 5, '-') << "+" << std::endl;
        };
        
        inline void print_title(const std::string& title) {
            int padding_total = total_width - 3 - title.length();
            int padding_left = padding_total / 2;
            int padding_right = padding_total - padding_left + 1;
            std::cout << "|" << std::string(padding_left, ' ') << title
                    << std::string(padding_right, ' ') << "|" << std::endl;
        };
        
        /**
         * @brief Displays the common metrics in a formatted table.
         * @details A private helper function used by the public `showMetrics` overloads
         * to render the shared part of the metrics data in a consistent, boxed format.
         * @param[in] metrics A reference to the metrics object to display.
         */
        inline void show_common_metrics(const Metrics& metrics) {
            const auto& in = metrics.inputs;
            const auto& out = metrics.outputs;

            // --- Top of Table ---
            std::cout << std::endl;
            print_header();
            print_title("SIMULATION " + metrics.simulation_type + " METRICS");
            
            // --- Block 1: General Info ---
            print_midle();
            print_row("Algorithm", metrics.algorithm_name);
            print_row("Execution Time (s)", utils::toString(out.execution_time_sec, 6));
            print_row("Mobile Technology", std::to_string(in.tech) + "G");

            // --- Block 2: Device Stats ---
            print_midle();
            print_title("DEVICES");
            print_midle();
            print_row("Total", std::to_string(in.devices));
            std::string devices_covered_str = std::to_string(out.devices_covered_count) + " (" + utils::toPercentageString(out.devices_covered_count, in.devices) + "%)";
            print_row("Covered", devices_covered_str);
            std::string devices_served_str = std::to_string(out.devices_served_count) + " (" + utils::toPercentageString(out.devices_served_count, in.devices) + "%)";
            print_row("Served", devices_served_str);
            std::string served_ec_str = std::to_string(out.devices_served_ec_count) + " (" + utils::toPercentageString(out.devices_served_ec_count, out.devices_served_count) + "% of served)";
            print_row("  - on EC", served_ec_str);
            std::string served_cc_str = std::to_string(out.devices_served_cc_count) + " (" + utils::toPercentageString(out.devices_served_cc_count, out.devices_served_count) + "% of served)";
            print_row("  - on CC", served_cc_str);

            // --- Block 3: Server Stats ---
            print_midle();
            print_title("SERVERS");
            print_midle();
            int total_servers = in.servers_ec + in.servers_cc;
            print_row("Total", std::to_string(total_servers) + " (" + std::to_string(in.servers_ec) + " EC + " + std::to_string(in.servers_cc) + " CC)");
            std::string servers_used_str = std::to_string(out.servers_used_count) + " (" + utils::toPercentageString(out.servers_used_count, total_servers) + "%)";
            print_row("Used", servers_used_str);
            std::string used_ec_str = std::to_string(out.servers_used_ec_count) + " (" + utils::toPercentageString(out.servers_used_ec_count, in.servers_ec) + "% of EC)";
            print_row("  - Used EC", used_ec_str);
            std::string used_cc_str = std::to_string(out.servers_used_cc_count) + " (" + utils::toPercentageString(out.servers_used_cc_count, in.servers_cc) + "% of CC)";
            print_row("  - Used CC", used_cc_str);
            
            // --- Block 4: Cost Analysis ---
            print_midle();
            print_title("COSTS");
            print_midle();
            print_row("TOTAL COST", utils::toString(out.total_cost, 6));
            print_row("  - Cost Non-Coverage", utils::toString(out.cost_of_non_coverage, 6));
            print_row("  - Cost Non-Service", utils::toString(out.cost_of_non_service, 6));
            print_row("  - Cost Servers Used", utils::toString(out.cost_of_servers_used, 6));

            // --- Block 5: TR-med ---
            print_midle();
            print_row("Avg. Response Time (ms)", utils::toString(out.average_response_time, 4));
        }
    }

    /**
     * @brief [OVERLOAD 1] Displays metrics for Mathematical simulations.
     * @details Renders the common metrics table and appends a section for solver-
     * specific statistics like final status, objective function value, and MIP gap.
     * @param[in] metrics The MathMetrics object to display.
     */
    inline void showMetrics(const MathMetrics& metrics) {
        show_common_metrics(metrics);

        print_midle();
        print_title("SOLVER STATS");
        print_midle();
        print_row("Solver Status", metrics.status);
        print_row("Objective Function (OF)", utils::toString(metrics.OF, 6));
        print_row("MIP Gap", utils::toPercentageString(metrics.gap, 1.0) + "%");
        print_header();
    }

    /**
     * @brief [OVERLOAD 2] Displays metrics for Heuristic simulations.
     * @param[in] metrics The HeuristicMetrics object to display.
     */
    inline void showMetrics(const HeuristicMetrics& metrics) {
        show_common_metrics(metrics);

        print_header();
    }

    /**
     * @brief [OVERLOAD 3] Displays metrics for MetaHeuristic simulations.
     * @details Renders the common metrics table and appends a section for
     * meta-heuristic specific parameters like initial temperature and cooling rate.
     * @param[in] metrics The MetaHeuristicMetrics object to display.
     */
    inline void showMetrics(const MetaHeuristicMetrics& metrics) {
        show_common_metrics(metrics);

        print_midle();
        print_title(metrics.algorithm_name + " PARAMETERS");
        print_midle();
        print_row("Initial Solution", metrics.heuristic_used);
        print_row("Initial Temperature", utils::toString(metrics.temperature, 2));
        print_row("Alpha (Cooling Rate)", utils::toString(metrics.alpha, 2));
        print_header();
    }   
}
