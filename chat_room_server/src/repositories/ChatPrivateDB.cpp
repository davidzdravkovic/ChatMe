#include "../include/repositories/ChatPrivateDB.h"
#include <algorithm>
#include <stdexcept>




std::pair<int, int> ChatPrivateDB::normalizePair(int userAId, int userBId) {
    return {
        std::min(userAId, userBId),
        std::max(userAId, userBId)
    };
}

int ChatPrivateDB::createPrivateChat(int userAId, int userBId,pqxx::work &tx) {
    auto [u1, u2] = normalizePair(userAId, userBId);


    pqxx::result r =tx.exec_params(
        R"(
            INSERT INTO private_chats (user1_id, user2_id)
            VALUES ($1, $2)
            RETURNING id
        )",
        u1, u2
    );

    int privateChatId = r[0][0].as<int>();


    return privateChatId;
}

std::optional<int> ChatPrivateDB::findPrivateChat(int userAId, int userBId,pqxx::work &tx) {
    auto [u1, u2] = normalizePair(userAId, userBId);

  auto r = tx.exec_params(
        R"(
            SELECT id
            FROM private_chats
            WHERE user1_id = $1 AND user2_id = $2
        )",
        u1, u2
    );

    if (r.empty())
        return std::nullopt;

    return r[0][0].as<int>();
}

void ChatPrivateDB::bindChatroom(int privateChatId, int chatroomId,pqxx::work &tx) {
    

    pqxx::result r = tx.exec_params(
        R"(
            UPDATE private_chats
            SET chatroom_id = $1
            WHERE id = $2
        )",
        chatroomId, privateChatId
    );

    if (r.affected_rows() != 1) {
        throw std::runtime_error("Failed to bind chatroom to private chat");
    }


}
int ChatPrivateDB::getChatroomId(int privateChatId, pqxx::work &tx)
{
    pqxx::result r = tx.exec_params(
        R"(
            SELECT chatroom_id
            FROM private_chats
            WHERE id = $1
        )",
        privateChatId
    );

    if (r.empty())
        throw std::runtime_error("Private chat not found");

    if (r[0][0].is_null())
        throw std::runtime_error("Chatroom not bound yet");

    return r[0][0].as<int>();
}
