#include "../include/Deserialization/DeserializeTyping.h"

TypingRequest DeserializeTyping::deserializeTyping(const RequestStruct &reqStruct) {
    TypingRequest typing{};
    const auto& j = reqStruct.data;

    typing.sessionId    = reqStruct.sessionID;
    typing.receiverName = j.value("receiverUser", std::string{});
    typing.senderUserName = j.value("senderUserName", std::string{});
    if (typing.senderUserName.empty() && !reqStruct.authenticatedUserName.empty())
        typing.senderUserName = reqStruct.authenticatedUserName;
    typing.senderId = j.contains("senderId") && !j["senderId"].is_null()
                          ? j["senderId"].get<int>()
                          : reqStruct.authenticatedUserId;
    typing.chatId       = j["chatroom_id"].get<int>();

    if (j.contains("typing")) {
        const auto& val = j["typing"];
        if (val.is_boolean())
            typing.isTyping = val.get<bool>();
        else
            typing.isTyping = val.get<std::string>() == "true";
    } else {
        typing.isTyping = false;
    }

    return typing;
}
