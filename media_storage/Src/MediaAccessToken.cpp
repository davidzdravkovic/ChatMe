#include "../include/MediaAccessToken.h"

#include <openssl/evp.h>
#include <openssl/hmac.h>

#include <cctype>
#include <ctime>
#include <cstdlib>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace media_auth {

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

bool envTruthy(const char* v) {
    if (!v || !v[0]) return false;
    const std::string s = trimEnv(v);
    return s == "1" || s == "true" || s == "TRUE" || s == "yes" || s == "YES";
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

std::vector<unsigned char> base64UrlDecode(const std::string& in) {
    std::string s = in;
    for (char& c : s) {
        if (c == '-') c = '+';
        else if (c == '_') c = '/';
    }
    while (s.size() % 4 != 0)
        s.push_back('=');

    std::vector<unsigned char> out((s.size() / 4) * 3);
    const int len = EVP_DecodeBlock(out.data(),
                                    reinterpret_cast<const unsigned char*>(s.data()),
                                    static_cast<int>(s.size()));
    if (len < 0)
        return {};
    std::size_t pad = 0;
    if (!s.empty() && s[s.size() - 1] == '=') pad++;
    if (s.size() > 1 && s[s.size() - 2] == '=') pad++;
    if (static_cast<std::size_t>(len) >= pad)
        out.resize(static_cast<std::size_t>(len) - pad);
    else
        out.clear();
    return out;
}

std::optional<std::string> hmacSha256Base64Url(const std::string& secret, const std::string& message) {
    if (secret.empty())
        return std::nullopt;
    unsigned char digest[EVP_MAX_MD_SIZE];
    unsigned int digestLen = 0;
    if (!HMAC(EVP_sha256(),
              secret.data(),
              static_cast<int>(secret.size()),
              reinterpret_cast<const unsigned char*>(message.data()),
              message.size(),
              digest,
              &digestLen)) {
        return std::nullopt;
    }
    return base64UrlEncode(digest, digestLen);
}

bool constantTimeEqual(const std::string& a, const std::string& b) {
    if (a.size() != b.size())
        return false;
    unsigned char diff = 0;
    for (std::size_t i = 0; i < a.size(); ++i)
        diff |= static_cast<unsigned char>(a[i] ^ b[i]);
    return diff == 0;
}

std::int64_t nowUnix() {
    return static_cast<std::int64_t>(std::time(nullptr));
}

Purpose parsePurpose(const std::string& s) {
    if (s == "read_message") return Purpose::ReadMessage;
    if (s == "read_profile") return Purpose::ReadProfile;
    if (s == "commit_message") return Purpose::CommitMessage;
    if (s == "commit_profile") return Purpose::CommitProfile;
    if (s == "write_temp") return Purpose::WriteTemp;
    if (s == "legacy_commit_profile") return Purpose::LegacyCommitProfile;
    throw std::runtime_error("unknown purpose");
}

std::optional<TokenClaims> parsePayload(const std::string& payload) {
    std::vector<std::string> parts;
    {
        std::istringstream iss(payload);
        std::string part;
        while (std::getline(iss, part, '|'))
            parts.push_back(part);
    }

    if (parts.size() < 5 || parts[0] != "1")
        return std::nullopt;

    TokenClaims c;
    try {
        c.userId = std::stoi(parts[1]);
        c.resourceId = std::stoi(parts[2]);

        if (parts.size() == 5) {
            c.purpose = parsePurpose(parts[3]);
            c.secondaryId = 0;
            c.expUnix = std::stoll(parts[4]);
        } else if (parts.size() == 6) {
            c.secondaryId = std::stoi(parts[3]);
            c.purpose = parsePurpose(parts[4]);
            c.expUnix = std::stoll(parts[5]);
        } else {
            return std::nullopt;
        }
    } catch (...) {
        return std::nullopt;
    }

    if (c.userId <= 0 || c.expUnix <= 0)
        return std::nullopt;
    if (c.expUnix < nowUnix())
        return std::nullopt;

    return c;
}

} // namespace

const char* purposeToString(Purpose p) {
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

bool allowUnsignedRequests() {
    return envTruthy(std::getenv("MEDIA_ALLOW_UNSIGNED"));
}

std::string mediaJwtSecret() {
    return trimEnv(std::getenv("JWT_SECRET"));
}

std::optional<TokenClaims> verifyAccessToken(const std::string& token) {
    if (token.empty())
        return std::nullopt;

    const auto dot = token.find('.');
    if (dot == std::string::npos || dot == 0 || dot + 1 >= token.size())
        return std::nullopt;

    const std::string payloadB64 = token.substr(0, dot);
    const std::string sigB64 = token.substr(dot + 1);

    const auto payloadBytes = base64UrlDecode(payloadB64);
    if (payloadBytes.empty())
        return std::nullopt;

    const std::string payload(payloadBytes.begin(), payloadBytes.end());
    const auto expectedSig = hmacSha256Base64Url(mediaJwtSecret(), payload);
    if (!expectedSig || !constantTimeEqual(*expectedSig, sigB64))
        return std::nullopt;

    return parsePayload(payload);
}

bool authorizeRequest(
    const std::string& accessToken,
    Purpose expectedPurpose,
    int pathPrimaryId,
    int pathSecondaryId) {

    if (allowUnsignedRequests())
        return true;

    if (mediaJwtSecret().empty())
        return false;

    const auto claims = verifyAccessToken(accessToken);
    if (!claims)
        return false;
    if (claims->purpose != expectedPurpose)
        return false;
    if (claims->resourceId != pathPrimaryId)
        return false;

    switch (expectedPurpose) {
    case Purpose::CommitProfile:
    case Purpose::LegacyCommitProfile:
        return claims->secondaryId == pathSecondaryId;
    default:
        return true;
    }
}

} // namespace media_auth
