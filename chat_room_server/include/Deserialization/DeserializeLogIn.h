#ifndef DESERIALIZELOGIN_H
#define DESERIALIZELOGIN_H
#include "./DTO/LogStruct.h"
#include "./DTO/RequestStruct.h"

class DeserializeLogIn {
public:
    LogStruct deserializeLog(const RequestStruct &reqStruct);
};
#endif
