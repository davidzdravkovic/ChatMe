#include "../include/Deserialization/DeserializeChatRetrieve.h"

ChatRetieve DeserializeChatRetrieve::deserializeChatRetrieve(const RequestStruct &reqStruct) {
    ChatRetieve chatRetieve;
    const auto& j = reqStruct.data;

    chatRetieve.sessionId        = reqStruct.sessionID;
    chatRetieve.senderUserName   = j.value("senderUserName", std::string{});
    if (chatRetieve.senderUserName.empty() && !reqStruct.authenticatedUserName.empty())
        chatRetieve.senderUserName = reqStruct.authenticatedUserName;
    chatRetieve.receiverUserName = j.value("receiverUserName", std::string{});

    return chatRetieve;
}
