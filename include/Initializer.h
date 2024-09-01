#pragma once

#include <spdlog/spdlog.h>

#include <nlohmann/json.hpp>
#include <string>

bool initializeProgram(int argc, char *argv[], std::string &filename, std::string &exportPath);
