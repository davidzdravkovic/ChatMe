#pragma once

#include "MediaAccessToken.h"
#include "httplib.h"

#include <stdexcept>
#include <string>

namespace media_auth {

/** Query param: `token` or `access` (HMAC token from chat_server). */
inline std::string accessTokenFromRequest(const httplib::Request& req) {
    if (req.has_param("token"))
        return req.get_param_value("token");
    if (req.has_param("access"))
        return req.get_param_value("access");
    return {};
}

inline int parsePathId(const std::string& raw) {
    return std::stoi(raw);
}

inline void set_cors(httplib::Response& res) {
    res.set_header("Access-Control-Allow-Origin", "*");
    res.set_header("Access-Control-Allow-Methods", "GET, PUT, POST, OPTIONS");
    res.set_header("Access-Control-Allow-Headers", "Content-Type");
    res.set_header("Access-Control-Max-Age", "86400");
}

/** Returns false when handler should return immediately (401 + CORS set). */
inline bool requireMediaAccess(
    const httplib::Request& req,
    httplib::Response& res,
    Purpose purpose,
    int pathPrimaryId,
    int pathSecondaryId = 0) {

    set_cors(res);

    try {
        if (authorizeRequest(accessTokenFromRequest(req), purpose, pathPrimaryId, pathSecondaryId))
            return true;
    } catch (const std::exception&) {
        res.status = 401;
        res.set_content("Unauthorized", "text/plain");
        return false;
    }

    res.status = 401;
    res.set_content("Unauthorized", "text/plain");
    return false;
}

} // namespace media_auth
