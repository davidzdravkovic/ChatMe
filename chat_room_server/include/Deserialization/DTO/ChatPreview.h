#ifndef CHATPREVIEW_H
#define CHATPREVIEW_H
#include <string>
#include <optional>

struct ChatPreview
{
    int chatroomID;
    int senderID;
    bool online = false;
    std::string otherUserName;
    int otherUserId;
    std::string content;
    std::string time;
    /** Signed GET path for other user's avatar, e.g. /media/profile/15?token=... */
    std::string profileUrl;
    std::optional<std::string> lastActiveAt;

};
#endif
