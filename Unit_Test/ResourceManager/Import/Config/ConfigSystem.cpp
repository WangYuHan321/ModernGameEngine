#include "ConfigSystem.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>

void ConfigureManager::LoadConfigure()
{
	std::filesystem::path path("../Configure.json");
	std::ifstream stream(path);
    if (stream.is_open()) {
        std::string line;
        while (std::getline(stream, line)) {
            std::cout << line << std::endl;
        }
        stream.close();
    }
    else {
        std::cerr << "Unable to open file: " << path << std::endl;
    }
}