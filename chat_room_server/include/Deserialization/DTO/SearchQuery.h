#ifndef SEARCHQUERY_H
#define SEARCHQUERY_H
#include <string>

/** Matches client `createSearchQueryDTO`: data.searchedCharacters, data.searcherQueryId; SessionId via envelope. */
struct SearchQuery {
    int sessionId = 0;
    std::string searchedCharacters;
    int searcherQueryId = 0;
    int userId;
};

#endif
