#include "../include/Deserialization/DeserializeSendMessage.h"

SendMessageStruct DeserializeSendMessage::deserializeMessage(const RequestStruct &reqStruct) {
    SendMessageStruct message;
    const auto& j = reqStruct.data;

    message.sessionId        = reqStruct.sessionID;
    message.senderUserName   = j.value("senderUserName", std::string{});
    if (message.senderUserName.empty() && !reqStruct.authenticatedUserName.empty())
        message.senderUserName = reqStruct.authenticatedUserName;
    message.receiverUserName = j.value("receiverUserName", std::string{});
    message.content          = j.value("content", std::string{});
    message.senderId = j.contains("senderId") && !j["senderId"].is_null()
                           ? j["senderId"].get<int>()
                           : 0; // filled from JWT in Handler when using deserialize<> path
    message.temporaryId      = j["temporaryId"].get<int>();
    message.replyToMessageId = j.value("replyToMessageId", 0);

    if (reqStruct.rType == RequestType::MESSAGE_REQUEST)
        message.chatRoomId = j["chatroom_id"].get<int>();
    return message;

}
