#ifndef DESERIALIZECONFIG_H
#define DESERIALIZECONFIG_H
#include "./AppConfig.h"
#include <filesystem>
#include <string>

class DeserializeConfig {
    std::string getEnv(const char *key);
    std::string readWholeFile(const std::filesystem::path& dir);

public:
    NetworkConfig loadNetwork(const std::filesystem::path& dir);
    DatabaseConfig loadDataBase(const std::filesystem::path& dir);
    JwtConfig loadJwt(const std::filesystem::path& dir);
};

#endif
