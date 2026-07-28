#ifndef HANDLER_H
#define HANDLER_H
#include "../include/Serialization/Serialization.h"
#include "../include/Serialization/Stringify.h"
#include "../include/Deserialization/DeserializeUploadPicture.h"
#include "./Deserialization/DTO/DtoFieldsValues.h"
#include "./Deserialization/Deserialization.h"
#include "./SharedContext/OnlineUserRegistry.h"
#include "./Deserialization/DeserializeCreate.h"
#include "./Deserialization/DeserializeMediaMessage.h"
#include "./services/UserService.h"
#include "./services/MessageService.h"
#include "../include/NetworkAction/ApplicationResponse.h"
#include "./NetworkAction/NetworkAction.h"
#include "./services/Manager.h"
#include <boost/asio.hpp>
#include <cstdint>
#include "./Mapper/Mapper.h"

class Handler {
 Serialization serialize;
 Stringify stringify;
 DeserializeUploadProfilePicture dUploadPicture;
 OnlineUserRegistry &registry;
 UserService &userService;
 Manager &manager;
 MessageService &messService;


 std::vector<NetworkAction> handleLogIn (const RequestStruct &req);
 std::vector<NetworkAction> handleAuth(const RequestStruct &req);
 std::vector<NetworkAction> handleCreate (const RequestStruct &req);
 std::vector<NetworkAction> handleGetChat(const RequestStruct &req);
 std::vector<NetworkAction> sendMessage (const RequestStruct &req);
 std::vector<NetworkAction> sendFirstMessage (const RequestStruct &req);
 std::vector<NetworkAction> getChatRooms (const RequestStruct &req);
 std::vector<NetworkAction> removeUser (const RequestStruct &req);
 std::vector<NetworkAction> uploadProfilePicture(const RequestStruct &req);
 std::vector<NetworkAction> typingForward(const RequestStruct &req);
 std::vector<NetworkAction> handleSeen(const RequestStruct &req);
 std::vector<NetworkAction> updateSeen(const RequestStruct &req);
 std::vector<NetworkAction> uploadImageMessage(const RequestStruct &req);
 std::vector<NetworkAction> fetchImages(const RequestStruct &req);
 std::vector<NetworkAction> logOut(const RequestStruct &req);
 std::vector<NetworkAction> searchQuery(const RequestStruct &req);
 std::vector<NetworkAction> handleReaction(const RequestStruct &req);
 std::vector<NetworkAction> searchMessages(const RequestStruct &req);
 
 void appendEmittedActions(std::vector<NetworkAction>& actions, const ApplicationResponse& response);
 std::vector<NetworkAction> emitResponses(const ApplicationResponse& response);



public:
  Handler(Manager &man, UserService &userS, MessageService &mess,OnlineUserRegistry &reg): manager(man), userService(userS), messService(mess), registry(reg)  {};
 std::vector<NetworkAction>  routeToManager(const RequestStruct &request);
};
#endif
