#ifndef APPLICATION_RESPONSE_H
#define APPLICATION_RESPONSE_H
#include "ResponseType.h" 
#include "./TypesResponses.h"
#include "../SharedContext/UserContext.h"
#include <string>
#include <vector>
#include <optional>
#include <variant>

enum class ResponseIntent {
 SEND,
 NOOP
};

/** Fan-out: deliver to every active WebSocket session for this username. */
struct FanOutToUser {
    std::string username;
};

/** Unicast: deliver to one connection only (SessionId, not UserId). */
struct UnicastToSession {
    SessionId sessionId;
};

using ResponseTarget = std::variant<FanOutToUser, UnicastToSession>;

using ResponsePayload = std::variant<
    CreateSuccessPayload,
    CreateFailurePayload,

    LoginSuccessPayload,
    LoginFailurePayload,

    AuthSuccessPayload,
    AuthFailurePayload,

    FetchMessagesSuccessPayload,
    FetchMessagesFailurePayload,

    SendMessageSuccessPayload,
    SendMessageAckPayload,
    SendMessageFailurePayload,

    FirstMessageSuccessPayload,
    FirstMessageFailurePayload,

    ChatRoomIdSuccessPayload,
    ChatRoomIdFailurePayload,

    PeerUserNotFoundPayload,

    RecentChatRoomsSuccessPayload,
    RecentChatRoomsFailurePayload,

    ActiveStatusUpdatePayload,
    ActiveStatusUpdateFailurePayload,

    UploadProfilePicturePayload,
    UploadProfilePictureFailurePayload,

    TypingPayload,
    TypingFailurePayload,

    SeenSuccessPayload,
    UploadImageMessagePayload,

    FetchImagesIdPayload,

    SearchQuerySuccessPayload,

    MessageSearchSuccessPayload,

    ReactionSuccessPayload,
    ReactionFailurePayload
>;



struct ApplicationResponse {
    ResponseType type;
    ResponseTarget target;
    // Default: responses are sent unless explicitly suppressed.
    ResponseIntent intent = ResponseIntent::SEND;
    std::optional<ResponsePayload> payload;
};

#endif 
