#ifndef UPLOADIMAGEMESSAGEH
#define UPLOADIMAGEMESSAGEH

#include <string>
#include <cstdint>
enum class UploadImageStage {
    INIT,   
    COMMIT,
    FINALIZE 
};
struct UploadImageMessageRequest {
    int userId;
    int sessionId;
    //Cause of design of deserialization there is reduncency in he temp id !
    int clientTempId;
    int messageTempId;
    ////////////////////////////////////////////////////////////////////////

    UploadImageStage stage;

    std::string mimeType;
    std::uint64_t fileSizeBytes;

    std::string senderUserName;
    std::string receiverUserName;

   int uploadId;
};
struct UploadImageMessageResponse {
    bool approved = false;
    int uploadId;
    int clientTempId;
    int userId;
    /** Set after COMMIT (inserted message id); 0 on INIT. */
    int messageId = 0;
    /** Signed POST path from INIT, e.g. /media/message/commit/265?token=... */
    std::string commitUrl;
    std::string error;
};
#endif