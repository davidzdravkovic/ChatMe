#include "../include/Deserialization/DeserializeRoomId.h"

ChatRoomDTO DeserializeRoomId::deserializeRoomId(const RequestStruct &rStruct) {
    const auto& j = rStruct.data;

    dRoomId.sessionId = rStruct.sessionID;
    dRoomId.userId    = j.contains("userID") && !j["userID"].is_null()
                            ? j["userID"].get<int>()
                            : rStruct.authenticatedUserId;

    return dRoomId;
}
