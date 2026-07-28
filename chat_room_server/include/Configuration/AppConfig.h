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

struct JwtConfig {
    /** HS256 signing secret from JWT_SECRET environment variable. */
    std::string secret;
    int accessTokenTtlSeconds{3600};
};

struct AppConfig {
    DatabaseConfig database;
    NetworkConfig network;
    JwtConfig jwt;
};

#endif
