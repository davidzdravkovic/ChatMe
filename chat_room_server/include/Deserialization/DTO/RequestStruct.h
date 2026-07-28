#ifndef REQUESTSTRUCT_H
#define REQUESTSTRUCT_H
#include "RequestType.h"
#include <nlohmann/json.hpp>
#include <string>

struct RequestStruct {
    std::string action;
    int sessionID;
    nlohmann::json data;
    RequestType rType{RequestType::ERROR_REQUEST};
    /** Raw bearer from JSON "token" (set before auth gate). */
    std::string token;
    /** Filled after successful JWT verification (per request, not per connection). */
    int authenticatedUserId{0};
    std::string authenticatedUserName;
    /** True when this request carried a valid JWT (see WsAuthMiddleware). */
    bool isAuthenticated{false};
};
#endif