#ifndef USERCONTEXT_H
#define USERCONTEXT_H

#include <cstdint>
#include <string>
#include <chrono>
#include <unordered_set>

using SessionId = int;
using Username  = std::string;
using UserId    = int;

struct UserContext {
    UserId userId;
    Username userName;
    std::unordered_set<SessionId> sessionIds;
    std::chrono::steady_clock::time_point connectedAt;
    std::chrono::steady_clock::time_point lastActivity;
    bool online;
};

#endif
