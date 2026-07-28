#ifndef SENDFIRSTMESSAGESTRUCT_H
#define SENDFIRSTMESSAGESTRUCT_H
#include <string>

struct SendFirstMessageStruct {
    int sessionId;
    std::string receiverUserName;
    int senderId;
    std::string content;
    std::string senderUserName;
    int temporaryId;
};

#endif