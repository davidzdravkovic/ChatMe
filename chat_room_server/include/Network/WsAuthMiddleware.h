#pragma once

#include "../Deserialization/DTO/RequestStruct.h"
#include "./Client.h"
#include <functional>
#include <memory>
#include <vector>

namespace ingress_pipeline {

/**
 * WebSocket ingress auth: SessionId must match the connection on every request (including
 * LOGIN_REQUEST / CREATE_REQUEST). Non-anonymous requests then require JWT: Auth::verify
 * fills authenticatedUserId / authenticatedUserName; on verify failure, code AUTH_FAILED.
 * LOGIN_REQUEST / CREATE_REQUEST skip JWT (isAuthenticated left false).
 * On failure: enqueues ERROR_RESPONSE via writeWs; disconnect when send queue drains and logOutPending.
 */
struct WsAuthMiddleware {
    static bool authenticate(
        RequestStruct& req,
        int connectionSessionId,
        const std::shared_ptr<Client>& client,
        const std::function<void(int sessionId, const std::vector<uint8_t>& payload)>& sendIngressError);
};

} // namespace ingress_pipeline
