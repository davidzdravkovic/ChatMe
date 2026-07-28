#include "../../include/Auth/MediaAccessToken.h"

#include <openssl/evp.h>
#include <openssl/hmac.h>

#include <cctype>
#include <cstdlib>
#include <ctime>
#include <sstream>
#include <stdexcept>

namespace media_access {

namespace {

std::string trimEnv(const char* v) {
    if (!v) return {};
    std::string s(v);
    while (!s.empty() && std::isspace(static_cast<unsigned char>(s.front())))
        s.erase(s.begin());
    while (!s.empty() && std::isspace(static_cast<unsigned char>(s.back())))
        s.pop_back();
    return s;
}

std::string jwtSecret() {
    return trimEnv(std::getenv("JWT_SECRET"));
}

std::int64_t nowUnix() {
    return static_cast<std::int64_t>(std::time(nullptr));
}

std::string base64UrlEncode(const unsigned char* data, std::size_t len) {
    static const char* kTable =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string out;
    out.reserve(((len + 2) / 3) * 4);
    for (std::size_t i = 0; i < len; i += 3) {
        const unsigned int b0 = data[i];
        const unsigned int b1 = (i + 1 < len) ? data[i + 1] : 0;
        const unsigned int b2 = (i + 2 < len) ? data[i + 2] : 0;
        const unsigned int n = (b0 << 16) | (b1 << 8) | b2;
        out.push_back(kTable[(n >> 18) & 0x3F]);
        out.push_back(kTable[(n >> 12) & 0x3F]);
        out.push_back(i + 1 < len ? kTable[(n >> 6) & 0x3F] : '=');
        out.push_back(i + 2 < len ? kTable[n & 0x3F] : '=');
    }
    while (!out.empty() && out.back() == '=')
        out.pop_back();
    for (char& c : out) {
        if (c == '+') c = '-';
        else if (c == '/') c = '_';
    }
    return out;
}

std::string hmacSha256Base64Url(const std::string& secret, const std::string& message) {
    unsigned char digest[EVP_MAX_MD_SIZE];
    unsigned int digestLen = 0;
    if (!HMAC(EVP_sha256(),
              secret.data(),
              static_cast<int>(secret.size()),
              reinterpret_cast<const unsigned char*>(message.data()),
              message.size(),
              digest,
              &digestLen)) {
        return {};
    }
    return base64UrlEncode(digest, digestLen);
}

std::string buildPayload(
    int userId,
    int resourceId,
    Purpose purpose,
    int secondaryId,
    int ttlSeconds) {

    const std::int64_t exp = nowUnix() + ttlSeconds;
    std::ostringstream oss;
    oss << "1|" << userId << "|" << resourceId;
    if (secondaryId != 0) {
        oss << "|" << secondaryId;
    }
    oss << "|" << purposeWire(purpose) << "|" << exp;
    return oss.str();
}

} // namespace

const char* purposeWire(Purpose p) {
    switch (p) {
    case Purpose::ReadMessage: return "read_message";
    case Purpose::ReadProfile: return "read_profile";
    case Purpose::CommitMessage: return "commit_message";
    case Purpose::CommitProfile: return "commit_profile";
    case Purpose::WriteTemp: return "write_temp";
    case Purpose::LegacyCommitProfile: return "legacy_commit_profile";
    default: return "unknown";
    }
}

std::string issueToken(
    int userId,
    int resourceId,
    Purpose purpose,
    int secondaryId,
    int ttlSeconds) {

    const std::string secret = jwtSecret();
    if (secret.empty() || userId <= 0 || resourceId <= 0 || ttlSeconds <= 0)
        return {};

    const std::string payload = buildPayload(userId, resourceId, purpose, secondaryId, ttlSeconds);
    const std::string sig = hmacSha256Base64Url(secret, payload);
    if (sig.empty())
        return {};

    return base64UrlEncode(reinterpret_cast<const unsigned char*>(payload.data()), payload.size())
           + "." + sig;
}

std::string messageReadPath(int viewerUserId, int mediaId, int ttlSeconds) {
    if (mediaId <= 0)
        return {};
    const std::string token = issueToken(viewerUserId, mediaId, Purpose::ReadMessage, 0, ttlSeconds);
    if (token.empty())
        return {};
    return "/media/message/" + std::to_string(mediaId) + "?token=" + token;
}

std::string profileReadPath(int viewerUserId, int profileUserId, int ttlSeconds) {
    if (viewerUserId <= 0 || profileUserId <= 0)
        return {};
    const std::string token = issueToken(viewerUserId, profileUserId, Purpose::ReadProfile, 0, ttlSeconds);
    if (token.empty())
        return {};
    return "/media/profile/" + std::to_string(profileUserId) + "?token=" + token;
}

std::string messageCommitPath(int uploaderUserId, int uploadId, int ttlSeconds) {
    if (uploaderUserId <= 0 || uploadId <= 0)
        return {};
    const std::string token = issueToken(uploaderUserId, uploadId, Purpose::CommitMessage, 0, ttlSeconds);
    if (token.empty())
        return {};
    return "/media/message/commit/" + std::to_string(uploadId) + "?token=" + token;
}

std::string profileCommitPath(int uploaderUserId, int uploadId, int profileUserId, int ttlSeconds) {
    if (uploaderUserId <= 0 || uploadId <= 0 || profileUserId <= 0)
        return {};
    const std::string token = issueToken(uploaderUserId, uploadId, Purpose::CommitProfile, profileUserId, ttlSeconds);
    if (token.empty())
        return {};
    return "/media/profile/commit/" + std::to_string(uploadId) + "/"
           + std::to_string(profileUserId) + "?token=" + token;
}

void enrichMessageForViewer(Message& message, int viewerUserId, int ttlSeconds) {
    message.mediaUrl.clear();
    if (message.mediaID <= 0 || viewerUserId <= 0)
        return;
    message.mediaUrl = messageReadPath(viewerUserId, message.mediaID, ttlSeconds);
}

Message messageForViewer(const Message& source, int viewerUserId, int ttlSeconds) {
    Message copy = source;
    enrichMessageForViewer(copy, viewerUserId, ttlSeconds);
    return copy;
}

void enrichAllMessagesForViewer(std::vector<std::vector<Message>>& batches, int viewerUserId, int ttlSeconds) {
    for (auto& batch : batches) {
        for (auto& message : batch)
            enrichMessageForViewer(message, viewerUserId, ttlSeconds);
    }
}

} // namespace media_access
