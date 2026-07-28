#ifndef DESERIALIZEMEDIAMESSAGEH
#define DESERIALIZEMEDIAMESSAGEH
#include "./DTO/UploadImageMessage.h"
#include "./DTO/RequestStruct.h"
#include <string>

class DeserializeMediaMessage {
public:
    UploadImageMessageRequest deserialize(const RequestStruct &reqStruct);
};

#endif
