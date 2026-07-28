#include "../include/services/UserService.h"
#include "../include/Auth/MediaAccessToken.h"
#include <chrono>
#include <optional>
#include <stdexcept>
#include <iostream>

static auto now() {
    return std::chrono::steady_clock::now();
}

namespace {

ApplicationResponse loginFailure(int sessionId, std::string reason) {
    ApplicationResponse res{};
    res.type = ResponseType::LOGIN_RESPONSE;
    res.target = UnicastToSession{sessionId};
    res.intent = ResponseIntent::SEND;
    res.payload = LoginFailurePayload{std::move(reason)};
    return res;
}

ApplicationResponse createFailure(int sessionId, std::string reason) {
    ApplicationResponse res{};
    res.type = ResponseType::CREATE_RESPONSE;
    res.target = UnicastToSession{sessionId};
    res.intent = ResponseIntent::SEND;
    res.payload = CreateFailurePayload{std::move(reason)};
    return res;
}

ApplicationResponse authFailure(int sessionId, std::string reason) {
    ApplicationResponse res{};
    res.type = ResponseType::AUTH_RESPONSE;
    res.target = UnicastToSession{sessionId};
    res.intent = ResponseIntent::SEND;
    res.payload = AuthFailurePayload{std::move(reason)};
    return res;
}

ApplicationResponse authSuccess(int sessionId, int userId, std::string userName) {
    ApplicationResponse res{};
    res.type = ResponseType::AUTH_RESPONSE;
    res.target = UnicastToSession{sessionId};
    res.intent = ResponseIntent::SEND;
    AuthSuccessPayload payload{userId, std::move(userName), {}, {}};
    payload.profileUrl = media_access::profileReadPath(userId, userId);
    res.payload = payload;
    return res;
}

} // namespace

LoginResult UserService::login(const std::string& username, const std::string& password,int sessionId) {
    //Db verification on credentials
    auto userOpt = manager.logIn(username, password);
    if (!userOpt.has_value()) 
    {
        return LoginResult{ {loginFailure(sessionId, "Invalid username or password")}, std::nullopt};
    }

    User user = std::move(*userOpt);

    //Creates a session entry and returns if it is first or one of many session for the user
    const bool firstSession = userRegistry.add(sessionId, user.userID, user.userName);

    ApplicationResponse res{};
    res.type = ResponseType::LOGIN_RESPONSE;
    res.target = UnicastToSession{sessionId};
    res.intent = ResponseIntent::SEND;
    LoginSuccessPayload payload{user};
    payload.profileUrl = media_access::profileReadPath(user.userID, user.userID);
    res.payload = payload;

    std::vector<ApplicationResponse> responses;
    if (firstSession) {
        UserContext presenceCtx;
        presenceCtx.userId = user.userID;
        presenceCtx.userName = user.userName;
        responses = presenceService.onUserOnline(presenceCtx);
    }
    responses.push_back(res);

    return LoginResult{std::move(responses), std::nullopt};
}

LoginResult UserService::resumeSession(int userId,const std::string& userName, int sessionId) {
    

  //**Defensive idempotency path** 
  // The client is requesting this path always with new connection (session id)
    if (auto bound = userRegistry.findBySession(sessionId)) {
        if (bound->userId == userId && bound->userName == userName) {
            return LoginResult{{authSuccess(sessionId, userId, userName)}, std::nullopt};
        }
    }

    const bool firstSession = userRegistry.add(sessionId, userId, userName);

    std::vector<ApplicationResponse> responses;

    if (firstSession) {
        UserContext presenceCtx;
        presenceCtx.userId = userId;
        presenceCtx.userName = userName;
        responses = presenceService.onUserOnline(presenceCtx);
    }
    responses.push_back(authSuccess(sessionId, userId, userName));

    return LoginResult{std::move(responses), std::nullopt};
}

std::vector<ApplicationResponse> UserService::create(const User& user, int sessionId) {

    //Db user creation  
    auto createdOpt = manager.createUser(user);

    if (!createdOpt.has_value()) {
        return {createFailure(sessionId, "Username already taken or registration failed")};
    }

    User created = std::move(*createdOpt);

    userRegistry.add(sessionId, created.userID, created.userName);

    ApplicationResponse response{};
    response.type = ResponseType::CREATE_RESPONSE;
    response.target = UnicastToSession{sessionId};
    response.intent = ResponseIntent::SEND;
    response.payload = CreateSuccessPayload{created};

    return {response};
}

std::vector<ApplicationResponse> UserService::logout(int sessionId) {
    auto it = userRegistry.findBySession(sessionId);
    //If there is no value already removed ?
    if (!it.has_value()) {
        return {};
    }

    std::cout << "[disconnect/logout] userId=" << it->userId
              << " userName=" << it->userName
              << " sessionId=" << sessionId
              << std::endl;

    const UserContext ctx = it.value();
    const bool fullyOffline = userRegistry.removeBySession(sessionId);

    std::vector<ApplicationResponse> responses;
    if (fullyOffline) {
        const auto lastActiveAt = manager.updateLastActiveAt(ctx.userId);
        responses = presenceService.onUserOffline(ctx, lastActiveAt);
        presenceService.removeClient(ctx.userId);
    }
    return responses;
}

std::vector<ApplicationResponse> UserService::uploadPictureProfile(const UploadProfilePictureRequest& upload) {
    if (upload.stage == UploadStage::INIT) {
        return handleUploadInit(upload);
    }

    if (upload.stage == UploadStage::COMMIT) {
        return handleUploadCommit(upload);
    }

    UploadProfilePictureResponse r;
    r.approved = false;
    r.error = "Unknown upload stage";
    ApplicationResponse res;
    res.type = ResponseType::UPLOAD_PROFILE_PICTURE_RESPONSE;
    res.target = UnicastToSession{upload.sessionId};
    res.payload = UploadProfilePictureFailurePayload{r};
    return {res};
}

std::vector<ApplicationResponse> UserService::handleUploadInit(const UploadProfilePictureRequest& upload) {
    auto ctx = userRegistry.findBySession(upload.sessionId);
    if (!ctx.has_value()) {
        UploadProfilePictureResponse r;
        r.approved = false;
        r.error = "Not connected or session expired";
        ApplicationResponse res;
        res.type = ResponseType::UPLOAD_PROFILE_PICTURE_RESPONSE;
        res.target = UnicastToSession{upload.sessionId};
        res.payload = UploadProfilePictureFailurePayload{r};
        return {res};
    }

    try {
        int mediaId = manager.createMedia(upload.userId, upload.mimeType, upload.fileSizeBytes);

        UploadProfilePictureResponse uploadResponse;
        uploadResponse.approved = true;
        uploadResponse.uploadId = mediaId;
        uploadResponse.userId = upload.userId;
        uploadResponse.commitUrl = media_access::profileCommitPath(
            upload.userId, mediaId, upload.userId);

        ApplicationResponse res;
        res.type = ResponseType::UPLOAD_PROFILE_PICTURE_RESPONSE;
        res.target = UnicastToSession{upload.sessionId};
        res.payload = UploadProfilePicturePayload{uploadResponse};

        return {res};
    } catch (const std::exception& e) {
        UploadProfilePictureResponse r;
        r.approved = false;
        r.error = std::string("Could not start upload: ") + e.what();
        ApplicationResponse res;
        res.type = ResponseType::UPLOAD_PROFILE_PICTURE_RESPONSE;
        res.target = UnicastToSession{upload.sessionId};
        res.payload = UploadProfilePictureFailurePayload{r};
        return {res};
    }
}

std::vector<ApplicationResponse> UserService::handleUploadCommit(const UploadProfilePictureRequest& upload) {
    bool ok = manager.finalizeProfilePictureUpload(upload.userId, upload.uploadId);

    ApplicationResponse res;
    res.type = ResponseType::UPLOAD_PROFILE_PICTURE_FINISHED_RESPONSE;
    res.target = UnicastToSession{upload.sessionId};

    UploadProfilePictureResponse response;
    if (!ok) {
        response.error = "Upload commit failed (media not ready or DB error)";
        response.approved = false;
        res.payload = UploadProfilePictureFailurePayload{response};
    } else {
        response.uploadId = upload.uploadId;
        response.userId = upload.userId;
        response.approved = true;
        response.profileUrl = media_access::profileReadPath(upload.userId, upload.userId);
        res.payload = UploadProfilePicturePayload{response};
    }

    return {res};
}

std::vector<ApplicationResponse> UserService::uploadImageMessage(const UploadImageMessageRequest& upload) {
    if (upload.stage == UploadImageStage::INIT) {
        return ImageMessageInitUpload(upload);
    }

    if (upload.stage == UploadImageStage::COMMIT) {
        return imageMessageCommitUpload(upload);
    }
    if (upload.stage == UploadImageStage::FINALIZE) {
        return imageMessageFinalize(upload);
    }

    UploadImageMessageResponse r;
    r.approved = false;
    r.error = "Unknown image upload stage";
    r.clientTempId = upload.clientTempId;
    ApplicationResponse res;
    res.type = ResponseType::UPLOAD_IMAGE_MESSAGE_RESPONSE;
    res.target = UnicastToSession{upload.sessionId};
    res.payload = UploadImageMessagePayload{r};
    return {res};
}

std::vector<ApplicationResponse> UserService::ImageMessageInitUpload(const UploadImageMessageRequest& upload) {
    auto ctx = userRegistry.findBySession(upload.sessionId);
    if (!ctx.has_value()) {
        UploadImageMessageResponse r;
        r.approved = false;
        r.error = "Not connected or session expired";
        r.clientTempId = upload.clientTempId;
        ApplicationResponse res;
        res.type = ResponseType::UPLOAD_IMAGE_MESSAGE_RESPONSE;
        res.target = UnicastToSession{upload.sessionId};
        res.payload = UploadImageMessagePayload{r};
        return {res};
    }

    try {
        int mediaId = manager.createMedia(upload.userId, upload.mimeType, upload.fileSizeBytes);

        UploadImageMessageResponse uploadResponse;
        uploadResponse.approved = true;
        uploadResponse.uploadId = mediaId;
        uploadResponse.clientTempId = upload.clientTempId;
        uploadResponse.userId = upload.userId;
        uploadResponse.commitUrl = media_access::messageCommitPath(upload.userId, mediaId);

        ApplicationResponse res;
        res.type = ResponseType::UPLOAD_IMAGE_MESSAGE_RESPONSE;
        res.target = UnicastToSession{upload.sessionId};
        res.payload = UploadImageMessagePayload{uploadResponse};

        return {res};
    } catch (const std::exception& e) {
        UploadImageMessageResponse r;
        r.approved = false;
        r.error = std::string("Could not start image upload: ") + e.what();
        r.clientTempId = upload.clientTempId;
        ApplicationResponse res;
        res.type = ResponseType::UPLOAD_IMAGE_MESSAGE_RESPONSE;
        res.target = UnicastToSession{upload.sessionId};
        res.payload = UploadImageMessagePayload{r};
        return {res};
    }
}

std::vector<ApplicationResponse> UserService::imageMessageCommitUpload(const UploadImageMessageRequest& upload) {
    try {
        Message mess = manager.initialInsertionImageMessage(upload);
        mess.senderUserName = upload.senderUserName;
        mess.receiverUserName = upload.receiverUserName;

        std::vector<ApplicationResponse> responses;

        ApplicationResponse res;
        res.type = ResponseType::UPLOAD_IMAGE_MESSAGE_FINISHED_RESPONSE;
        res.target = UnicastToSession{upload.sessionId};

        UploadImageMessagePayload payload;
        payload.uploadImageMessageResponse.uploadId = mess.mediaID;
        payload.uploadImageMessageResponse.clientTempId = upload.clientTempId;
        payload.uploadImageMessageResponse.messageId = mess.messageID;
        payload.uploadImageMessageResponse.approved = true;

        res.payload = payload;
        responses.push_back(res);

        return responses;
    } catch (const std::exception& e) {
        UploadImageMessageResponse r;
        r.approved = false;
        r.error = std::string("Image commit failed: ") + e.what();
        r.clientTempId = upload.clientTempId;
        ApplicationResponse res;
        res.type = ResponseType::UPLOAD_IMAGE_MESSAGE_FINISHED_RESPONSE;
        res.target = UnicastToSession{upload.sessionId};
        res.payload = UploadImageMessagePayload{r};
        return {res};
    }
}

std::vector<ApplicationResponse> UserService::imageMessageFinalize(const UploadImageMessageRequest& upload) {
    try {
        Message mess = manager.finalizeImageMessageReady(upload);
        mess.senderUserName = upload.senderUserName;
        mess.receiverUserName = upload.receiverUserName;
        mess.clientTempId = upload.messageTempId;

        std::vector<ApplicationResponse> responses;

        ApplicationResponse ack;
        ack.type = ResponseType::MESSAGE_ACK_RESPONSE;
        ack.target = UnicastToSession{upload.sessionId};
        ack.payload = SendMessageAckPayload{
        media_access::messageForViewer(mess, upload.userId)};

        responses.push_back(ack);

        ApplicationResponse messageResponse;
        messageResponse.type = ResponseType::MESSAGE_RESPONSE;
        messageResponse.target = FanOutToUser{upload.receiverUserName};
        messageResponse.payload = SendMessageSuccessPayload{
        media_access::messageForViewer(mess, mess.receiverId)};

        ApplicationResponse senderSync;
        senderSync.type = ResponseType::MESSAGE_RESPONSE;
        senderSync.target = FanOutToUser{upload.senderUserName};
        senderSync.payload = SendMessageSuccessPayload{
        media_access::messageForViewer(mess, upload.userId)};

        responses.push_back(messageResponse);
        responses.push_back(senderSync);

        return responses;
    } catch (const std::exception& e) {
        UploadImageMessageResponse r;
        r.approved = false;
        r.error = std::string("Finalize failed: ") + e.what();
        r.clientTempId = upload.clientTempId;
        ApplicationResponse res;
        res.type = ResponseType::MESSAGE_IMAGE_FINALIZE_RESPONSE;
        res.target = UnicastToSession{upload.sessionId};
        res.payload = UploadImageMessagePayload{r};
        return {res};
    }
}

ApplicationResponse UserService::searchQueryResponse(const std::string& searchedCharacters,int searcherQueryId,int requestingUserId, int sessionId) {
    const int limit = 8;
    std::vector<User> users = manager.searchUsersByCharacters(searchedCharacters, requestingUserId, limit);

    std::vector<SearchUserHit> hits;
    hits.reserve(users.size());
    for (const auto& u : users) {
        SearchUserHit hit{u.userID, u.userName};
        hit.profileUrl = media_access::profileReadPath(requestingUserId, u.userID);
        hits.push_back(std::move(hit));
    }

    ApplicationResponse response{};
    response.type = ResponseType::SEARCH_QUERY_RESPONSE;
    response.target = UnicastToSession{sessionId};
    response.payload = SearchQuerySuccessPayload{searcherQueryId, std::move(hits)};
    return response;
}
