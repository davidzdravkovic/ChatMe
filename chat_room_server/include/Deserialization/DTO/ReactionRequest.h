#pragma once

#include <string>

struct ReactionRequest {
    int messageId = 0;
    int chatRoomId = 0;
    int userId = 0;
    std::string userName;
    /** Emoji string; empty removes this user's reaction. */
    std::string reaction;
    int sessionId = 0;
};
