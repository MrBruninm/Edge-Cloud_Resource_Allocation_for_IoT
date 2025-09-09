#pragma once

#include "FileManager.h"
#include "utils.h"

namespace DataGenerator {
    const std::filesystem::path dataPath = std::filesystem::current_path() / "data";
    const std::filesystem::path basePath = dataPath / "baseFiles";

    /**
     * @brief Generates or reads the services data file.
     * @details This function first checks if 'Services.txt' exists in the base path.
     * If the file is found, it is read and its contents are returned immediately.
     * Otherwise, this function generates data for 5 distinct services, each with
     * randomized attributes (PCC, MEM, STO, S_d) and a calculated condition (CND)
     * based on its characteristics. The newly generated data is then written to
     * 'data/baseFiles/Services.txt' to be reused in subsequent runs.
     *
     * @return A 2D vector of strings (`std::vector<std::vector<std::string>>`)
     * containing the service data, including a header row. Returns an empty matrix
     * on error, such as a failure to write the new file.
     */
    inline std::vector<std::vector<std::string>> servicesData() {
        std::filesystem::path servicePath = basePath / "Services.txt";
        if (std::filesystem::exists(servicePath)) {
            return FileManager::read(servicePath.string(), ' ').value_or(std::vector<std::vector<std::string>>{});
        }

        std::vector<std::vector<std::string>> services;
        services.push_back({"#", "CND", "PCC", "PCN", "MEM", "STO", "S_d", "TSK"});

        const int numServices = 5;
        for (int i = 1; i <= numServices; ++i) {
            int tsk = utils::randomNumber(1, 4);
            int pcn = utils::randomNumber(1, 4);
            double s_d = utils::randomNumber(0.00484, 12.0);
            double pcc = 0.0, mem = 0.0, sto = 0.0;

            for (int j = 0; j < tsk; ++j) {
                pcc += utils::randomNumber(0.00001, 2.5);
                mem += utils::randomNumber(0.00001, 2.5);
                sto += utils::randomNumber(0.00001, 15.0);
            }

            double cnd = 0.0;
            if      (pcn == 1) cnd += 3;
            else if (pcn == 2) cnd += 5;
            else if (pcn == 3) cnd += 7;
            else if (pcn == 4) cnd += 9;

            if      (pcc >= 0 && pcc < 2.5)   cnd += 0.3;
            else if (pcc >= 2.5 && pcc < 5)   cnd += 0.5;
            else if (pcc >= 5 && pcc < 7.5)   cnd += 0.7;
            else if (pcc >= 7.5 && pcc <= 10) cnd += 0.9;

            services.push_back({
                utils::toString(i), utils::toString(cnd), utils::toString(pcc),
                utils::toString(pcn), utils::toString(mem), utils::toString(sto),
                utils::toString(s_d), utils::toString(tsk)
            });
        }

        if (!FileManager::write(servicePath.string(), services, ' ')) {
            std::cerr << "Error writing to " << servicePath.string() << std::endl;
            return {};
        }
        return services;
    }

    /**
     * @brief Generates or reads a device data file of a specific size.
     * @details Checks if a file named 'data/devices/Devices_{length}.txt' already exists.
     * If so, it reads and returns its content. Otherwise, it generates a new dataset by
     * reading a base file of 1000 devices and the available services. It then randomly
     * selects 'length' unique devices from the base data and assigns a random service
     * to each one. The new file is saved for future use.
     *
     * @param[in] length The number of devices to include in the file.
     * @return A `std::vector<std::vector<std::string>>` containing the device data with a
     * header. Returns an empty matrix on error (e.g., base files not found).
     */
    inline std::vector<std::vector<std::string>> devicesData(int length) {
        std::filesystem::path deviceFilePath = dataPath / "devices" / ("Devices_" + std::to_string(length) + ".txt");
        if (std::filesystem::exists(deviceFilePath)) {
            return FileManager::read(deviceFilePath.string(), ' ').value_or(std::vector<std::vector<std::string>>{});
        }

        std::vector<std::vector<std::string>> services = servicesData();
        if (services.size() <= 1) {
            std::cerr << "Error: No services data available." << std::endl;
            return {};
        }

        std::filesystem::path baseDevicesPath = basePath / "Devices_1000.txt";
        auto devices1000Opt = FileManager::read(baseDevicesPath.string(), ' ');
        if (!devices1000Opt || devices1000Opt->empty()) {
            std::cerr << "Error: Base device data file not found or is empty." << std::endl;
            return {};
        }
        const std::vector<std::vector<std::string>>& devices1000 = *devices1000Opt;

        std::vector<std::vector<std::string>> devices;
        devices.push_back({"#", "LAT", "LON", "CND", "PCC", "PCN", "MEM", "STO", "S_d", "SVC"});

        std::vector<int> randomIndexes = utils::shuffledRange(0, (int)devices1000.size() - 1);

        for (int i = 0; i < length; ++i) {
            const auto& sourceDeviceRow = devices1000.at(randomIndexes[i]);
            const auto& sourceServiceRow = services.at(utils::randomNumber(1, (int)services.size() - 1));

            devices.push_back({
                utils::toString(i + 1),
                sourceDeviceRow.at(1),  // LAT from source device
                sourceDeviceRow.at(2),  // LON from source device
                sourceServiceRow.at(1), // CND from service
                sourceServiceRow.at(2), // PCC from service
                sourceServiceRow.at(3), // PCN from service
                sourceServiceRow.at(4), // MEM from service
                sourceServiceRow.at(5), // STO from service
                sourceServiceRow.at(6), // S_d from service
                sourceServiceRow.at(0)  // SVC ID from service
            });
        }

        if (!FileManager::write(deviceFilePath.string(), devices, ' ')) {
            std::cerr << "Error writing to " << deviceFilePath.string() << std::endl;
            return {};
        }
        return devices;
    }

    /**
     * @brief Generates or reads a cloud computing (CC) server data file.
     * @details If 'data/servers/CC_{length}.txt' exists, it is read. Otherwise, this
     * function reads the corresponding base file from 'data/baseFiles/CC_{length}.txt',
     * adds a 'T_p' (processing time) column calculated as `12.5 / PCC` for each
     * server, and saves the result to the 'servers' directory for future use.
     *
     * @param[in] length The number of servers, corresponding to the base file name.
     * @return A `std::vector<std::vector<std::string>>` containing the CC server data.
     * Returns an empty matrix on error.
     */
    inline std::vector<std::vector<std::string>> ccData(int length) {
        std::filesystem::path serverFilePath = dataPath / "servers" / ("CC_" + std::to_string(length) + ".txt");
        if (std::filesystem::exists(serverFilePath)) {
            return FileManager::read(serverFilePath.string(), ' ').value_or(std::vector<std::vector<std::string>>{});
        }

        std::filesystem::path sourcePath = basePath / ("CC_" + std::to_string(length) + ".txt");
        auto sourceDataOpt = FileManager::read(sourcePath.string(), ' ');
        if (!sourceDataOpt || sourceDataOpt->empty()) {
            std::cerr << "Error: CC source file not found or is empty: " << sourcePath.string() << std::endl;
            return {};
        }

        std::vector<std::vector<std::string>> cc;
        cc.push_back({"#", "LAT", "LON", "CSC", "PCC", "PCN", "MEM", "STO", "T_p"});
        for (const auto& row : *sourceDataOpt) {
            if (row.at(0) == "#") continue;
            std::vector<std::string> newRow = row;
            newRow.push_back(utils::toString(12.5 / std::stod(row.at(4)))); 
            cc.push_back(newRow);
        }

        if (!FileManager::write(serverFilePath.string(), cc, ' ')) {
            std::cerr << "Error writing to " << serverFilePath.string() << std::endl;
            return {};
        }
        return cc;
    }

    /**
     * @brief Generates or reads an edge computing (EC) server data file.
     * @details If 'data/servers/EC_{length}.txt' exists, it's read. Otherwise, it
     * uses the base EC file and, for each server, randomizes hardware specifications
     * (PCC, PCN, MEM, STO) by selecting from a set of predefined profiles. It then
     * calculates a corresponding cost (CSC) and processing time (T_p) and saves
     * the new data to the 'servers' directory.
     *
     * @param[in] length The number of servers, corresponding to the base file name.
     * @return A `std::vector<std::vector<std::string>>` containing the EC server data.
     * Returns an empty matrix on error.
     */
    inline std::vector<std::vector<std::string>> ecData(int length) {
        std::filesystem::path serverFilePath = dataPath / "servers" / ("EC_" + std::to_string(length) + ".txt");
        if (std::filesystem::exists(serverFilePath)) {
            return FileManager::read(serverFilePath.string(), ' ').value_or(std::vector<std::vector<std::string>>{});
        }

        std::filesystem::path sourcePath = basePath / ("EC_" + std::to_string(length) + ".txt");
        auto sourceDataOpt = FileManager::read(sourcePath.string(), ' ');
        if (!sourceDataOpt || sourceDataOpt->empty()) {
            std::cerr << "Error: EC source file not found or is empty: " << sourcePath.string() << std::endl;
            return {};
        }
        const std::vector<std::vector<std::string>>& sourceData = *sourceDataOpt;

        std::vector<std::vector<std::string>> ec;
        ec.push_back({"#", "LAT", "LON", "CSC","PCC", "PCN", "MEM", "STO", "T_p"});

        for (const auto& s : sourceData) {
            if (s.at(0) == "#") continue;
            int raffle = utils::randomNumber(1, 5);
            std::string pcn, pcc, csc;

            if      (raffle == 1) { pcn = "2";  pcc = "1.6"; csc = "0.00085";}
            else if (raffle == 2) { pcn = "4";  pcc = "2.3"; csc = "0.00097";}
            else if (raffle == 3) { pcn = "6";  pcc = "2.9"; csc = "0.00121";}
            else if (raffle == 4) { pcn = "8";  pcc = "3.0"; csc = "0.00138";}
            else                  { pcn = "10"; pcc = "3.0"; csc = "0.00153";}

            double mem = utils::randomNumber(0.00001, 125.0);
            double sto = utils::randomNumber(0.00001, 1000.0);
            double t_p = 12.5 / std::stod(pcc);

            ec.push_back({
                s.at(0), s.at(1), s.at(2), csc, pcc, pcn,
                utils::toString(mem), utils::toString(sto), utils::toString(t_p)
            });
        }

        if (!FileManager::write(serverFilePath.string(), ec, ' ')) {
            std::cerr << "Error writing to " << serverFilePath.string() << std::endl;
            return {};
        }
        return ec;
    }
}