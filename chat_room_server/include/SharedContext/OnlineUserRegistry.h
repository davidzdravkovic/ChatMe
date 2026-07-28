#ifndef ONLINEUSERREGISTRY_H
#define ONLINEUSERREGISTRY_H
#include "UserContext.h"
#include <unordered_map>
#include <optional>
#include <mutex>
#include <vector>

using SessionId = int;
using Username  = std::string;

class OnlineUserRegistry {
public:
    /** Registers a session for the user. Returns true if this was the first active session. */
    bool add(SessionId sid, UserId userId, Username userName);

    /** Removes one session. Returns true if the user has no sessions left. */
    bool removeBySession(SessionId sid);

    std::optional<UserContext> findBySession(SessionId sid) const;
    std::vector<SessionId> findSessionIdsByUsername(const Username& name) const;
    std::vector<SessionId> findSessionIdsByUserId(UserId userId) const;
    std::optional<UserContext> findByUserId(UserId userId) const;

    bool isOnline(const Username& name) const;

private:
    mutable std::mutex mtx;

    std::unordered_map<UserId, UserContext> usersById;
    std::unordered_map<SessionId, UserId> userIdBySession;
    std::unordered_map<Username, UserId> userIdByUsername;
};

#endif
