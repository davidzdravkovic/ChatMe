#ifndef SEENDBH
#define SEENDBH

#include <pqxx/pqxx>
#include <optional>
#include <string>

namespace SeenDB {

struct SeenRow {
    int lastSeenMessageId{0};
    std::optional<std::string> seenAt;
};

std::optional<SeenRow> fetchSeen(int chatID, int userID, pqxx::work& tx);
std::optional<SeenRow> upsertLastSeenMessage(int chatID, int userID, int lastSeenMessageID, pqxx::work& tx);

}

#endif
