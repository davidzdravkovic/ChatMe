#pragma once

#include "../models/Messages.h"

#include <cstdint>
#include <string>
#include <vector>

namespace media_access {

/** Must match media_storage MediaAccessToken payload + HMAC format. */
enum class Purpose {
    ReadMessage,
    ReadProfile,
    CommitMessage,
    CommitProfile,
    WriteTemp,
    LegacyCommitProfile,
};

const char* purposeWire(Purpose p);

/** Signed query token: base64url(payload) + "." + base64url(hmac-sha256). Uses JWT_SECRET. */
std::string issueToken(
    int userId,
    int resourceId,
    Purpose purpose,
    int secondaryId = 0,
    int ttlSeconds = 900);

std::string messageReadPath(int viewerUserId, int mediaId, int ttlSeconds = 900);

/** GET /media/profile/{profileUserId}?token=... (purpose read_profile). */
std::string profileReadPath(int viewerUserId, int profileUserId, int ttlSeconds = 900);

/** POST /media/message/commit/{uploadId}?token=... (purpose commit_message). */
std::string messageCommitPath(int uploaderUserId, int uploadId, int ttlSeconds = 900);

/** POST /media/profile/commit/{uploadId}/{userId}?token=... (purpose commit_profile). */
std::string profileCommitPath(int uploaderUserId, int uploadId, int profileUserId, int ttlSeconds = 900);

/** Sets message.mediaUrl when mediaID > 0. */
void enrichMessageForViewer(Message& message, int viewerUserId, int ttlSeconds = 900);

Message messageForViewer(const Message& source, int viewerUserId, int ttlSeconds = 900);

void enrichAllMessagesForViewer(std::vector<std::vector<Message>>& batches, int viewerUserId, int ttlSeconds = 900);

} // namespace media_access
