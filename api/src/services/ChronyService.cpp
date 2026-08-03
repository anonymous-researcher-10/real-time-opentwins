#include "services/ChronyService.h"
#include <fstream>
#include <cstdlib>
#include <iostream>
#include <vector>

bool ChronyService::updateConfiguration(const ChronyConfigDTO& config, std::string& error_message) {
    std::string config_path = "build/tmp/chrony_test.conf";
    std::vector<std::string> config_lines;
    // std::ofstream file("build/tmp/chrony_test.conf", std::ios::out | std::ios::trunc);

    std::ifstream file(config_path);

    if (!file.is_open()) {
        error_message = "Cant open old config file.";
        return false;
    }

    std::string line;
    int updated_fields[4] = {0, 0, 0, 0}; // server, polling_interval, minpoll, maxpoll 

    while (std::getline(file, line)) {
        if (line.find("server") == 0 && config.server.has_value()) {
            line = "server " + config.server.value() + " iburst";
            updated_fields[0] = 1;
            config_lines.push_back(line);
        } else if (line.find("polling_interval") == 0 && config.polling_interval.has_value()) {
            line = "polling_interval " + std::to_string(config.polling_interval.value());
            updated_fields[1] = 1;
            config_lines.push_back(line);
        } else if (line.find("minpoll") == 0 && config.minpoll.has_value()) {
            line = "minpoll " + std::to_string(config.minpoll.value());
            updated_fields[2] = 1;
            config_lines.push_back(line);
        } else if (line.find("maxpoll") == 0 && config.maxpoll.has_value()) {
            line = "maxpoll " + std::to_string(config.maxpoll.value());
            updated_fields[3] = 1;
            config_lines.push_back(line);
        }else{
            config_lines.push_back(line);
        }

    }

    file.close();


    if (config.server.has_value() && updated_fields[0] == 0) {
        line = "";
        config_lines.push_back(line);
        line = "server " + config.server.value() + " iburst";
        config_lines.push_back(line);
    }
    if (config.polling_interval.has_value() && updated_fields[1] == 0) {
        line = "";
        config_lines.push_back(line);
        line = "polling_interval " + std::to_string(config.polling_interval.value());
        config_lines.push_back(line);
    }
    if (config.minpoll.has_value() && updated_fields[2] == 0) {
        line = "";
        config_lines.push_back(line);
        line = "minpoll " + std::to_string(config.minpoll.value());
        config_lines.push_back(line);
    }
    if (config.maxpoll.has_value() && updated_fields[3] == 0) {
        line = "";
        config_lines.push_back(line);
        line = "maxpoll " + std::to_string(config.maxpoll.value());
        config_lines.push_back(line);
    }

    std::ofstream output_file(config_path, std::ios::out | std::ios::trunc);
    if (!output_file.is_open()) {
        error_message = "Cant open new config file.";
        return false;
    }

    for (const auto& l : config_lines) {
        output_file << l << "\n";
    }

    output_file.close();

    return true; // Todo salió perfecto
}

void ChronyService::restartService() {
    std::cout << "Reiniciando servicio de Chrony..." << std::endl;
    // Aquí podrías usar system("systemctl restart chronyd") o similar, dependiendo de tu sistema operativo
}