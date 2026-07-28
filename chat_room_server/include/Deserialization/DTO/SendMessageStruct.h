#ifndef SENDMESSAGESTRUCT_H
#define SENDMESSAGESTRUCT_H
#include <string>

struct SendMessageStruct {
    int sessionId;
    std::string receiverUserName;
    int senderId;
    std::string content;
    int chatRoomId;
    std::string senderUserName;
    int temporaryId;
    int replyToMessageId = 0;
};

#endif