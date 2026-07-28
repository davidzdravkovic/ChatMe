#pragma once

#include <cstdint>
#include <string>
using UserId = int;
using ChatId = int;


struct TypingRequest {
    ChatId chatId;        
    UserId senderId; 
    std::string senderUserName;
    std::string receiverName;    
    bool isTyping;     
    int sessionId;
};
