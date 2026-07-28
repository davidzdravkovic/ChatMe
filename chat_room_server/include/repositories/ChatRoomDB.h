#ifndef CHATROOMDB_H
#define CHATROOMDB_H
#include "./models/ChatRoom.h"
#include <pqxx/pqxx>

namespace ChatRoomDB {

 

 int createChatRoom(const int &sizeRoom, pqxx::work &tx);
 


 
};

#endif