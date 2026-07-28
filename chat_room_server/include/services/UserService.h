#ifndef USERSERVICE_H
#define USERSERVICE_H
#include "../services/PresenceService.h"
#include "../models/User.h"
#include "../SharedContext/OnlineUserRegistry.h"
#include "../services/Manager.h"
#include "../MessageStatuses/MessageStatuses.h"
#include "../include/NetworkAction/ApplicationResponse.h"
#include "ResponseType.h"
#include <optional>
#include <vector>

struct LoginResult {
    std::vector<ApplicationResponse> responses;
    std::optional<int> oldSessionToDisconnect;
};

class UserService {
public:
    UserService(Manager& manager, OnlineUserRegistry& registry, PresenceService& service)
        : manager(manager), userRegistry(registry), presenceService(service) {}

        LoginResult login(const std::string& username,
                          const std::string& password,
                          int sessionId);

        /** JWT re-bind after reconnect; same registry/presence side effects as login success. */
        LoginResult resumeSession(int userId, const std::string& userName, int sessionId);

        std::vector<ApplicationResponse>  create(const User& user, int sessionId);
        std::vector<ApplicationResponse>  uploadPictureProfile(const UploadProfilePictureRequest& upload);
        std::vector<ApplicationResponse>   uploadImageMessage(const UploadImageMessageRequest& upload);

    std::vector<ApplicationResponse> logout(int sessionId);

    /** Builds SEARCH_QUERY_RESPONSE with SearchQuerySuccessPayload (DB via Manager). */
    ApplicationResponse searchQueryResponse(const std::string& searchedCharacters,int searcherQueryId,  int requestingUserId, int sessionId);

private:

std::vector<ApplicationResponse> handleUploadInit(const UploadProfilePictureRequest& upload);
std::vector<ApplicationResponse> handleUploadCommit( const UploadProfilePictureRequest& upload);
std::vector<ApplicationResponse> ImageMessageInitUpload(const UploadImageMessageRequest& upload);
std::vector<ApplicationResponse> imageMessageCommitUpload( const UploadImageMessageRequest& upload);
std::vector<ApplicationResponse> imageMessageFinalize(const UploadImageMessageRequest& upload);

    Manager& manager;
    OnlineUserRegistry& userRegistry;
    PresenceService& presenceService;
};

#endif
