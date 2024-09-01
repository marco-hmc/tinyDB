#include "../include/Initializer.h"

#include <spdlog/sinks/basic_file_sink.h>

#include <filesystem>
#include <fstream>
#include <iostream>

bool initializeProgram(int argc, char *argv[], std::string &filename, std::string &exportPath) {
    std::ifstream config_file("config.json");
    if (!config_file.is_open()) {
        std::filesystem::path current_path = std::filesystem::current_path();
        std::cerr << "current working dir: " << current_path << '\n';
        std::cerr << "cannot open config file" << '\n';
        return false;
    }

    nlohmann::json config;
    config_file >> config;

    std::string log_file = config["log_file"];
    std::ofstream ofs(log_file, std::ofstream::out | std::ofstream::trunc);
    ofs.close();

    auto file_logger = spdlog::basic_logger_mt("file_logger", log_file);
    spdlog::set_default_logger(file_logger);
    spdlog::set_level(spdlog::level::info);

    if (argc == 1) {
        filename = config["default_filename"];
        exportPath = config["default_export_path"];
    } else {
        filename = argv[1];
        exportPath = argv[2];
    }

    spdlog::info("<filename>: {}, <exportPath>: {}", filename, exportPath);
    return true;
}