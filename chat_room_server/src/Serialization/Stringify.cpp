#include "../include/Serialization/Stringify.h"
#include "../include/models/Messages.h"
#include <nlohmann/json.hpp>
#include <iostream>
#include <string>

namespace {

void appendReplyFields(std::vector<std::pair<std::string, std::string>>& pair, const Message& m) {
    if (m.replyToMessageId <= 0)
        return;
    pair.push_back({"replyToMessageId", std::to_string(m.replyToMessageId)});
    pair.push_back({"replyPreviewContent", m.replyPreviewContent});
    pair.push_back({"replyPreviewSenderId", std::to_string(m.replyPreviewSenderId)});
}

void appendReactionFields(std::vector<std::pair<std::string, std::string>>& pair, const Message& m) {
    nlohmann::json arr = nlohmann::json::array();
    for (const MessageReaction& r : m.reactions) {
        arr.push_back({
            {"userId", r.userId},
            {"reaction", r.reaction},
        });
    }
    pair.push_back({"reactions", arr.dump()});
}

} // namespace

// JWT on wire: only LoginSuccessPayload / CreateSuccessPayload .accessToken (filled in Handler), emitted as "token" below.

std::vector<std::pair<std::string, std::string>> Stringify :: transform(const LoginSuccessPayload& payload) {
  std::vector<std::pair<std::string, std::string>> pair  = {{"name",payload.user.name },
                                                             {"userName",payload.user.userName},
                                                             {"userId", std::to_string(payload.user.userID)},
                                                                  {"email", payload.user.email}
                                                                                         };
  if (!payload.accessToken.empty())
      pair.push_back({"token", payload.accessToken});
  if (!payload.profileUrl.empty())
      pair.push_back({"profileUrl", payload.profileUrl});

 return pair;

}
std::vector<std::pair<std::string, std::string>>
Stringify::transform(const LoginFailurePayload& payload) {
    return {
        { "error", payload.reason }
    };
}

std::vector<std::pair<std::string, std::string>>
Stringify::transform(const AuthSuccessPayload& payload) {
    std::vector<std::pair<std::string, std::string>> pair = {
        {"userName", payload.userName},
        {"userId", std::to_string(payload.userId)},
    };
    if (!payload.accessToken.empty())
        pair.push_back({"token", payload.accessToken});
    if (!payload.profileUrl.empty())
        pair.push_back({"profileUrl", payload.profileUrl});
    return pair;
}

std::vector<std::pair<std::string, std::string>>
Stringify::transform(const AuthFailurePayload& payload) {
    return {
        {"error", payload.reason}
    };
}


std::vector<std::pair<std::string, std::string>>
Stringify::transform(const FirstMessageFailurePayload& payload) {
    return {
        { "error", payload.reason }
    };
}
std::vector<std::pair<std::string, std::string>> Stringify::transform(const CreateSuccessPayload& payload) {
  std::vector<std::pair<std::string, std::string>> pair  = {{"name",payload.user.name },
                                                             {"userName",payload.user.userName},
                                                             {"userId", std::to_string(payload.user.userID)},
                                                                  {"email", payload.user.email}
                                                                                         };
  if (!payload.accessToken.empty())
      pair.push_back({"token", payload.accessToken});

 return pair;
}

std::vector<std::pair<std::string, std::string>>
Stringify::transform(const CreateFailurePayload& payload) {
    return {
        { "error", payload.reason }
    };
}
 std::vector<MessageString> Stringify::transform( const FetchMessagesSuccessPayload& payload) {
    std::vector<MessageString> messagesString;

    //  META OBJECT (FIRST ELEMENT — NOT A MESSAGE)
    if (payload.lastSeenMessageId.has_value()) {
        MessageString meta;
        meta.pair = {
            {"last_seen_message_id",
             std::to_string(*payload.lastSeenMessageId)}
        };
        if (payload.seenAt.has_value())
            meta.pair.push_back({"seen_at", *payload.seenAt});
        messagesString.push_back(meta);
    }
      if (payload.lastSeenIdByOther.has_value()) {
        MessageString meta;
        meta.pair = {
            {"last_seen_message_id_by_other",
             std::to_string(*payload.lastSeenIdByOther)}
        };
        if (payload.seenAtByOther.has_value())
            meta.pair.push_back({"seen_at_by_other", *payload.seenAtByOther});
        messagesString.push_back(meta);
    }
    //RECEIVER INFO (SECOND ELEMENT — ALSO NOT A MESSAGE)l
   MessageString receiverInfo;
   receiverInfo.pair = {
         {"chatIdentifier", std::to_string(payload.identifier)}
    };
    messagesString.push_back(receiverInfo);
    
    MessageString peerUserid;
    peerUserid.pair = {
         {"otherUserId", std::to_string(payload.otherUserId)}
    };
    if (!payload.peerProfileUrl.empty())
        peerUserid.pair.push_back({"profileUrl", payload.peerProfileUrl});
    messagesString.push_back(peerUserid);

    //  REAL MESSAGES (UNCHANGED) - INITIAL
    for (int i = 0; i < payload.messages[0].size(); i++) {
        MessageString newMessage;
        newMessage.pair = {
            {"Content", payload.messages[0][i].content},
            {"SenderId", std::to_string(payload.messages[0][i].senderID)},
            {"Sender", payload.messages[0][i].senderUserName},
            {"chatroom_id", std::to_string(payload.messages[0][i].chatRoomID)},
            {"messageId", std::to_string(payload.messages[0][i].messageID)},
            {"mediaId",std::to_string(payload.messages[0][i].mediaID)},
            {"Time", payload.messages[0][i].time}
        };
        if (!payload.messages[0][i].mediaUrl.empty())
            newMessage.pair.push_back({"mediaUrl", payload.messages[0][i].mediaUrl});
        appendReplyFields(newMessage.pair, payload.messages[0][i]);
        appendReactionFields(newMessage.pair, payload.messages[0][i]);
        messagesString.push_back(newMessage);
    }
    //Barier from initial messages and last message
   MessageString barier;
   barier.pair = {
         {"endOfInitialSize", std::to_string(messagesString.size())}
    };
    messagesString.push_back(barier);

    if(payload.messages.size() == 2) {
    //Last messages 
       for (int i = 0; i < payload.messages[1].size(); i++) {
        MessageString lastMessages;
        lastMessages.pair = {
            {"Content", payload.messages[1][i].content},
            {"SenderId", std::to_string(payload.messages[1][i].senderID)},
            {"Sender", payload.messages[1][i].senderUserName},
            {"chatroom_id", std::to_string(payload.messages[1][i].chatRoomID)},
            {"messageId", std::to_string(payload.messages[1][i].messageID)},
            {"mediaId",std::to_string(payload.messages[1][i].mediaID)},
            {"Time", payload.messages[1][i].time}
        };
        if (!payload.messages[1][i].mediaUrl.empty())
            lastMessages.pair.push_back({"mediaUrl", payload.messages[1][i].mediaUrl});
        appendReplyFields(lastMessages.pair, payload.messages[1][i]);
        appendReactionFields(lastMessages.pair, payload.messages[1][i]);
        messagesString.push_back(lastMessages);
    }
}


    return messagesString;
}
std::vector<std::pair<std::string, std::string>>
Stringify::transform(const FetchMessagesFailurePayload& payload) {
    return {
        { "error", payload.reason }
    };
}

 std::vector<std::pair<std::string, std::string>> Stringify :: transform(const SendMessageSuccessPayload& payload) {
  std::vector<std::pair<std::string, std::string>> pair  = {  {"Content", payload.message.content},
                                                              {"SenderId",std::to_string(payload.message.senderID)},
                                                              {"Sender", payload.message.senderUserName},
                                                              {"ReceiverUserName", payload.message.receiverUserName},
                                                              {"receiverUserName", payload.message.receiverUserName},
                                                              {"receiver_id", std::to_string(payload.message.receiverId)},
                                                              {"chatroom_id", std::to_string(payload.message.chatRoomID)},
                                                              {"messageId",std::to_string(payload.message.messageID)},
                                                              {"mediaId",std::to_string(payload.message.mediaID)},
                                                              {"Time",payload.message.time} };
  if (!payload.message.mediaUrl.empty())
      pair.push_back({"mediaUrl", payload.message.mediaUrl});
  appendReplyFields(pair, payload.message);
  appendReactionFields(pair, payload.message);

 return pair;

}
    std::vector<std::pair<std::string, std::string>> Stringify :: transform(const SendMessageAckPayload& payload) {
        std::vector<std::pair<std::string, std::string>> pair  = {  
                                                                    {"Content", payload.message.content},
                                                                    {"SenderId",std::to_string(payload.message.senderID)},
                                                                    {"Sender", payload.message.senderUserName},
                                                                    {"ReceiverUserName", payload.message.receiverUserName},
                                                                    {"chatroom_id", std::to_string(payload.message.chatRoomID)},                                                                                                                        {"receiverUserName", payload.message.receiverUserName},
                                                                    {"clientId",std::to_string(payload.message.clientTempId)},
                                                                    {"temporaryId",std::to_string(payload.message.clientTempId)},
                                                                    {"messageId",std::to_string(payload.message.messageID)},
                                                                    {"mediaId",std::to_string(payload.message.mediaID)},
                                                                    {"Time",payload.message.time} };
    if (!payload.message.mediaUrl.empty())
        pair.push_back({"mediaUrl", payload.message.mediaUrl});
    appendReplyFields(pair, payload.message);
    appendReactionFields(pair, payload.message);

    return pair;

    }
std::vector<std::pair<std::string, std::string>>
Stringify::transform(const SendMessageFailurePayload& payload) {
    return {
        { "error", payload.reason }
    };
}

 std::vector<std::pair<std::string, std::string>> Stringify :: transform(const ChatRoomIdSuccessPayload &roomid) {
  std::vector<std::pair<std::string, std::string>> pair  = {
                                                            {"chatroom_id", std::to_string(roomid.chatRoomId)},
                                                             {"receiver_id", std::to_string(roomid.receiverId)},
                                                             {"receiver_UserName", roomid.receiver_UserName} 
                                                               
                                                             };


 return pair;

}
std::vector<std::pair<std::string, std::string>>
Stringify::transform(const ChatRoomIdFailurePayload& payload) {
    return {
        { "error", payload.reason }
    };
}

std::vector<std::pair<std::string, std::string>>
Stringify::transform(const PeerUserNotFoundPayload& payload) {
    return {
        { "error", payload.reason },
        { "peerUsername", payload.peerUsername },
        { "chatIdentifier", std::to_string(payload.chatIdentifier) },
    };
}

std::vector<ChatPreviewString> Stringify :: transform(const RecentChatRoomsSuccessPayload& payload) {
    std::vector<ChatPreviewString> chatPreviewsString;
    if (!payload.selfProfileUrl.empty()) {
        ChatPreviewString selfMeta;
        selfMeta.pair = {{"selfProfileUrl", payload.selfProfileUrl}};
        chatPreviewsString.push_back(selfMeta);
    }
    for(int i=0; i<payload.rooms.size(); i++) {
        ChatPreviewString newChat;
        newChat = {{
            {"other_userId", std::to_string(payload.rooms[i].otherUserId)},
            {"chatroom_id", std::to_string(payload.rooms[i].chatroomID)},
            {"other_username", payload.rooms[i].otherUserName},
            {"content", payload.rooms[i].content},
            {"time", payload.rooms[i].time},
            {"online", payload.rooms[i].online ? "true" : "false"}
        }};
        if (!payload.rooms[i].profileUrl.empty())
            newChat.pair.push_back({"profileUrl", payload.rooms[i].profileUrl});
        if (payload.rooms[i].lastActiveAt.has_value())
            newChat.pair.push_back({"last_active_at", *payload.rooms[i].lastActiveAt});
        chatPreviewsString.push_back(newChat);
    }
    return chatPreviewsString;
}
std::vector<std::pair<std::string, std::string>>
Stringify::transform(const RecentChatRoomsFailurePayload& payload) {
    return {
        { "error", payload.reason }
    };
}
std::vector<std::pair<std::string, std::string>>
Stringify::transform(const FirstMessageSuccessPayload& payload) {
    std::vector<std::pair<std::string, std::string>> pair  = {
                                                              {"Content", payload.message.content},
                                                              {"SenderId",std::to_string(payload.message.senderID)},
                                                              {"Sender", payload.message.senderUserName},
                                                              {"chatroom_id", std::to_string(payload.message.chatRoomID)},
                                                              {"messageId",std::to_string(payload.message.messageID)},
                                                              {"Time",payload.message.time} };
    appendReplyFields(pair, payload.message);
    appendReactionFields(pair, payload.message);

 return pair;
}

std::vector<std::pair<std::string, std::string>>
Stringify::transform(const ReactionSuccessPayload& payload) {
    return {
        {"messageId", std::to_string(payload.messageId)},
        {"chatroom_id", std::to_string(payload.chatRoomId)},
        {"userId", std::to_string(payload.userId)},
        {"reaction", payload.reaction},
    };
}

std::vector<std::pair<std::string, std::string>>
Stringify::transform(const ReactionFailurePayload& payload) {
    return {
        {"error", payload.reason},
    };
}

std::vector<std::pair<std::string, std::string>>
Stringify::transform(const ActiveStatusUpdatePayload& payload) {
    std::vector<std::pair<std::string, std::string>> pair ={ {"userName", payload.status.userName},
                                                            {"status",payload.status.active ? "true" : "false" }
                                                             };
    if (!payload.status.active && payload.status.lastActiveAt.has_value())
        pair.push_back({"last_active_at", *payload.status.lastActiveAt});
        return pair;
}


std::vector<std::pair<std::string, std::string>>
Stringify::transform(const ActiveStatusUpdateFailurePayload& payload) {
    return {
        { "error", payload.error }
    };
}

std::vector<std::pair<std::string, std::string>>
Stringify::transform(const UploadProfilePicturePayload& payload) {
    std::vector<std::pair<std::string, std::string>> out = {
        {"uploadId", std::to_string(payload.UploadPictureResponse.uploadId)},
        {"approved", payload.UploadPictureResponse.approved ? "true" : "false"},
    };
    if (!payload.UploadPictureResponse.commitUrl.empty())
        out.push_back({"commitUrl", payload.UploadPictureResponse.commitUrl});
    if (!payload.UploadPictureResponse.profileUrl.empty())
        out.push_back({"profileUrl", payload.UploadPictureResponse.profileUrl});
    return out;
}
std::vector<std::pair<std::string, std::string>>
Stringify::transform(const UploadProfilePictureFailurePayload& payload) {
    return {
        {"approved", payload.UploadPictureResponse.approved ? "true" : "false"},
         {"error", payload.UploadPictureResponse.error}
    };
}

std::vector<std::pair<std::string, std::string>>
Stringify::transform(const TypingPayload& payload) {
    return { {"senderUserName", payload.typing.senderUserName},
              {"isTyping", payload.typing.isTyping? "true" : "false"},
               {"senderId",std::to_string(payload.typing.senderId)},
              {"chatRoomId",std::to_string(payload.typing.chatId)}
    };
}
std::vector<std::pair<std::string, std::string>>
Stringify::transform(const TypingFailurePayload& payload) {
    return {
         {"error", payload.error}
    };
}

std::vector<std::pair<std::string, std::string>>
Stringify::transform(const SeenSuccessPayload& payload) {
    std::vector<std::pair<std::string, std::string>> out = {
        {"chatroom_id",std::to_string(payload.seen.chatId)},
        {"last_seen_message_id",std::to_string(payload.seen.lastSeenMessageId)}
    };
    if (payload.seen.seenAt.has_value())
        out.push_back({"seen_at", *payload.seen.seenAt});
    return out;
}

std::vector<std::pair<std::string, std::string>>
Stringify::transform(const UploadImageMessagePayload& payload) {
    std::vector<std::pair<std::string, std::string>> out = {
        {"uploadId", std::to_string(payload.uploadImageMessageResponse.uploadId)},
        {"approved", payload.uploadImageMessageResponse.approved ? "true" : "false"},
        {"clientId", std::to_string(payload.uploadImageMessageResponse.clientTempId)},
        {"messageId", std::to_string(payload.uploadImageMessageResponse.messageId)},
    };
    if (!payload.uploadImageMessageResponse.commitUrl.empty())
        out.push_back({"commitUrl", payload.uploadImageMessageResponse.commitUrl});
    if (!payload.uploadImageMessageResponse.error.empty())
        out.push_back({"error", payload.uploadImageMessageResponse.error});
    return out;
}

std::vector<FetchImagesIdString> Stringify::transform( const FetchImagesIdPayload& payload) {
    std::vector<FetchImagesIdString> imagesId;

    // META OBJECT (FIRST ELEMENT — NOT AN IMAGE ROW)
    FetchImagesIdString meta;
    meta.pair = {
        {"chatIdentifier", std::to_string(payload.chatIdentifier)}
    };
    imagesId.push_back(meta);

    // IMAGE ROWS
    for (int i = 0; i < payload.imagesId.size(); i++) {
        FetchImagesIdString imageId;
        imageId.pair = {
            {"imageId", std::to_string(payload.imagesId[i])}
        };
        if (i < static_cast<int>(payload.imageMediaUrls.size()) && !payload.imageMediaUrls[i].empty())
            imageId.pair.push_back({"mediaUrl", payload.imageMediaUrls[i]});
        imagesId.push_back(imageId);
    }

    return imagesId;
}

std::vector<MessageString> Stringify::transform(const MessageSearchSuccessPayload& payload) {
    std::vector<MessageString> rows;

    // META OBJECT (FIRST ELEMENT — echoes the client query id for stale filtering)
    MessageString meta;
    meta.pair = {
        {"searchQueryId", std::to_string(payload.searchQueryId)},
    };
    rows.push_back(meta);

    for (const auto& m : payload.hits) {
        MessageString row;
        row.pair = {
            {"messageId", std::to_string(m.messageID)},
            {"chatroom_id", std::to_string(m.chatRoomID)},
            {"Content", m.content},
            {"SenderId", std::to_string(m.senderID)},
            {"Time", m.time},
        };
        rows.push_back(row);
    }
    return rows;
}

std::vector<FetchImagesIdString> Stringify::transform(const SearchQuerySuccessPayload& payload) {
    std::vector<FetchImagesIdString> rows;
    FetchImagesIdString meta;
    meta.pair = {
        {"searcherQueryId", std::to_string(payload.searcherQueryId)},
    };
    rows.push_back(meta);
    for (const auto& h : payload.hits) {
        FetchImagesIdString row;
        row.pair = {
            {"other_username", h.userName},
            {"other_userId", std::to_string(h.userId)},
        };
        if (!h.profileUrl.empty())
            row.pair.push_back({"profileUrl", h.profileUrl});
        rows.push_back(row);
    }
    return rows;
}