#include "../include/Deserialization/DeserializeLogIn.h"

LogStruct DeserializeLogIn::deserializeLog(const RequestStruct &reqStruct) {
    LogStruct log;
    const auto& j = reqStruct.data;

    log.sessionId = reqStruct.sessionID;
    log.userName  = j.value("username", std::string{});
    log.password  = j.value("password", std::string{});

    return log;
}
