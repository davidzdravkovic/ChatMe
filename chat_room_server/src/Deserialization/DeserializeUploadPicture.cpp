#include "../include/Deserialization/DeserializeUploadPicture.h"

UploadProfilePictureRequest DeserializeUploadProfilePicture::deserialize(const RequestStruct &reqStruct)
{
    UploadProfilePictureRequest upload;
    const auto& j = reqStruct.data;

    upload.sessionId = reqStruct.sessionID;
    upload.userId    = j.contains("userId") && !j["userId"].is_null()
                           ? j["userId"].get<int>()
                           : reqStruct.authenticatedUserId;

    std::string stage = j["stage"].get<std::string>();
    if (stage == "INIT") {
        upload.stage         = UploadStage::INIT;
        upload.mimeType      = j["mimeType"].get<std::string>();
        upload.fileSizeBytes = j["fileSizeBytes"].get<std::uint64_t>();
    }
    else if (stage == "COMMIT") {
        upload.stage    = UploadStage::COMMIT;
        upload.uploadId = j["uploadId"].get<int>();
    }

    return upload;
}
