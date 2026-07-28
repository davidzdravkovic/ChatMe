#pragma once

#include "./JwtAuth.h"
#include <string>

/**
 * Application auth API: credential verification lives in services; token crypto lives in JwtAuth.
 * Handler composes success responses with issueJWT(); Network may verify tokens on the read path.
 */
struct Auth {
    static std::string issueJWT(int userId, const std::string& userName) {
        return chat_jwt::issueAccessToken(userId, userName);
    }

    static bool verify(const std::string& token, chat_jwt::AuthPrincipal& out) {
        return chat_jwt::verifyToken(token, out);
    }

    static int extractUserId(const std::string& token) {
        chat_jwt::AuthPrincipal p;
        return chat_jwt::verifyToken(token, p) ? p.userId : 0;
    }
};
