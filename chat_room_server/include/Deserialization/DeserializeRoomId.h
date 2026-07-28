#ifndef DESERIALIZEROOMID_H
#define DESERIALIZEROOMID_H
#include "./DTO/RequestStruct.h"
#include "./DTO/ChatRoomDTO.h"

class DeserializeRoomId {
    ChatRoomDTO dRoomId;

public:
    ChatRoomDTO deserializeRoomId(const RequestStruct &rStruct);
};

#endif
