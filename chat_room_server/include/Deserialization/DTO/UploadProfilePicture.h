#ifndef UPLOADPROFILEPICTURE_DTOS_H
#define UPLOADPROFILEPICTURE_DTOS_H

#include <string>
#include <cstdint>

enum class UploadStage {
    INIT,   
    COMMIT 
};


struct UploadProfilePictureRequest {
    int userId;
    int sessionId;

    UploadStage stage;

    std::string mimeType;
    std::uint64_t fileSizeBytes;

   int uploadId;
};


struct UploadProfilePictureResponse {
    bool approved = false;
    int uploadId;
    int userId;
    /** Signed POST path from INIT, e.g. /media/profile/commit/264/53?token=... */
    std::string commitUrl;
    /** Signed GET for profile after successful commit. */
    std::string profileUrl;
    std::string error;
};

#endif
