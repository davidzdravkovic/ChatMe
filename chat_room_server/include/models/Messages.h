#ifndef MESSAGE_H
#define MESSAGE_H
#include "MessageReaction.h"
#include <string>
#include <vector>

struct Message {
    int messageID;     
    int chatRoomID;   
    int senderID;
   //Added just for first message creation cause the CHATROOM ID response needs to carry (for client to fetch avatar with it) 
    int receiverId; 
    std::string senderUserName;
    std::string receiverUserName;
    std::string time;   
    std::string content;
    int clientTempId; 
    int mediaID;
    /** Signed GET path for this viewer, e.g. /media/message/12?token=... */
    std::string mediaUrl;
    /** 0 = not a reply; otherwise parent message in the same chatroom. */
    int replyToMessageId = 0;
    /** Parent snippet for UI; bound by server on fetch/send, not stored in DB. */
    std::string replyPreviewContent;
    int replyPreviewSenderId = 0;
    /** Loaded from message_reactions; not stored on messages row. */
    std::vector<MessageReaction> reactions;
    // Message(const int &messageId,const int &roomId,const int &senderId,const std::string &content) : messageID(messageId) {};
    // Message(){};
};

#endif 