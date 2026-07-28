#ifndef MESSAGESEARCHQUERY_H
#define MESSAGESEARCHQUERY_H
#include <string>

/**
 * In-chat message search request.
 * Matches client `createMessageSearchDTO`: data.receiverUserName, data.searchedText, data.searchQueryId.
 * The acting user (sender) is taken from the JWT, not the wire.
 */
struct MessageSearchQuery {
    int sessionId = 0;
    std::string receiverUserName;
    std::string searchedText;
    int searchQueryId = 0;
};

#endif
