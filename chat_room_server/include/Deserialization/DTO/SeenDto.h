#ifndef SEENDTOH
#define SEENDTOH

#include <cstdint>
#include <optional>
#include <string>

struct SeenDTO
{
    int sessionId;
    int chatId;
    int lastSeenMessageId;
    int userId;
    std::optional<std::string> seenAt;
};

#endif
