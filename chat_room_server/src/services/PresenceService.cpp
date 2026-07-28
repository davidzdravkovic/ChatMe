#include "../include/services/PresenceService.h"
#include <iostream>

void PresenceService::subscribe(UserId userId, UserId targetUserId) {
    std::lock_guard<std::mutex> lock(mtx);

    auto user = subscribersByUser.find(targetUserId);
    if(user != subscribersByUser.end()) {
        user->second.insert(userId);
    }

    else {
        subscribersByUser[targetUserId].insert(userId);
    }

    auto self = subscriptionsBySubscriber.find(userId); 
    if(self != subscriptionsBySubscriber.end()) 
    { 
        self->second.insert(targetUserId);
    }
    else {
    subscriptionsBySubscriber[userId].insert(targetUserId);
}
}

void PresenceService::removeClient(int UserID) {
    std::lock_guard<std::mutex> lock(mtx);

auto it = subscriptionsBySubscriber.find(UserID);
if (it == subscriptionsBySubscriber.end())
    return;

for (const auto& watched : it->second) {
    auto wit = subscribersByUser.find(watched);
    if (wit != subscribersByUser.end()) {
        wit->second.erase(UserID);   
        if (wit->second.empty()) {
            subscribersByUser.erase(wit);
        }
    }
}

subscriptionsBySubscriber.erase(it);

}

std::vector<ApplicationResponse> PresenceService::onUserOnline(const UserContext& ctx) {
    std::lock_guard<std::mutex> lock(mtx);
    return buildNotifications_unsafe(ctx, true, std::nullopt);
}

std::vector<ApplicationResponse> PresenceService::onUserOffline(
    const UserContext& ctx,
    const std::optional<std::string>& lastActiveAt)
{
    std::lock_guard<std::mutex> lock(mtx);
    return buildNotifications_unsafe(ctx, false, lastActiveAt);
}

std::vector<ApplicationResponse> PresenceService::buildNotifications_unsafe(
    const UserContext& changedUser,
    bool isOnline,
    const std::optional<std::string>& lastActiveAt)
{

    std::vector<ApplicationResponse> out;

    auto it = subscribersByUser.find(changedUser.userId);
    if (it == subscribersByUser.end()) {
         std::cout<<"no one is subscribed to me!"<<std::endl; 
         return {};
        }
     for(auto userId : it->second) {
         std::optional<UserContext> user = onlineUsers.findByUserId(userId);
         if(!user.has_value()) {
            continue;
         }
         ActiveStatus status;
         status.userName = changedUser.userName;
         status.active = isOnline;
         if (!isOnline)
             status.lastActiveAt = lastActiveAt;

        for (SessionId watcherSid : user->sessionIds) {
            ApplicationResponse response;
            response.type = ResponseType::ACTIVE_STATUS_RESPONSE;
            response.target = UnicastToSession{watcherSid};
            response.payload = ActiveStatusUpdatePayload{status};
            out.push_back(response);
        }
     }

    return out;
}
