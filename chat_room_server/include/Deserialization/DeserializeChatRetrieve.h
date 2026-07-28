#ifndef DESERIALIZECHATRETRIEVE_H
#define DESERIALIZECHATRETRIEVE_H
#include "./DTO/ChatRetrieve.h"
#include "./DTO/RequestStruct.h"
#include <string>

class DeserializeChatRetrieve {
public:
    ChatRetieve deserializeChatRetrieve(const RequestStruct &reqStruct);
};

#endif
