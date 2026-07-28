#include "../include/SharedContext/OnlineUserRegistry.h"
#include <chrono>
#include <iostream>

static auto now() {
    return std::chrono::steady_clock::now();
}

bool OnlineUserRegistry::add(SessionId sid, UserId userId, Username userName) {
    std::lock_guard<std::mutex> lock(mtx);

    //Even could not happen but is defensive mechanism to avoid two different Clients to share same session id 
    if (auto bound = userIdBySession.find(sid); bound != userIdBySession.end() && bound->second != userId) {
        if (auto previousUser = usersById.find(bound->second); previousUser != usersById.end()) {
            previousUser->second.sessionIds.erase(sid);
            if (previousUser->second.sessionIds.empty()) {
                userIdByUsername.erase(previousUser->second.userName);
                usersById.erase(previousUser);
            }
        }
    }

    userIdBySession[sid] = userId;

  
    const auto timestamp = now();
    auto existing = usersById.find(userId);

    //User first create or log in
    if (existing == usersById.end()) {
        userIdByUsername[userName] = userId;
        UserContext ctx;
        ctx.userId = userId;
        ctx.userName = std::move(userName);
        ctx.sessionIds.insert(sid);
        ctx.connectedAt = timestamp;
        ctx.lastActivity = timestamp;
        ctx.online = true;
        usersById[userId] = std::move(ctx);
     
        return true;
    }
   // New session for an existing online user (second tab / reconnect).
    UserContext& user = existing->second;
    user.sessionIds.insert(sid);
    user.lastActivity = timestamp;
    user.online = true;
    return false;
}

bool OnlineUserRegistry::removeBySession(SessionId sid) {
    std::lock_guard<std::mutex> lock(mtx);

    auto itSession = userIdBySession.find(sid);
    if (itSession == userIdBySession.end())
        return false;

    const UserId uid = itSession->second;
    userIdBySession.erase(itSession);

    auto itCtx = usersById.find(uid);
    if (itCtx == usersById.end())
        return false;

    itCtx->second.sessionIds.erase(sid);
    if (!itCtx->second.sessionIds.empty())
        return false;

    userIdByUsername.erase(itCtx->second.userName);
    usersById.erase(itCtx);
    return true;
}

std::vector<SessionId> OnlineUserRegistry::findSessionIdsByUsername(const Username& name) const {
    std::lock_guard<std::mutex> lock(mtx);

    auto userId = userIdByUsername.find(name);
    if (userId == userIdByUsername.end())
        return {};

    auto userContext = usersById.find(userId->second);
    if (userContext == usersById.end())
        return {};

    return std::vector<SessionId>(userContext->second.sessionIds.begin(), userContext->second.sessionIds.end());
}


std::vector<SessionId> OnlineUserRegistry::findSessionIdsByUserId(UserId userId) const {
    std::lock_guard<std::mutex> lock(mtx);

    auto userContext = usersById.find(userId);
    if (userContext == usersById.end())
        return {};

    return std::vector<SessionId>(userContext->second.sessionIds.begin(), userContext->second.sessionIds.end());
}

std::optional<UserContext> OnlineUserRegistry::findBySession(SessionId sid) const {
    std::lock_guard<std::mutex> lock(mtx);

    auto it = userIdBySession.find(sid);
    if (it == userIdBySession.end())
        return std::nullopt;

    auto userContext = usersById.find(it->second);
    if (userContext == usersById.end())
        return std::nullopt;

    if (userContext->second.sessionIds.find(sid) == userContext->second.sessionIds.end())
        return std::nullopt;

    return userContext->second;
}

std::optional<UserContext> OnlineUserRegistry::findByUserId(UserId userId) const {
    std::lock_guard<std::mutex> lock(mtx);

    auto user = usersById.find(userId);
    if (user == usersById.end())
        return std::nullopt;

    return user->second;
}

bool OnlineUserRegistry::isOnline(const Username& name) const {
    std::lock_guard<std::mutex> lock(mtx);

    auto it = userIdByUsername.find(name);
    if (it == userIdByUsername.end())
        return false;

    auto userContext = usersById.find(it->second);
    if (userContext == usersById.end())
        return false;

    return !userContext->second.sessionIds.empty();
}
