#ifndef NETWORKACTION_H
#define NETWORKACTION_H
#include <cstdint>
#include <vector>
#include <string>

struct NetworkAction {

    enum class ActionType {
        SEND_TO_SESSION,
        /** Login/create replies: send only if session still in connections; else DISCONNECT cleanup. */
        SEND_AUTH_RESPONSE,
        SEND_TO_USERNAME,
        CHATROOM_ID,
        USER_OFFLINE,
        LOGOUT,
        FORCE_DISCONNECT,
        RELEASE_ORDERED_GATE,
        NOOP
    };

    ActionType type;

    int sessionId = -1;                       
    std::vector<uint8_t> payload;
    bool ordered = false;

    static NetworkAction noop() {
        NetworkAction a;
        a.type = ActionType::NOOP;
        return a;
    }
};

#endif