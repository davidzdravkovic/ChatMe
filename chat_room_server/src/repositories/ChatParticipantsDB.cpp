#include "../include/repositories/ChatParticipantsDB.h"
#include <iostream>
#include <vector>


std::optional<std::vector<int>> ChatParticipantsDB :: getUsersID (const int &userID, pqxx::work &tx) {
std::vector<int> usersID;
try {

 
     pqxx::result r=tx.exec_params(R"(
        SELECT DISTINCT cp2.user_id
        FROM chat_participants cp1 JOIN chat_participants cp2
        ON cp1.chatroom_id = cp2.chatroom_id WHERE cp1.user_id = $1 AND cp2.user_id <> $1)",userID);

        if(!r.empty()) {
     for(int i=0; i<r.size(); i++) {
        int userID;
        userID = r[i]["user_id"].as<int>();
        usersID.push_back(userID);
     }
        }
     return usersID;   

}
  catch(pqxx::sql_error &e) {
 std::cerr<<"SQL error: "<<e.what()<<std::endl;
 std::cerr<<"Querie error: "<<e.query()<<std::endl;
return std::nullopt;
    }
    catch(std::exception &e) {
  std::cerr<<"Error: "<<e.what()<<std::endl;
return std::nullopt;
    }
   

}


int ChatParticipantsDB :: getChatID (const int &userID, const int &searchUserID, pqxx::work &tx) {
    int chatID = -1;
   try {

      pqxx::result r = tx.exec(
            "SELECT chatroom_id "
            "FROM chat_participants "
            "WHERE user_id = " + tx.quote(userID) + " OR user_id = " + tx.quote(searchUserID) + " "
            "GROUP BY chatroom_id "
            "HAVING COUNT(DISTINCT user_id) = 2;"
        );
        if(!r.empty()) {
          chatID = r[0]["chatroom_id"].as<int>();
          return chatID;
        }
   }
    catch(pqxx::sql_error &e) {
 std::cerr<<"SQL error: "<<e.what()<<std::endl;
 std::cerr<<"Querie error: "<<e.query()<<std::endl;
   return chatID;
    }
    catch(std::exception &e) {
  std::cerr<<"Error: "<<e.what()<<std::endl;
   return chatID;
    }
    return chatID;
   
}

void ChatParticipantsDB :: insertChatParticipants (const int & userID, const int & searchUserID, const int & chatID, pqxx::work &tx) {


     try {
   
      pqxx::result r = tx.exec_params("INSERT into chat_participants (chatroom_id, user_id) VALUES ($1,$2)",
      chatID, userID);
      pqxx::result r1 = tx.exec_params("INSERT into chat_participants (chatroom_id, user_id) VALUES ($1,$2)",
      chatID, searchUserID);  
   
        
   }
    catch(pqxx::sql_error &e) {
 std::cerr<<"SQL error: "<<e.what()<<std::endl;
 std::cerr<<"Querie error: "<<e.query()<<std::endl;
   
    }
    catch(std::exception &e) {
  std::cerr<<"Error: "<<e.what()<<std::endl;

    }


}
 

std::vector<int> ChatParticipantsDB :: getChatRoomsID (const int &userID, pqxx::work &tx) {
    std::vector<int> chatRoomsID;
   try {

      pqxx::result r = tx.exec_params(
            "SELECT DISTINCT chatroom_id "
            "FROM chat_participants "
            "WHERE user_id = $1",
            userID
        );
        for(int i=0; i<r.size(); i++) {
        int chatRoomID;
        chatRoomID = r[i]["chatroom_id"].as<int>();
        chatRoomsID.push_back(chatRoomID);
     }
     return chatRoomsID;
   }
    catch(pqxx::sql_error &e) {
 std::cerr<<"SQL error: "<<e.what()<<std::endl;
 std::cerr<<"Querie error: "<<e.query()<<std::endl;
   return chatRoomsID;
    }
    catch(std::exception &e) {
  std::cerr<<"Error: "<<e.what()<<std::endl;
   return chatRoomsID;
    }
    return chatRoomsID;
   
}

int ChatParticipantsDB::getOtherParticipant(  const int& chatID,const int& userID, pqxx::work& tx) {
    int otherUserId = -1;

    try {
        pqxx::result r = tx.exec_params(
            "SELECT user_id "
            "FROM chat_participants "
            "WHERE chatroom_id = $1 "
            "  AND user_id <> $2 "
            "LIMIT 1",
            chatID,
            userID
        );

        if (!r.empty()) {
            otherUserId = r[0]["user_id"].as<int>();
        }
    }
    catch (const pqxx::sql_error& e) {
        std::cerr << "SQL error (getOtherParticipant): "
                  << e.what() << "\nQuery: "
                  << e.query() << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Error (getOtherParticipant): "
                  << e.what() << std::endl;
    }

    return otherUserId;
}