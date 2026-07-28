#ifndef MAPPER_H
#define MAPPER_H
#include "../Deserialization/DTO/LogStruct.h"
#include "../Deserialization/DTO/CreateStruct.h"
#include "../Deserialization/DTO/SendMessageStruct.h"
#include "../Deserialization/DTO/SendFirstMessageStruct.h"
#include "../models/User.h"

class Mapper {
public:
    static User mapLogStructToUser(const LogStruct &dto) {
        User u;
        u.userName=dto.userName;
        u.password=dto.password;
        return u;
    }

    static User mapCreateStructToUser(const CreateStruct &dto) {
        User u;
        u.userName=dto.userName;
        u.name=dto.name;
        u.password=dto.password;
        u.email=dto.email;
        return u;
    }
        static Message createMessage(const SendMessageStruct &dto) {
        Message message;
        message.content = dto.content;
        message.senderID = dto.senderId;
        message.chatRoomID = dto.chatRoomId;
        message.senderUserName = dto.senderUserName;
        message.receiverUserName = dto.receiverUserName;
        message.clientTempId = dto.temporaryId;
        message.replyToMessageId = dto.replyToMessageId;
        return message;
    }
    static Message createFirstMessageStruct (const SendFirstMessageStruct &dto) {
        Message message;
        message.content = dto.content;
        message.senderID = dto.senderId;
        message.senderUserName = dto.senderUserName;
        message.receiverUserName = dto.receiverUserName;
        message.clientTempId = dto.temporaryId;
        return message;
    }
    
};

#endif