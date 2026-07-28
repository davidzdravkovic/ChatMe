#ifndef CHATPARTICIPANTSDB_H
#define CHATPARTICIPANTSDB_H
#include "./models/ChatParticipants.h"
#include <pqxx/pqxx>
#include <vector>

namespace ChatParticipantsDB {

 
 
 std::optional<std::vector<int>> getUsersID( const int &userID,pqxx::work &tx);

 int getChatID (const int &userID, const int &searchUserID, pqxx::work &tx);

 void insertChatParticipants(const int & userID, const int &searchUserID,const int & chatID,pqxx::work &tx);

 std::vector<int> getChatRoomsID (const int &userID,pqxx::work &tx);
 int getOtherParticipant(  const int& chatID,const int& userID, pqxx::work& tx);
 


};

#endif