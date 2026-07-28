#ifndef PRESENCESERVICE_H
#define PRESENCESERVICE_H
#include "./SharedContext/UserContext.h"
#include "./NetworkAction/ApplicationResponse.h"
#include "./SharedContext/OnlineUserRegistry.h"
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <mutex>
#include <chrono>
#include <optional>


class PresenceService {
public:
    PresenceService(OnlineUserRegistry &registry) :onlineUsers(registry) {}

    void subscribe(UserId userId, UserId targetUserId);

    void removeClient(int userId);

 
    std::vector<ApplicationResponse> onUserOnline(const UserContext& ctx);
    std::vector<ApplicationResponse> onUserOffline(
        const UserContext& ctx,
        const std::optional<std::string>& lastActiveAt = std::nullopt
    );

private:
    std::vector<ApplicationResponse> buildNotifications_unsafe(
        const UserContext& changedUser,
        bool isOnline,
        const std::optional<std::string>& lastActiveAt = std::nullopt
    );

private:

OnlineUserRegistry &onlineUsers;
mutable std::mutex mtx;
std::unordered_map<UserId, std::unordered_set<UserId>> subscribersByUser;
std::unordered_map<UserId, std::unordered_set<UserId>> subscriptionsBySubscriber;

};

#endif