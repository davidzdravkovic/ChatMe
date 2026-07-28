#include "../../include/Network/WsAuthMiddleware.h"
#include "../../include/Auth/Auth.h"
#include <nlohmann/json.hpp>
#include <vector>

namespace ingress_pipeline {

namespace {

using json = nlohmann::json;

void sendErrorCloseSession(
    const std::shared_ptr<Client>& client,
    int sessionId,
    const std::function<void(int sessionId, const std::vector<uint8_t>& payload)>& sendIngressError,
    const char* code,
    const char* message) {
    json err;
    err["request"] = "ERROR_RESPONSE";
    err["data"] = {{"code", code}, {"message", message}};
    const std::string payloadStr = err.dump();
    const std::vector<uint8_t> payload(payloadStr.begin(), payloadStr.end());

    client->setAuthRejectClose(true);
    sendIngressError(sessionId, payload);
}

} // namespace

bool WsAuthMiddleware::authenticate(
    RequestStruct& req,
    int connectionSessionId,
    const std::shared_ptr<Client>& client,
    const std::function<void(int sessionId, const std::vector<uint8_t>& payload)>& sendIngressError) {
    if (!client)
        return false;

    if (req.sessionID != connectionSessionId) {
        sendErrorCloseSession(client, connectionSessionId, sendIngressError, "SESSION_MISMATCH",
                              "SessionId does not match connection");
        return false;
    }

    const bool anonymous =
        (req.rType == RequestType::LOGIN_REQUEST || req.rType == RequestType::CREATE_REQUEST);
    if (anonymous) {
        req.isAuthenticated = false;
        return true;
    }

    if (req.token.empty()) {
        sendErrorCloseSession(client, connectionSessionId, sendIngressError, "UNAUTHORIZED", "missing token");
        return false;
    }

    chat_jwt::AuthPrincipal principal;
    if (!Auth::verify(req.token, principal)) {
        sendErrorCloseSession(client, connectionSessionId, sendIngressError, "AUTH_FAILED",
                              "invalid or expired token");
        return false;
    }

    req.authenticatedUserId = principal.userId;
    req.authenticatedUserName = std::move(principal.userName);
    req.isAuthenticated = true;
    return true;
}

} // namespace ingress_pipeline
