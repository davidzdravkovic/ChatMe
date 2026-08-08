#include "../../include/Auth/JwtAuth.h"

#include <cstdlib>
#include <stdexcept>
#include <string_view>

#ifdef CHAT_ENABLE_JWT
#include <chrono>
#include <jwt-cpp/jwt.h>
#endif


namespace chat_jwt {

namespace {

constexpr const char* kIssuer = "chat_server";

#ifdef CHAT_ENABLE_JWT
std::string jwtSecret() {
    const char* s = std::getenv("JWT_SECRET");
    return s ? std::string(s) : std::string{};
}
#endif

} // namespace

#ifndef CHAT_ENABLE_JWT
/** Dev-style `dev:…` tokens when JWT-cpp is not linked: allow if ops set either env. */
static bool nonJwtTokenModeEnabled() {
    const char* dev = std::getenv("CHAT_INSECURE_DEV_AUTH");
    if (dev && dev[0] != '\0')
        return true;
    const char* secret = std::getenv("JWT_SECRET");
    return secret && secret[0] != '\0';
}
#endif

bool verifyToken(const std::string& token, AuthPrincipal& out) {
    out = {};

#ifndef CHAT_ENABLE_JWT
    if (!nonJwtTokenModeEnabled())
        return false;
    constexpr std::string_view pfx = "dev:";
    if (token.size() <= pfx.size() || token.compare(0, pfx.size(), pfx) != 0)
        return false;
    const std::string rest = token.substr(pfx.size());
    const auto colon = rest.find(':');
    if (colon == std::string::npos)
        return false;
    try {
        out.userId = std::stoi(rest.substr(0, colon));
        out.userName = rest.substr(colon + 1);
        return out.userId > 0;
    } catch (...) {
        return false;
    }
#else
    const std::string secret = jwtSecret();
    if (secret.empty() || token.empty())
        return false;
    try {
        const auto decoded = jwt::decode(token);
        jwt::verify()
            .allow_algorithm(jwt::algorithm::hs256{secret})
            .with_issuer(kIssuer)
            .verify(decoded);

        out.userId = std::stoi(decoded.get_subject());
        const auto& un = decoded.get_payload_claim("uname");
        if (!un.as_string().empty())
            out.userName = un.as_string();
        return out.userId > 0;
    } catch (...) {
        return false;
    }
#endif
}

std::string issueAccessToken(int userId, const std::string& userName) {
#ifndef CHAT_ENABLE_JWT
    if (!nonJwtTokenModeEnabled())
        return {};
    return std::string("dev:") + std::to_string(userId) + ":" + userName;
#else
    const std::string secret = jwtSecret();
    if (secret.empty())
        return {};
    using clock = std::chrono::system_clock;
    const auto now = clock::now();
    return jwt::create()
        .set_issuer(kIssuer)
        .set_type("JWT")
        .set_issued_at(now)
        .set_expires_at(now + std::chrono::hours{24 * 7})
        .set_subject(std::to_string(userId))
        .set_payload_claim("uname", jwt::claim(userName))
        .sign(jwt::algorithm::hs256{secret});
#endif
}

} // namespace chat_jwt
