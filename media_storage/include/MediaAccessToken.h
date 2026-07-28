#pragma once

#include <cstdint>
#include <optional>
#include <string>

namespace media_auth {

enum class Purpose {
    ReadMessage,
    ReadProfile,
    CommitMessage,
    CommitProfile,
    WriteTemp,
    LegacyCommitProfile,
};

const char* purposeToString(Purpose p);

/** Canonical pipe form signed by chat_server and verified here (see MediaAccessToken.cpp). */
struct TokenClaims {
    int userId{0};
    int resourceId{0};       // mediaId, uploadId, or profile userId (primary path id)
    int secondaryId{0};      // profile userId on commit_profile; else 0
    Purpose purpose{};
    std::int64_t expUnix{0};
};

/**
 * Verify `access` query token: base64url(payload) + "." + base64url(hmac-sha256).
 * Uses JWT_SECRET (same env var as chat_server).
 */
std::optional<TokenClaims> verifyAccessToken(const std::string& token);

/** True when MEDIA_ALLOW_UNSIGNED=1 (local dev only). */
bool allowUnsignedRequests();

/** Shared secret from JWT_SECRET; empty if unset. */
std::string mediaJwtSecret();

bool authorizeRequest(
    const std::string& accessToken,
    Purpose expectedPurpose,
    int pathPrimaryId,
    int pathSecondaryId = 0);

} // namespace media_auth
