#include "../include/Deserialization/DeserializeCreate.h"

CreateStruct DeserializeCreate::deserializeCreate(const RequestStruct &reqStruct) {
    CreateStruct cStruct;
    const auto& j = reqStruct.data;

    cStruct.sessionId = reqStruct.sessionID;
    cStruct.userName  = j.value("userName", std::string{});
    cStruct.password  = j.value("password", std::string{});
    cStruct.name      = j.value("name", std::string{});
    cStruct.email     = j.value("email", std::string{});

    return cStruct;
}
