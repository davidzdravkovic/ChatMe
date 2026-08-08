#include "../include/Configuration/DeserializeConfig.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cstdlib>

std::string DeserializeConfig::getEnv(const char* key) {
    const char* val = std::getenv(key);
    if (!val) {
        throw std::runtime_error(std::string("Missing env var: ") + key);
    }
    return std::string(val);
}

std::string DeserializeConfig::readWholeFile(const std::filesystem::path& file) {
    std::ifstream in(file);
    if (!in) {
        throw std::runtime_error("Cannot open file: " + file.string());
    }
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

namespace {

/** Use nested object (e.g. { "database": { ... } }) when present; else root (flat JSON). */
const nlohmann::json& objectOrNested(const nlohmann::json& root, const char* nestedKey) {
    if (root.contains(nestedKey) && root[nestedKey].is_object())
        return root[nestedKey];
    return root;
}

} // namespace

DatabaseConfig DeserializeConfig::loadDataBase(const std::filesystem::path& dir) {
    auto j = nlohmann::json::parse(readWholeFile(dir));
    const auto& db = objectOrNested(j, "database");
    DatabaseConfig dbConfig;

    dbConfig.host     = db.value("host", std::string{});
    dbConfig.port     = db.value("port", 0);
    dbConfig.name     = db.value("name", std::string{});
    dbConfig.user     = getEnv("db_user");
    dbConfig.password = getEnv("db_password");

    return dbConfig;
}

NetworkConfig DeserializeConfig::loadNetwork(const std::filesystem::path& dir) {
    auto j = nlohmann::json::parse(readWholeFile(dir));
    const auto& net = objectOrNested(j, "network");
    NetworkConfig netConfig;

    netConfig.ip     = net.value("ip", std::string{});
    netConfig.wsPort = net.value("wsPort", 0);

    return netConfig;
}
