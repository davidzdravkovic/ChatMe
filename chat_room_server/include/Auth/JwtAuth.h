#pragma once

#include <string>

namespace chat_jwt {

struct AuthPrincipal {
    int userId{0};
    std::string userName;
};

/** HS256 JWT when CHAT_ENABLE_JWT is defined (JWT_SECRET). Otherwise dev-style `dev:…` tokens if CHAT_INSECURE_DEV_AUTH or JWT_SECRET is set (secret not used for dev tokens; use CHAT_ENABLE_JWT for real JWT). */
bool verifyToken(const std::string& token, AuthPrincipal& out);

std::string issueAccessToken(int userId, const std::string& userName);

} // namespace chat_jwt
