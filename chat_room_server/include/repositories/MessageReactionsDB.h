#ifndef MESSAGEREACTIONSDB_H
#define MESSAGEREACTIONSDB_H

#include "../models/MessageReaction.h"
#include <optional>
#include <pqxx/pqxx>
#include <string>
#include <vector>

namespace MessageReactionsDB {

/** Insert or replace this user's reaction on a message. */
bool upsert(int messageId, int userId, const std::string& reaction, pqxx::work& tx);

/** Remove this user's reaction on a message. Returns true if a row was deleted. */
bool remove(int messageId, int userId, pqxx::work& tx);

std::optional<MessageReaction> get(int messageId, int userId, pqxx::work& tx);

/** Load all reactions for the given message ids (empty input → empty vector). */
std::vector<MessageReaction> getForMessageIds(const std::vector<int>& messageIds, pqxx::work& tx);

} // namespace MessageReactionsDB

#endif
