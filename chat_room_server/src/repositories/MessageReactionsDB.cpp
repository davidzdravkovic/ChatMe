#include "../include/repositories/MessageReactionsDB.h"
#include <iostream>

namespace {

MessageReaction mapRow(const pqxx::row& row) {
    MessageReaction r;
    r.messageId = row["message_id"].as<int>();
    r.userId = row["user_id"].as<int>();
    r.reaction = row["reaction"].as<std::string>();
    return r;
}

} // namespace

namespace MessageReactionsDB {

bool upsert(int messageId, int userId, const std::string& reaction, pqxx::work& tx) {
    if (messageId <= 0 || userId <= 0 || reaction.empty())
        return false;

    try {
        pqxx::result r = tx.exec_params(
            R"(
                INSERT INTO message_reactions (message_id, user_id, reaction)
                VALUES ($1, $2, $3)
                ON CONFLICT (message_id, user_id)
                DO UPDATE SET
                    reaction = EXCLUDED.reaction,
                    created_at = now()
                RETURNING message_id
            )",
            messageId,
            userId,
            reaction);

        return !r.empty();
    } catch (const pqxx::sql_error& e) {
        std::cerr << "SQL error (MessageReactionsDB::upsert): " << e.what()
                  << "\nQuery: " << e.query() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error (MessageReactionsDB::upsert): " << e.what() << std::endl;
    }

    return false;
}

bool remove(int messageId, int userId, pqxx::work& tx) {
    if (messageId <= 0 || userId <= 0)
        return false;

    try {
        pqxx::result r = tx.exec_params(
            R"(
                DELETE FROM message_reactions
                WHERE message_id = $1
                  AND user_id = $2
                RETURNING message_id
            )",
            messageId,
            userId);

        return !r.empty();
    } catch (const pqxx::sql_error& e) {
        std::cerr << "SQL error (MessageReactionsDB::remove): " << e.what()
                  << "\nQuery: " << e.query() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error (MessageReactionsDB::remove): " << e.what() << std::endl;
    }

    return false;
}

std::optional<MessageReaction> get(int messageId, int userId, pqxx::work& tx) {
    if (messageId <= 0 || userId <= 0)
        return std::nullopt;

    try {
        pqxx::result r = tx.exec_params(
            R"(
                SELECT message_id, user_id, reaction
                FROM message_reactions
                WHERE message_id = $1
                  AND user_id = $2
                LIMIT 1
            )",
            messageId,
            userId);

        if (r.empty())
            return std::nullopt;

        return mapRow(r[0]);
    } catch (const pqxx::sql_error& e) {
        std::cerr << "SQL error (MessageReactionsDB::get): " << e.what()
                  << "\nQuery: " << e.query() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error (MessageReactionsDB::get): " << e.what() << std::endl;
    }

    return std::nullopt;
}

std::vector<MessageReaction> getForMessageIds(const std::vector<int>& messageIds, pqxx::work& tx) {
    std::vector<MessageReaction> reactions;
    if (messageIds.empty())
        return reactions;

    try {
        pqxx::result r = tx.exec_params(
            R"(
                SELECT message_id, user_id, reaction
                FROM message_reactions
                WHERE message_id = ANY($1)
                ORDER BY message_id, user_id
            )",
            pqxx::params{messageIds});

        reactions.reserve(r.size());
        for (const auto& row : r)
            reactions.push_back(mapRow(row));
    } catch (const pqxx::sql_error& e) {
        std::cerr << "SQL error (MessageReactionsDB::getForMessageIds): " << e.what()
                  << "\nQuery: " << e.query() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error (MessageReactionsDB::getForMessageIds): " << e.what() << std::endl;
    }

    return reactions;
}

} // namespace MessageReactionsDB
