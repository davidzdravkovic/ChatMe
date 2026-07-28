#ifndef RESPONSE_PAYLOADS_H
#define RESPONSE_PAYLOADS_H
#include "../models/User.h"
#include "../Deserialization/DTO/ChatPreview.h"
#include "../Deserialization/DTO/ActiveStatus.h"
#include "../Deserialization/DTO/TypingDTO.h"
#include "../Deserialization/DTO/SeenDto.h"
#include "../Deserialization/DTO/UploadProfilePicture.h"
#include "../Deserialization/DTO/UploadImageMessage.h"
#include "../models/Messages.h"
#include <optional>
#include <string>
#include <vector>



struct CreateSuccessPayload {
    User user;
    /** Set only by Handler after successful create; serialized to wire key "token" in Stringify (single path). */
    std::string accessToken{};
};

struct CreateFailurePayload {
    std::string reason;
};



struct LoginSuccessPayload {
    User user;
    /** Set only by Handler after successful login; serialized to wire key "token" in Stringify (single path). */
    std::string accessToken{};
    /** Signed GET for own profile picture. */
    std::string profileUrl{};
};

struct LoginFailurePayload {
    std::string reason;
};

struct AuthSuccessPayload {
    int userId{0};
    std::string userName;
    /** Optional refresh; serialized as wire key "token" when set. */
    std::string accessToken{};
    /** Signed GET for own profile picture (re-bind after refresh). */
    std::string profileUrl{};
};

struct AuthFailurePayload {
    std::string reason;
};

struct FetchMessagesSuccessPayload {
    int identifier;
    std::vector<std::vector<Message>> messages;
    std::optional<int> lastSeenMessageId;
    std::optional<std::string> seenAt;
    std::optional<int> lastSeenIdByOther;
    std::optional<std::string> seenAtByOther;
    int otherUserId;
    std::string peerProfileUrl;
};

struct FetchMessagesFailurePayload {
    std::string reason;
};



struct SendMessageSuccessPayload {
    Message message;
};

struct SendMessageAckPayload {
    Message message;
};

struct SendMessageFailurePayload {
    std::string reason;
};


struct FirstMessageSuccessPayload {
    Message message;
};
struct FirstMessageFailurePayload {
    std::string reason;
};



struct ChatRoomIdSuccessPayload {
    int chatRoomId;
    int receiverId;
    std::string receiver_UserName;
};
struct ChatRoomIdFailurePayload {
    std::string reason;
};

/** Receiver username does not exist (e.g. search / first message / fetch). */
struct PeerUserNotFoundPayload {
    std::string reason;
    std::string peerUsername;
    /** Echoes client FETCH `identifier` (conversation epoch); 0 when not applicable. */
    int chatIdentifier = 0;
};


struct RecentChatRoomsSuccessPayload {
    std::vector<ChatPreview> rooms;
    /** Signed GET for the requesting user's own avatar (meta row on wire). */
    std::string selfProfileUrl;
};

struct RecentChatRoomsFailurePayload {
    std::string reason;
};

struct ActiveStatusUpdatePayload {
    ActiveStatus status;
};
struct ActiveStatusUpdateFailurePayload {
    std::string error;
};
struct UploadProfilePicturePayload {
 UploadProfilePictureResponse UploadPictureResponse;
};
struct UploadProfilePictureFailurePayload {
  UploadProfilePictureResponse UploadPictureResponse;
};
struct TypingPayload {
 TypingRequest typing;
};
struct TypingFailurePayload {
  TypingRequest typing;
  std::string error;
};
struct SeenSuccessPayload {
  SeenDTO seen;
};
struct UploadImageMessagePayload {
 UploadImageMessageResponse uploadImageMessageResponse;
};
struct FetchImagesIdPayload {
    std::vector<int> imagesId;
    /** Parallel to imagesId — signed read paths for gallery fetch. */
    std::vector<std::string> imageMediaUrls;
    int chatIdentifier = 0;
};

/** One row for username search (wire keys aligned with recent-chat user fields). */
struct SearchUserHit {
    int userId = 0;
    std::string userName;
    std::string profileUrl;
};

/** Echoes client `searcherQueryId` for stale-response filtering. */
struct SearchQuerySuccessPayload {
    int searcherQueryId = 0;
    std::vector<SearchUserHit> hits;
};

/** In-chat message search results; echoes client `searchQueryId` for stale-response filtering. */
struct MessageSearchSuccessPayload {
    int searchQueryId = 0;
    std::vector<Message> hits;
};

struct ReactionSuccessPayload {
    int messageId = 0;
    int chatRoomId = 0;
    int userId = 0;
    /** Empty when the user removed their reaction. */
    std::string reaction;
};

struct ReactionFailurePayload {
    std::string reason;
};

#endif
 