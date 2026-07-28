#include "../include/Handler/handler.h"
#include "../include/Auth/Auth.h"
#include "../include/NetworkAction/ApplicationResponse.h"
#include "../include/Deserialization/DTO/SearchQuery.h"
#include <iostream>
#include <type_traits>
#include <variant>

namespace {

/** JSON `status` field: 1 → "SUCCESS", 0 → empty (clients treat non-SUCCESS as failure). */
static int wireStatusFromPayload(const ResponsePayload& p) {
    return std::visit([](const auto& payload) -> int {
        using T = std::decay_t<decltype(payload)>;
        if constexpr (std::is_same_v<T, LoginFailurePayload> ||
                      std::is_same_v<T, CreateFailurePayload> ||
                      std::is_same_v<T, AuthFailurePayload> ||
                      std::is_same_v<T, FetchMessagesFailurePayload> ||
                      std::is_same_v<T, SendMessageFailurePayload> ||
                      std::is_same_v<T, FirstMessageFailurePayload> ||
                      std::is_same_v<T, ChatRoomIdFailurePayload> ||
                      std::is_same_v<T, PeerUserNotFoundPayload> ||
                      std::is_same_v<T, RecentChatRoomsFailurePayload> ||
                      std::is_same_v<T, ActiveStatusUpdateFailurePayload> ||
                      std::is_same_v<T, UploadProfilePictureFailurePayload> ||
                      std::is_same_v<T, TypingFailurePayload> ||
                      std::is_same_v<T, ReactionFailurePayload>) {
            return 0;
        }
        if constexpr (std::is_same_v<T, UploadImageMessagePayload>) {
            return payload.uploadImageMessageResponse.approved ? 1 : 0;
        }
        return 1;
    }, p);
}

/** JWT is source of truth for "who is acting"; `data` still uses names like senderId / userID on the wire. */
void bindTrustedUser(const RequestStruct& req, int& actorUserId) {
    actorUserId = req.authenticatedUserId;
}

void bindTrustedUser(const RequestStruct& req, int& actorUserId, std::string& actorUserName) {
    actorUserId = req.authenticatedUserId;
    if (!req.authenticatedUserName.empty())
        actorUserName = req.authenticatedUserName;
}

/**
 * Issues JWT only for successful login/create application responses.
 * Does not run for other entries in the same batch (e.g. presence / ACTIVE_STATUS).
 */
void issueJwtOnLoginOrCreateSuccess(std::vector<ApplicationResponse>& responses) {
    for (auto& r : responses) {
        if (!r.payload.has_value())
            continue;

        if (r.type == ResponseType::LOGIN_RESPONSE) {
            if (auto* p = std::get_if<LoginSuccessPayload>(&*r.payload))
                p->accessToken = Auth::issueJWT(p->user.userID, p->user.userName);
            continue;
        }
        if (r.type == ResponseType::CREATE_RESPONSE) {
            if (auto* p = std::get_if<CreateSuccessPayload>(&*r.payload))
                p->accessToken = Auth::issueJWT(p->user.userID, p->user.userName);
        }
    }
}

void issueJwtOnAuthSuccess(std::vector<ApplicationResponse>& responses) {
    for (auto& r : responses) {
        if (!r.payload.has_value())
            continue;
        if (r.type != ResponseType::AUTH_RESPONSE)
            continue;
        if (auto* p = std::get_if<AuthSuccessPayload>(&*r.payload))
            p->accessToken = Auth::issueJWT(p->userId, p->userName);
    }
}

} // namespace

void Handler::appendEmittedActions(std::vector<NetworkAction>& actions, const ApplicationResponse& response) {

    for (NetworkAction action : emitResponses(response)) {
        actions.push_back(std::move(action));
    }
    std::cout<<"Append function"<<std::endl;

}

 std::vector<NetworkAction> Handler :: routeToManager (const RequestStruct &req) {
    if(req.rType==RequestType::CREATE_REQUEST) {
        return handleCreate(req); 
    }
      if(req.rType==RequestType::LOGIN_REQUEST){
      return handleLogIn(req); 
    }
      if(req.rType==RequestType::AUTH_REQUEST){
      return handleAuth(req);
    }
       if(req.rType==RequestType::FETCH_MESSAGES_REQUEST) {
        return handleGetChat(req); 
    } 
    if(req.rType == RequestType::MESSAGE_REQUEST) {
          return sendMessage(req);
    }
        if(req.rType == RequestType::FIRST_MESSAGE_REQUEST) {
        return sendFirstMessage(req);
    }
    if(req.rType == RequestType::RECENT_CHATROOM_REQUEST) {
        return getChatRooms(req);
    }
    //Internal app request
    if(req.rType == RequestType::DISCONNECT_REQUEST) {
        return removeUser(req);
         
    }
    if(req.rType == RequestType::UPLOAD_PROFILE_PICTURE_REQUEST) {
        return uploadProfilePicture(req);
    } 
    if(req.rType == RequestType::TYPING_REQUEST) {
        return typingForward(req);
    }
    if(req.rType==RequestType::SEEN_REQUEST) {
        return handleSeen(req);
    }
    if(req.rType==RequestType::UPLOAD_IMAGE_MESSAGE_REQUEST) {
        return uploadImageMessage(req);
    }
    if(req.rType==RequestType::FETCH_IMAGES_FOR_CHAT_REQUEST) {
        return fetchImages(req);
    }
    if(req.rType==RequestType::LOGOUT_REQUEST) {
        return logOut(req);
    }
    if (req.rType == RequestType::SEARCH_QUERY_REQUEST) {
        return searchQuery(req);
    }
    if (req.rType == RequestType::REACTION_REQUEST) {
        return handleReaction(req);
    }
    if (req.rType == RequestType::MESSAGE_SEARCH_REQUEST) {
        return searchMessages(req);
    }
     return {};
}


std::vector<NetworkAction> Handler::handleCreate(const RequestStruct &req) {
  
    std::vector<NetworkAction> actions;
    CreateStruct dto = deserialize<CreateStruct>(req);
    User u = Mapper::mapCreateStructToUser(dto);

  //This function returns 2 responses one for presence and one for the user creation itself used for issuing the token.
  std::vector<ApplicationResponse> response = userService.create(u, dto.sessionId);
    issueJwtOnLoginOrCreateSuccess(response);

   for(auto &resp : response) {
    appendEmittedActions(actions, resp);
        } 
    return actions;

}

std::vector<NetworkAction> Handler::handleLogIn(const RequestStruct &req) {

      std::vector<NetworkAction> actions;
  
    LogStruct dto = deserialize<LogStruct>(req);
    User u = Mapper::mapLogStructToUser(dto);
    LoginResult result = userService.login(u.userName, u.password, dto.sessionId);

    issueJwtOnLoginOrCreateSuccess(result.responses);

  for(auto &resp : result.responses) {
    appendEmittedActions(actions, resp);
        } 
    return actions;
    }

std::vector<NetworkAction> Handler::handleAuth(const RequestStruct &req) {
    std::vector<NetworkAction> actions;

   if (!req.isAuthenticated || req.authenticatedUserId <= 0 || req.authenticatedUserName.empty()) {
        ApplicationResponse fail{};
        fail.type = ResponseType::AUTH_RESPONSE;
        fail.target = UnicastToSession{req.sessionID};
        fail.intent = ResponseIntent::SEND;
        fail.payload = AuthFailurePayload{"Unauthorized"};
        appendEmittedActions(actions, fail);
        return actions;
    }

    LoginResult result = userService.resumeSession(req.authenticatedUserId,req.authenticatedUserName,req.sessionID);
    issueJwtOnAuthSuccess(result.responses);

    for (auto& resp : result.responses) {
        appendEmittedActions(actions, resp);
    }
    return actions;
}
    
std::vector<NetworkAction> Handler::handleGetChat(const RequestStruct &req) {

    std::vector<NetworkAction> actions;
    ChatRetieve cRetrieve = deserialize<ChatRetieve>(req);
    if (!req.authenticatedUserName.empty())
        cRetrieve.senderUserName = req.authenticatedUserName;
    ApplicationResponse response = messService.fetchMessages(cRetrieve.senderUserName, cRetrieve.receiverUserName,cRetrieve.limit,cRetrieve.beforeMessageId, cRetrieve.afterMessageId,cRetrieve.identifier, req.sessionID, cRetrieve.anchorMessageId);
  for(auto &resp : {response}) {
    appendEmittedActions(actions, resp);
        } 
    return actions;
  
}
std::vector<NetworkAction> Handler::sendMessage(const RequestStruct &req) {

    std::vector<NetworkAction> actions;
    SendMessageStruct message =  deserialize<SendMessageStruct>(req);
    bindTrustedUser(req, message.senderId, message.senderUserName);
    Message mess = Mapper::createMessage(message);
    std::vector<ApplicationResponse> response = messService.sendMessage(mess, req.sessionID);

      for(auto &resp : response) 
      {
        appendEmittedActions(actions, resp); 
      } 

    return actions;


}
std::vector<NetworkAction> Handler::sendFirstMessage(const RequestStruct &req) {
    std::vector<NetworkAction> actions;
    SendFirstMessageStruct message = deserialize<SendFirstMessageStruct>(req);
    bindTrustedUser(req, message.senderId, message.senderUserName);
    Message mess = Mapper::createFirstMessageStruct(message);
    std::vector<ApplicationResponse> responses = messService.sendFirstMessage(mess, req.sessionID);

    for(auto &response : responses) {
        appendEmittedActions(actions, response);
    }
    
    return actions;
}

std::vector<NetworkAction> Handler::getChatRooms(const RequestStruct &req) {
    std::vector<NetworkAction> actions;
    ChatRoomDTO chatRoom;
    chatRoom = deserialize<ChatRoomDTO>(req);
    bindTrustedUser(req, chatRoom.userId);
   ApplicationResponse response  = messService.getRecentChats(chatRoom.userId,chatRoom.sessionId);
   
  for(auto &resp : {response}) {
    appendEmittedActions(actions, resp);
        } 
    return actions;
}


std::vector<NetworkAction> Handler::removeUser(const RequestStruct &req) {

    std::vector<ApplicationResponse> responses = userService.logout(req.sessionID);
    std::vector<NetworkAction> actions;

  for(auto &resp : responses) {
    appendEmittedActions(actions, resp);
        } 
    return actions;
}

//This function is not sending the network response on the wire so bypasses the emitResponses 
std::vector<NetworkAction> Handler::logOut(const RequestStruct &req) {
    // Uses the same endpoint remove user (client is removed also from network stack)
     std::vector<NetworkAction> actions;
            NetworkAction force;
            force.type = NetworkAction::ActionType::FORCE_DISCONNECT;
            force.sessionId = req.sessionID;
            actions.push_back(force);
            return actions;
}

std::vector<NetworkAction> Handler::typingForward(const RequestStruct &req) {

    std::vector<NetworkAction> actions;
    TypingRequest typing = deserialize<TypingRequest>(req);
    bindTrustedUser(req, typing.senderId, typing.senderUserName);

auto receiverSessions = registry.findSessionIdsByUsername(typing.receiverName);
if (receiverSessions.empty()) {
    return {};
}

if (!registry.findByUserId(typing.senderId))
    return {};

  ApplicationResponse response;
  response.type = ResponseType::TYPING_RESPONSE;
  response.target = FanOutToUser{typing.receiverName};
  response.payload = TypingPayload{typing};
  appendEmittedActions(actions, response);
    return actions;

}

std::vector<NetworkAction> Handler::uploadProfilePicture(const RequestStruct &req) {
    std::vector<NetworkAction> actions;


        UploadProfilePictureRequest request = dUploadPicture.deserialize(req);
    request.userId = req.authenticatedUserId;
   //The init first round of request just checks for user existence 
    std::vector<ApplicationResponse> responses = userService.uploadPictureProfile(request);
    
    for (auto &resp : responses) {
        appendEmittedActions(actions, resp);
    }

    return actions;
}


std::vector<NetworkAction> Handler::uploadImageMessage(const RequestStruct &req) {
    std::vector<NetworkAction> actions;

        DeserializeMediaMessage deserialize;
        UploadImageMessageRequest request = deserialize.deserialize(req);
    request.userId = req.authenticatedUserId;
    if (!req.authenticatedUserName.empty()) {
        request.senderUserName = req.authenticatedUserName;
    }

    // As the profile picture upload the init first round checks the user existence 
    std::vector<ApplicationResponse> responses = userService.uploadImageMessage(request);
    
    for (auto &resp : responses) {
        appendEmittedActions(actions, resp);
    }

    return actions;
}

std::vector<NetworkAction> Handler::handleSeen(const RequestStruct &req) {
     std::vector<NetworkAction> actions;
     SeenDTO seenRequest = deserialize<SeenDTO>(req);
     bindTrustedUser(req, seenRequest.userId);
     //Uses the registry to fetch the other user
     ApplicationResponse response = messService.makeSeen(seenRequest.chatId, seenRequest.userId, seenRequest.lastSeenMessageId);
    

      for(auto &resp : {response}) {
    appendEmittedActions(actions, resp);
        } 
    return actions;
}

std::vector<NetworkAction> Handler::fetchImages(const RequestStruct &req) {
   std::vector<NetworkAction> actions;
   FetchDTO fetchImageRequest = deserialize<FetchDTO>(req);
   bindTrustedUser(req, fetchImageRequest.userId);
   //Uses the registry to to see the receiver of this response and that is the sender for existense
   ApplicationResponse response = messService.fetchImagesId(
       fetchImageRequest.chatId,
       fetchImageRequest.userId,
       fetchImageRequest.chatIdentifier,
       req.sessionID
   );

      for(auto &resp : {response}) {
    appendEmittedActions(actions, resp);
        } 
    return actions;
}

std::vector<NetworkAction> Handler::handleReaction(const RequestStruct& req) {
    std::vector<NetworkAction> actions;
    ReactionRequest dto = deserialize<ReactionRequest>(req);
    bindTrustedUser(req, dto.userId, dto.userName);

    std::vector<ApplicationResponse> responses = messService.setReaction(
        dto.messageId,
        dto.chatRoomId,
        dto.userId,
        dto.userName,
        dto.reaction,
        req.sessionID);

    for (auto& resp : responses)
        appendEmittedActions(actions, resp);

    return actions;
}

std::vector<NetworkAction> Handler::searchQuery(const RequestStruct& req) {
    std::vector<NetworkAction> actions;
    if (!req.isAuthenticated || req.authenticatedUserId <= 0) {
        return actions;
    }

    SearchQuery dto = deserialize<SearchQuery>(req);
    ApplicationResponse response = userService.searchQueryResponse(
        dto.searchedCharacters,
        dto.searcherQueryId,
        req.authenticatedUserId,
        req.sessionID);
    appendEmittedActions(actions, response);
    return actions;
}

std::vector<NetworkAction> Handler::searchMessages(const RequestStruct& req) {
    std::vector<NetworkAction> actions;
    if (!req.isAuthenticated || req.authenticatedUserId <= 0 || req.authenticatedUserName.empty()) {
        return actions;
    }

    MessageSearchQuery dto = deserialize<MessageSearchQuery>(req);
    std::string senderUserName = req.authenticatedUserName;

    ApplicationResponse response = messService.searchMessages(
        senderUserName,
        dto.receiverUserName,
        dto.searchedText,
        dto.searchQueryId,
        req.sessionID);

    appendEmittedActions(actions, response);
    return actions;
}

// Recieves one application response and returns network actions as many session's has the target client.
// Network action gets serialized payload, target session id and the type of the action.
std::vector<NetworkAction> Handler::emitResponses(const ApplicationResponse& response)

{
    std::cout<<"[emitResponse] type=" << static_cast<int>(response.type) << "\n";
    if (response.intent == ResponseIntent::NOOP) {
        std::cerr << "[emitResponse] DROP intent=NOOP type=" << static_cast<int>(response.type) << "\n";
        return {NetworkAction::noop()};
    }

    if (!response.payload.has_value()) {
        std::cerr << "[emitResponse] DROP missing payload type=" << static_cast<int>(response.type) << "\n";
        return {NetworkAction::noop()};
    }

    //To application response could be added a bool field but this is more concrete it can not drift from the type, 
    //success or failuare is derived from the type
    const int wireStatus = wireStatusFromPayload(*response.payload);

    std::vector<uint8_t> bytes;
    std::visit([&](auto&& payload) {
            bytes = serialize.serialize(stringify.transform(payload),
            wireStatus,
            response.type);}, response.payload.value());

    NetworkAction actionTemplate;

    if (response.type == ResponseType::LOGIN_RESPONSE || 
        response.type == ResponseType::CREATE_RESPONSE || 
        response.type == ResponseType::AUTH_RESPONSE) 
        {
    actionTemplate.type = NetworkAction::ActionType::SEND_AUTH_RESPONSE;
    } 
    else {
    actionTemplate.type = NetworkAction::ActionType::SEND_TO_SESSION;        
    }

    actionTemplate.payload = bytes;

    if (auto* fanOut = std::get_if<FanOutToUser>(&response.target)) {
        const std::vector<SessionId> sessionIds =
            registry.findSessionIdsByUsername(fanOut->username);
        if (sessionIds.empty()) {
            std::cerr << "[emitResponse] DROP no session for username='" << fanOut->username
                      << "' type=" << static_cast<int>(response.type) << "\n";
            return {NetworkAction::noop()};
        }

        std::vector<NetworkAction> actions;
        actions.reserve(sessionIds.size());
        for (SessionId sid : sessionIds) {
            NetworkAction action = actionTemplate;
            action.sessionId = sid;
            actions.push_back(std::move(action));
        }
        return actions;
    }

    if (auto* unicast = std::get_if<UnicastToSession>(&response.target)) {
        NetworkAction action = actionTemplate;
        action.sessionId = unicast->sessionId;
        return {action};
    }

    std::cerr << "[emitResponse] DROP unknown target variant type=" << static_cast<int>(response.type) << "\n";
    return {NetworkAction::noop()};
}