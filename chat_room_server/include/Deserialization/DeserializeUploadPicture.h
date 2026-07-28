#ifndef DESERIALIZEUPLOADPROFILEPICTURE_H
#define DESERIALIZEUPLOADPROFILEPICTURE_H
#include "./DTO/UploadProfilePicture.h"
#include "./DTO/RequestStruct.h"
#include <string>

class DeserializeUploadProfilePicture {
public:
    UploadProfilePictureRequest deserialize(const RequestStruct &reqStruct);
};

#endif
