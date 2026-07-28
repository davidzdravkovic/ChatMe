#ifndef DESERIALIZESENDMESSAGE_H
#define DESERIALIZESENDMESSAGE_H
#include "./DTO/SendMessageStruct.h"
#include "./DTO/RequestStruct.h"
#include <string>

class DeserializeSendMessage {
public:
    SendMessageStruct deserializeMessage(const RequestStruct &reqStruct);
};

#endif
