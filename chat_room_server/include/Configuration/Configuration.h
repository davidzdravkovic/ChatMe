#ifndef CONFIGURATION_H
#define CONFIGURATION_H
#include "./AppConfig.h"
#include "./DeserializeConfig.h"
#include <filesystem>
#include <string>

class Configuration {

std::filesystem::path configDir; 
AppConfig config;
public:
  /** Directory containing DataBase.json, Network.json, Jwt.json (see .cpp for search order). */
  static std::filesystem::path resolveConfigDirectory();

Configuration(const std::filesystem::path& dir): configDir(dir) {};
AppConfig loadConfig();  
    
};

#endif