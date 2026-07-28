#include "../include/Deserialization/DeserializeMediaMessage.h"

namespace {

/** Prefer JSON; if absent or empty, use identity from JWT (same idea as userId). */
std::string senderUserNameOrJwt(const nlohmann::json& j, const RequestStruct& req) {
    if (j.contains("senderUserName") && !j["senderUserName"].is_null()) {
        const auto s = j["senderUserName"].get<std::string>();
        if (!s.empty())
            return s;
    }
    return req.authenticatedUserName;
}

} // namespace

UploadImageMessageRequest DeserializeMediaMessage::deserialize(const RequestStruct &reqStruct)
{
    UploadImageMessageRequest upload;
    const auto& j = reqStruct.data;

    upload.sessionId = reqStruct.sessionID;
    upload.userId    = j.contains("userId") && !j["userId"].is_null()
                           ? j["userId"].get<int>()
                           : reqStruct.authenticatedUserId;

    std::string stage = j["stage"].get<std::string>();

    if (stage == "INIT") {
        upload.stage         = UploadImageStage::INIT;
        upload.clientTempId  = j["clientId"].get<int>();
        upload.mimeType      = j["mimeType"].get<std::string>();
        upload.fileSizeBytes = j["fileSizeBytes"].get<std::uint64_t>();
    }
    else if (stage == "COMMIT") {
        upload.stage            = UploadImageStage::COMMIT;
        upload.clientTempId     = j["clientId"].get<int>();
        upload.uploadId         = j["uploadId"].get<int>();
        upload.senderUserName   = senderUserNameOrJwt(j, reqStruct);
        upload.receiverUserName = j["receiverUserName"].get<std::string>();
    }
    else if (stage == "FINALIZE") {
        upload.stage            = UploadImageStage::FINALIZE;
        upload.clientTempId     = j["clientId"].get<int>();
        upload.messageTempId    = j["messageTempId"].get<int>();
        upload.uploadId         = j["uploadId"].get<int>();
        upload.senderUserName   = senderUserNameOrJwt(j, reqStruct);
        upload.receiverUserName = j["receiverUserName"].get<std::string>();
    }

    return upload;
}
