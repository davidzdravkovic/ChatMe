#include "../include/repositories/SeenDB.h"
#include <iostream>

namespace SeenDB {

namespace {

std::optional<std::string> readSeenAt(const pqxx::row& row) {
    if (row["seen_at"].is_null())
        return std::nullopt;
    return row["seen_at"].as<std::string>();
}

SeenRow rowToSeen(const pqxx::row& row) {
    SeenRow out;
    out.lastSeenMessageId = row["last_seen_message_id"].as<int>();
    out.seenAt = readSeenAt(row);
    return out;
}

} // namespace

std::optional<SeenRow> fetchSeen(int chatID, int userID, pqxx::work& tx) {
    try {
        pqxx::result r = tx.exec_params(
            "SELECT last_seen_message_id, seen_at "
            "FROM chat_seen "
            "WHERE chatroom_id = $1 "
            "  AND user_id = $2",
            pqxx::params{chatID, userID}
        );

        if (r.empty())
            return std::nullopt;

        return rowToSeen(r[0]);
    }
    catch (const pqxx::sql_error& e) {
        std::cerr << "SQL error (fetchSeen): " << e.what() << "\nQuery: " << e.query() << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Error (fetchSeen): " << e.what() << std::endl;
    }
    return std::nullopt;
}

std::optional<SeenRow> upsertLastSeenMessage(int chatID, int userID, int lastSeenMessageID, pqxx::work& tx) {
    try {
        pqxx::result r = tx.exec_params(
            "INSERT INTO chat_seen (chatroom_id, user_id, last_seen_message_id) "
            "VALUES ($1, $2, $3) "
            "ON CONFLICT (chatroom_id, user_id) "
            "DO UPDATE SET "
            "  last_seen_message_id = GREATEST(chat_seen.last_seen_message_id, EXCLUDED.last_seen_message_id), "
            "  seen_at = CASE "
            "    WHEN EXCLUDED.last_seen_message_id > chat_seen.last_seen_message_id THEN now() "
            "    WHEN chat_seen.seen_at IS NULL AND EXCLUDED.last_seen_message_id >= chat_seen.last_seen_message_id THEN now() "
            "    ELSE chat_seen.seen_at "
            "  END "
            "RETURNING last_seen_message_id, seen_at",
            pqxx::params{chatID, userID, lastSeenMessageID}
        );

        if (r.empty())
            return std::nullopt;

        return rowToSeen(r[0]);
    }
    catch (const pqxx::sql_error& e) {
        std::cerr << "SQL error (upsertLastSeenMessage): " << e.what() << "\nQuery: " << e.query() << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Error (upsertLastSeenMessage): " << e.what() << std::endl;
    }
    return std::nullopt;
}

}
