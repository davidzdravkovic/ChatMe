#include "../include/repositories/MessagesDB.h"
#include "../include/repositories/MessageReactionsDB.h"
#include <unordered_map>
#include <vector>
#include <string>
#include <iostream>

namespace {

void mapRowToMessage(const pqxx::row& row, Message& m) {
    m.chatRoomID = row["chatroom_id"].as<int>();
    m.time       = row["created_at"].as<std::string>();
    m.senderID   = row["sender_id"].as<int>();
    m.messageID  = row["message_id"].as<int>();
    m.content    = row["content"].as<std::string>();
    if (row["media_id"].is_null())
        m.mediaID = 0;
    else
        m.mediaID = row["media_id"].as<int>();
    if (row["reply_to_message_id"].is_null())
        m.replyToMessageId = 0;
    else
        m.replyToMessageId = row["reply_to_message_id"].as<int>();
}

} // namespace

    std::vector<Message> MessagesDB :: getOldMessages (const int &chatID,int limit,std::optional<int> beforeMessageId, pqxx::work &tx) {

      std::vector<Message> messages;

    try {
    
    limit = std::min(limit, 100);

     pqxx::result r;

     if (beforeMessageId.has_value()) {
        r = tx.exec_params(
            "SELECT * FROM messages "
            "WHERE chatroom_id = $1 "
            "AND message_id < $2 "
              "AND status = 'READY'"
            "ORDER BY message_id DESC "
            "LIMIT $3",
            chatID,
            beforeMessageId.value(),
            limit
        );
    } else {
        r = tx.exec_params(
            "SELECT * FROM messages "
            "WHERE chatroom_id = $1 "
            "AND status = 'READY'"
            "ORDER BY message_id DESC "
            "LIMIT $2",
            chatID,
            limit
        );
    }

    for (const auto& row : r) {
        Message m;
        mapRowToMessage(row, m);
        messages.push_back(m);
    }

    std::reverse(messages.begin(), messages.end());
    return messages;

    } catch (pqxx::sql_error &e) {
        std::cerr << "SQL error: " << e.what() << std::endl;
        std::cerr << "Querie error: " << e.query() << std::endl;
        return {};
    } catch (std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return {};
    }
}


std::vector<Message> MessagesDB::getLatest(int chatID, int limit, pqxx::work& tx) {
    limit = std::min(limit, 100);

    pqxx::result r = tx.exec_params(
        R"(
            SELECT *
            FROM messages
            WHERE chatroom_id = $1
                AND status = 'READY'
            ORDER BY message_id DESC
            LIMIT $2
        )",
        chatID,
        limit
    );

    return buildAscending(r,true);
}

std::vector<Message> MessagesDB::getBefore(int chatID, int beforeMessageId, int limit,pqxx::work& tx) {
    limit = std::min(limit, 100);

    pqxx::result r = tx.exec_params(
        R"(
            SELECT *
            FROM messages
            WHERE chatroom_id = $1
              AND message_id < $2
               AND status = 'READY'
            ORDER BY message_id DESC
            LIMIT $3
        )",
        chatID,
        beforeMessageId,
        limit
    );

    return buildAscending(r,true);
}
std::vector<Message> MessagesDB::getAfter( int chatID,int afterMessageId,int limit,pqxx::work& tx) {
    limit = std::min(limit, 100);

    pqxx::result r = tx.exec_params(
        R"(
            SELECT *
            FROM messages
            WHERE chatroom_id = $1
              AND message_id > $2
             AND status = 'READY'
            ORDER BY message_id ASC
            LIMIT $3
        )",
        chatID,
        afterMessageId,
        limit
    );

    return buildAscending(r,false);
}


Message MessagesDB::createMessage(const Message &message, pqxx::work &tx) {

    try {

        pqxx::result r;
        if (message.replyToMessageId > 0) {
            r = tx.exec_params(
                "INSERT into messages (chatroom_id, sender_id, content, reply_to_message_id) "
                "VALUES ($1, $2, $3, $4) RETURNING *;",
                message.chatRoomID,
                message.senderID,
                message.content,
                message.replyToMessageId);
        } else {
            r = tx.exec_params(
                "INSERT into messages (chatroom_id, sender_id, content) VALUES ($1, $2, $3) RETURNING *;",
                message.chatRoomID,
                message.senderID,
                message.content);
        }

        if (!r.empty()) {
            Message theMessage;
            mapRowToMessage(r[0], theMessage);
            return theMessage;
        }
    } catch (pqxx::sql_error &e) {
        std::cerr << "SQL error: " << e.what() << std::endl;
        std::cerr << "Querie error: " << e.query() << std::endl;

    } catch (std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    return {};
}

Message MessagesDB :: getMessage(const int &chatID, const int &userID, pqxx::work &tx) {

  
  try {
       
        pqxx::result r = tx.exec_params( "SELECT * FROM messages WHERE chatroom_id=$1 AND sender_id=$2 AND status = 'READY'",chatID,userID);

        if (!r.empty()) {
            Message message;
            message.chatRoomID = r[0]["chatroom_id"].as<int>();
            message.senderID = r[0]["sender_id"].as<int>();
            message.content = r[0]["content"].as<std::string>();
            return message;
        }
        return {};
    } catch (pqxx::sql_error &e) {
        std::cerr << "SQL error: " << e.what() << std::endl;
        std::cerr << "Querie error: " << e.query() << std::endl;
        return {};
    } catch (std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return {};
    }
}

std::vector<ChatPreview> MessagesDB::getLastMessagePerChat( const std::vector<int>& chatIDs,int myUserID, pqxx::work& tx) {
    
std::vector<ChatPreview> chats;

    try {
   pqxx::result r = tx.exec_params(
    "WITH last_messages AS ("
    "   SELECT DISTINCT ON (m.chatroom_id) "
    "       m.chatroom_id, "
    "       m.sender_id, "
    "       m.content, "
    "       m.created_at, "
    "       u.user_id  AS other_user_id, "
    "       u.username AS other_username, "
    "       u.last_active_at AS other_last_active_at "
    "   FROM messages m "
    "   JOIN chat_participants cp ON cp.chatroom_id = m.chatroom_id "
    "   JOIN users u ON u.user_id = cp.user_id "
    "   WHERE m.chatroom_id = ANY($2) "
    "     AND cp.user_id <> $1 "
    "     AND status = 'READY'"
    "   ORDER BY m.chatroom_id, m.created_at DESC "
    ") "
    "SELECT * "
    "FROM last_messages "
    "ORDER BY created_at DESC",
    pqxx::params{myUserID, chatIDs}
);

        for (int i = 0; i < r.size(); ++i) {
            ChatPreview chat;
            
            chat.otherUserId    = r[i]["other_user_id"].as<int>();
            chat.chatroomID     = r[i]["chatroom_id"].as<int>();
            chat.senderID       = r[i]["sender_id"].as<int>();
            chat.content        = r[i]["content"].as<std::string>();
            chat.time           = r[i]["created_at"].as<std::string>();
            chat.otherUserName  = r[i]["other_username"].as<std::string>();

            if (!r[i]["other_last_active_at"].is_null())
                chat.lastActiveAt = r[i]["other_last_active_at"].as<std::string>();

            chats.push_back(chat);
        }
    }
    catch (const pqxx::sql_error& e) {
        std::cerr << "SQL error: " << e.what()
                  << "\nQuery: " << e.query() << std::endl;
    }

    return chats;
}
 int MessagesDB::getSenderIdForMessage(const int& chatID, const int& messageID, pqxx::work& tx)
{
    try {
        pqxx::result r = tx.exec_params(
            R"(
                SELECT sender_id
                FROM messages
                WHERE chatroom_id = $1
                  AND message_id  = $2
                  AND status      = 'READY'
            )",
            chatID,
            messageID
        );

        if (!r.empty())
            return r[0]["sender_id"].as<int>();
    }
    catch (pqxx::sql_error& e) {
        std::cerr << "SQL error: " << e.what()
                  << "\nQuery error: " << e.query() << std::endl;
    }
    catch (std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return -1;
}

std::vector<Message> MessagesDB :: buildAscending(const pqxx::result& r, bool reverse) {
    std::vector<Message> messages;
    messages.reserve(r.size());

    for (const auto& row : r) {
        Message m;
        mapRowToMessage(row, m);
        messages.push_back(m);
    }
     if(reverse) 
      std::reverse(messages.begin(), messages.end());
    return messages;
}
std::optional<Message> MessagesDB::getById(int chatID, int messageId, pqxx::work& tx)
{
    pqxx::result r = tx.exec_params(
        R"(
            SELECT *
            FROM messages
            WHERE chatroom_id = $1
              AND message_id  = $2
              AND status      = 'READY'
            LIMIT 1
        )",
        chatID,
        messageId
    );

    if (r.empty())
        return std::nullopt;

    auto vec = buildAscending(r, false);
    return vec.front();
}

std::vector<Message> MessagesDB::searchInChat(int chatID, const std::string& text, int limit, pqxx::work& tx) {
    if (text.empty())
        return {};

    limit = std::min(limit, 50);

    try {
        pqxx::result r = tx.exec_params(
            R"(
                SELECT *
                FROM messages
                WHERE chatroom_id = $1
                  AND status = 'READY'
                  AND content ILIKE '%' || $2 || '%'
                ORDER BY message_id DESC
                LIMIT $3
            )",
            chatID,
            text,
            limit
        );
        // Keep newest-first ordering for the results list (no reverse).
        return buildAscending(r, false);
    }
    catch (pqxx::sql_error& e) {
        std::cerr << "SQL error: " << e.what()
                  << "\nQuery: " << e.query() << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    return {};
}

std::optional<int> MessagesDB::getLastSentMessageId(int chatID, int userID, pqxx::work& tx)
{
    try {
        pqxx::result r = tx.exec_params(
            R"(
                SELECT message_id
                FROM messages
                WHERE chatroom_id = $1
                  AND sender_id   = $2
                  AND status      = 'READY'
                ORDER BY message_id DESC
                LIMIT 1
            )",
            chatID,
            userID
        );

        if (r.empty())
            return std::nullopt;

        return r[0]["message_id"].as<int>();
    }
    catch (pqxx::sql_error& e) {
        std::cerr << "SQL error: " << e.what()
                  << "\nQuery: " << e.query() << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return std::nullopt;
}
Message MessagesDB::insertingMessageImagePending(int chatroomId,int senderId,std::optional<int> mediaId, pqxx::work& tx)
{
    pqxx::result r = tx.exec_params(
        R"(
            INSERT INTO messages (
                chatroom_id,
                sender_id,
                content,
                media_id,
                status
            )
            VALUES ($1, $2, '', $3, 'PENDING_MEDIA')
            RETURNING *
        )",
        chatroomId,
        senderId,
        mediaId
    );

    if (r.empty()) {
        throw std::runtime_error("insertImageMessage failed: no row returned");
    }

    Message m;
    mapRowToMessage(r[0], m);
    return m;
}

std::optional<Message>MessagesDB::updatingMessageImageReady(int mediaId, pqxx::work& tx)
{
    pqxx::result r = tx.exec_params(
        R"(
            UPDATE messages
            SET status = 'READY'
            WHERE media_id = $1
              AND status = 'PENDING_MEDIA'
            RETURNING *
        )",
        mediaId
    );

    if (r.empty())
        return std::nullopt;

    Message m;
    mapRowToMessage(r[0], m);
    return m;
}

void MessagesDB::bindReplyPreview(Message& message, pqxx::work& tx) {
    if (message.replyToMessageId <= 0)
        return;

    std::optional<Message> parent =
        getById(message.chatRoomID, message.replyToMessageId, tx);
    if (!parent)
        return;

    if (!parent->content.empty()) {
        message.replyPreviewContent = parent->content;
    } else if (parent->mediaID > 0) {
        message.replyPreviewContent = "Photo";
    } else {
        message.replyPreviewContent = "Message";
    }
    message.replyPreviewSenderId = parent->senderID;
}

void MessagesDB::bindReplyPreviews(std::vector<Message>& messages, int chatID, pqxx::work& tx) {
    for (Message& m : messages) {
        if (m.chatRoomID <= 0)
            m.chatRoomID = chatID;
        bindReplyPreview(m, tx);
    }
}

void MessagesDB::bindReplyPreviewsAll(
    std::vector<std::vector<Message>>& batches,
    int chatID,
    pqxx::work& tx)
{
    for (auto& batch : batches)
        bindReplyPreviews(batch, chatID, tx);
}

void MessagesDB::bindReactions(std::vector<Message>& messages, pqxx::work& tx) {
    std::vector<int> ids;
    ids.reserve(messages.size());
    for (const Message& m : messages) {
        if (m.messageID > 0)
            ids.push_back(m.messageID);
    }
    if (ids.empty())
        return;

    const std::vector<MessageReaction> all = MessageReactionsDB::getForMessageIds(ids, tx);
    std::unordered_map<int, std::vector<MessageReaction>> byMessage;
    for (const MessageReaction& r : all)
        byMessage[r.messageId].push_back(r);

    for (Message& m : messages) {
        auto it = byMessage.find(m.messageID);
        if (it != byMessage.end())
            m.reactions = it->second;
    }
}

void MessagesDB::bindReactionsAll(
    std::vector<std::vector<Message>>& batches,
    pqxx::work& tx)
{
    for (auto& batch : batches)
        bindReactions(batch, tx);
}

std::vector<int> MessagesDB::imagesId(int chatId, pqxx::work& tx) {
    pqxx::result r = tx.exec_params(
        R"(
            SELECT media_id
            FROM messages
            WHERE chatroom_id = $1
              AND media_id IS NOT NULL
            ORDER BY message_id DESC
        )",
        chatId);

    std::vector<int> ids;
    ids.reserve(static_cast<size_t>(r.size()));
    for (const auto& row : r) {
        ids.push_back(row[0].as<int>());
    }
    return ids;
}