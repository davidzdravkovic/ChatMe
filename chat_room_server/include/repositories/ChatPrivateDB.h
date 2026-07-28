#ifndef CHATPRIVATEDB_H
#define CHATPRIVATEDB_H
#include <pqxx/pqxx>
#include <optional>
#include <utility>




namespace ChatPrivateDB {

    int createPrivateChat(int userAId, int userBId,pqxx::work &tx);
    std::optional<int> findPrivateChat(int userAId, int userBId,pqxx::work &tx);
    void bindChatroom(int privateChatId, int chatroomId,pqxx::work &tx);
    int getChatroomId(int privateChatId, pqxx::work &tx); 
    static std::pair<int, int> normalizePair(int userAId, int);

 
};

#endif 
