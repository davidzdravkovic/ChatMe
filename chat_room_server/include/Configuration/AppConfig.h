#ifndef APPCONFIG_H
#define APPCONFIG_H

#include <string>

struct DatabaseConfig {
    std::string host;
    std::string user;
    std::string name;
    std::string password;
    int port{};
};

struct NetworkConfig {
    std::string ip;
    int wsPort{};
};

struct AppConfig {
    DatabaseConfig database;
    NetworkConfig network;
};

#endif
