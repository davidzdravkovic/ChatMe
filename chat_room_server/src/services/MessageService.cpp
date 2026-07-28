#include "../include/services/MessageService.h"
#include "../include/Auth/MediaAccessToken.h"
#include <chrono>

static auto now() {
    return std::chrono::steady_clock::now();
}


static ApplicationResponse senderSessionSyncMessage(const Message& stored, const std::string& senderUserName) {
    ApplicationResponse sync{};
    sync.type = ResponseType::MESSAGE_RESPONSE;
    sync.target = FanOutToUser{senderUserName};
    sync.payload = SendMessageSuccessPayload{
    media_access::messageForViewer(stored, stored.senderID)};
    return sync;
}

std::vector<ApplicationResponse> MessageService::sendMessage(Message &mess, int issuerSessionId) {
    std::vector<ApplicationResponse> responses;

     std::string  senderUserName  = mess.senderUserName;
     std::string  receiverUserName  = mess.receiverUserName;

    Message stored = manager.sendingMessages(mess);
    stored.senderUserName = senderUserName;
    stored.receiverUserName = receiverUserName;
    stored.clientTempId = mess.clientTempId;
    stored.receiverId = manager.getUserId(receiverUserName);
    if (stored.receiverId < 0)
        stored.receiverId = 0;
    ApplicationResponse response;
    if(userRegistry.isOnline(receiverUserName)) {
    response.type = ResponseType::MESSAGE_RESPONSE;
    response.target = FanOutToUser{receiverUserName};
    response.payload = SendMessageSuccessPayload{
    media_access::messageForViewer(stored, stored.receiverId)};
    }

   else {
     response.type = ResponseType::USER_OFFLINE;
     response.target = UnicastToSession{issuerSessionId};
     response.payload = SendMessageFailurePayload{ "User is offline"};
  

    }
    responses.push_back(response);

    responses.push_back(senderSessionSyncMessage(stored, senderUserName));

    ApplicationResponse otherResponse{};
    otherResponse.type = ResponseType::MESSAGE_ACK_RESPONSE;
    otherResponse.target = UnicastToSession{issuerSessionId};
    otherResponse.payload = SendMessageAckPayload{
    media_access::messageForViewer(stored, stored.senderID)};
    responses.push_back(otherResponse);

    return responses;
}


   std::vector<ApplicationResponse> MessageService::sendFirstMessage(Message message, int issuerSessionId) {
   std::vector<ApplicationResponse> responses;
   std::string senderUserName = message.senderUserName;
   std::string receiverUserName = message.receiverUserName;

    if (manager.getUserId(receiverUserName) == -1) {
        ApplicationResponse nf;
        nf.type = ResponseType::PEER_USER_NOT_FOUND_RESPONSE;
        nf.target = UnicastToSession{issuerSessionId};
        nf.payload = PeerUserNotFoundPayload{
            "No user found with that username.",
            receiverUserName,
            0,
        };
        responses.push_back(nf);
        return responses;
    }

    Message stored = manager.createFirstMessage(message);
    stored.senderUserName = senderUserName;
    stored.receiverUserName = receiverUserName;
    stored.clientTempId = message.clientTempId;

    ApplicationResponse response;
    response.type = ResponseType::MESSAGE_RESPONSE;
    response.target = FanOutToUser{receiverUserName};
    response.payload = SendMessageSuccessPayload{
        media_access::messageForViewer(stored, stored.receiverId)};

    ApplicationResponse chatIdResponse;
    chatIdResponse.type = ResponseType::CHATROOM_ID_RESPONSE;
    chatIdResponse.target = FanOutToUser{senderUserName};
    chatIdResponse.payload = ChatRoomIdSuccessPayload{stored.chatRoomID, stored.receiverId,stored.receiverUserName};
    responses.push_back(response);
    responses.push_back(chatIdResponse);
    responses.push_back(senderSessionSyncMessage(stored, senderUserName));

    ApplicationResponse otherResponse{};
    otherResponse.type = ResponseType::MESSAGE_ACK_RESPONSE;
    otherResponse.target = UnicastToSession{issuerSessionId};
    otherResponse.payload = SendMessageAckPayload{
        media_access::messageForViewer(stored, stored.senderID)};
    responses.push_back(otherResponse);

    return responses;
}


   ApplicationResponse MessageService::fetchMessages( std::string& senderUserName, std::string& receiverUserName,int limit,std::optional<int> beforeMessageId, std::optional<int> afterMessageId, int identifier, int issuerSessionId, std::optional<int> anchorMessageId) {
    if (manager.getUserId(receiverUserName) == -1) {
        ApplicationResponse response;
        response.type = ResponseType::PEER_USER_NOT_FOUND_RESPONSE;
        response.target = UnicastToSession{issuerSessionId};
        response.payload = PeerUserNotFoundPayload{
            "No user found with that username.",
            receiverUserName,
            identifier,
        };
        return response;
    }

    std::vector<std::vector<Message>> messages =  manager.retreiveMessages(senderUserName, receiverUserName,limit,beforeMessageId,afterMessageId,anchorMessageId);

    const int viewerUserId = manager.getUserId(senderUserName);
    media_access::enrichAllMessagesForViewer(messages, viewerUserId);

    std::optional<int> lastSeenId;
    std::optional<std::string> seenAt;
    std::optional<int> lastSeenIdByOther;
    std::optional<std::string> seenAtByOther;

    if (!messages.empty()) {
        if(!messages[0].empty()) {
        const int chatRoomId = messages[0][0].chatRoomID;
        if (auto selfSeen = manager.getSeenState(chatRoomId, senderUserName)) {
            lastSeenId = selfSeen->lastSeenMessageId;
            seenAt = selfSeen->seenAt;
        }
        if (auto otherSeen = manager.getSeenState(chatRoomId, receiverUserName)) {
            lastSeenIdByOther = otherSeen->lastSeenMessageId;
            seenAtByOther = otherSeen->seenAt;
        }
        }
    }
    //if the other user exists firstly 
    int otherUserId = manager.getUserId(receiverUserName);

    std::string peerProfileUrl;
    if (viewerUserId > 0 && otherUserId > 0)
        peerProfileUrl = media_access::profileReadPath(viewerUserId, otherUserId);
        
    ApplicationResponse response;
    response.type = ResponseType::FETCH_MESSAGES_RESPONSE;
    response.target = UnicastToSession{issuerSessionId};

    response.payload = FetchMessagesSuccessPayload{
        identifier, messages, lastSeenId, seenAt, lastSeenIdByOther, seenAtByOther, otherUserId, peerProfileUrl};

    return response;
}


ApplicationResponse MessageService::searchMessages(std::string& senderUserName, std::string& receiverUserName, const std::string& text, int searchQueryId, int issuerSessionId) {
    ApplicationResponse response{};
    response.type = ResponseType::MESSAGE_SEARCH_RESPONSE;
    response.target = UnicastToSession{issuerSessionId};

    if (text.empty() || manager.getUserId(receiverUserName) == -1) {
        response.payload = MessageSearchSuccessPayload{searchQueryId, {}};
        return response;
    }

    std::vector<Message> hits = manager.searchMessages(senderUserName, receiverUserName, text, 50);
    response.payload = MessageSearchSuccessPayload{searchQueryId, std::move(hits)};
    return response;
}

ApplicationResponse MessageService::getRecentChats(int userId, int sessionId) {
    std::vector<ChatPreview> rooms = manager.getChatRooms(userId);
    for(auto &room : rooms) {
        if(userRegistry.isOnline(room.otherUserName)) {
            room.online = true;
        } else {
            room.online = false;
        }
        room.profileUrl = media_access::profileReadPath(userId, room.otherUserId);
        presenceService.subscribe(userId,room.otherUserId);
    }

    ApplicationResponse response;
    response.type = ResponseType::RECENT_CHATROOM_RESPONSE;
    response.target = UnicastToSession{sessionId};
    RecentChatRoomsSuccessPayload payload;
    payload.rooms = std::move(rooms);
    payload.selfProfileUrl = media_access::profileReadPath(userId, userId);
    response.payload = payload;

    return response;
}

ApplicationResponse MessageService::makeSeen( int chatId, int userId, int lastSeenMessageId) {
    ApplicationResponse response{};
    response.type = ResponseType::SEEN_RESPONSE;
    SeenDTO seen;
    auto seenRow = manager.CreateSeen(chatId, userId, lastSeenMessageId);
     
    if (!seenRow.has_value()) {
        response.intent = ResponseIntent::NOOP;
        return response;
    }

    int otherUserId =
        manager.getOtherUserId(chatId, userId);

    std::optional<UserContext> userContext =
        userRegistry.findByUserId(otherUserId);

    if (!userContext.has_value()) {
        response.intent = ResponseIntent::NOOP;
        return response;
    }

    seen.lastSeenMessageId = seenRow->lastSeenMessageId;
    seen.chatId = chatId;
    seen.seenAt = seenRow->seenAt;

    response.intent = ResponseIntent::SEND;
    response.type   = ResponseType::SEEN_RESPONSE;
    response.target = FanOutToUser{userContext->userName};
    response.payload = SeenSuccessPayload{ seen };

    return response;
}
 
ApplicationResponse MessageService::fetchImagesId (int chatId, int userId, int chatIdentifier, int issuerSessionId) {
    ApplicationResponse response{};
    response.type = ResponseType::FETCH_IMAGES_FOR_CHAT_RESPONSE;

    if (!userRegistry.findBySession(issuerSessionId).has_value()) {
        response.intent = ResponseIntent::NOOP;
        return response;
    }
    std::vector<int> imagesId = manager.getImagesId(chatId,userId);
    std::vector<std::string> imageMediaUrls;
    imageMediaUrls.reserve(imagesId.size());
    for (int id : imagesId)
        imageMediaUrls.push_back(media_access::messageReadPath(userId, id));

    response.type = ResponseType::FETCH_IMAGES_FOR_CHAT_RESPONSE;
    response.target = UnicastToSession{issuerSessionId};
    response.payload = FetchImagesIdPayload{imagesId, imageMediaUrls, chatIdentifier};
    
    return response;
}

std::vector<ApplicationResponse> MessageService::setReaction(
    int messageId,
    int chatRoomId,
    int userId,
    const std::string& userName,
    const std::string& reaction,
    int issuerSessionId)
{
    std::vector<ApplicationResponse> responses;

    auto makeFailure = [&](const std::string& reason) {
        ApplicationResponse fail{};
        fail.type = ResponseType::REACTION_RESPONSE;
        fail.target = UnicastToSession{issuerSessionId};
        fail.payload = ReactionFailurePayload{reason};
        responses.push_back(fail);
        return responses;
    };

    if (messageId <= 0 || chatRoomId <= 0 || userId <= 0 || userName.empty())
        return makeFailure("Invalid request");

    if (!userRegistry.findByUserId(userId))
        return makeFailure("Unauthorized");

    if (!manager.setMessageReaction(messageId, chatRoomId, userId, reaction))
        return makeFailure("Could not set reaction");

    ReactionSuccessPayload event{messageId, chatRoomId, userId, reaction};

    ApplicationResponse toActor{};
    toActor.type = ResponseType::REACTION_RESPONSE;
    toActor.target = FanOutToUser{userName};
    toActor.payload = event;
    responses.push_back(toActor);

    const int otherUserId = manager.getOtherUserId(chatRoomId, userId);
    if (otherUserId > 0) {
        if (auto other = userRegistry.findByUserId(otherUserId)) {
            ApplicationResponse toPeer{};
            toPeer.type = ResponseType::REACTION_RESPONSE;
            toPeer.target = FanOutToUser{other->userName};
            toPeer.payload = event;
            responses.push_back(toPeer);
        }
    }

    return responses;
}
