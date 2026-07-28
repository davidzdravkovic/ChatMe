#ifndef DESERIALIZECREATE_H
#define DESERIALIZECREATE_H
#include "./DTO/CreateStruct.h"
#include "./DTO/RequestStruct.h"
#include <string>

class DeserializeCreate {
public:
    CreateStruct deserializeCreate(const RequestStruct &reqStruct);
};

#endif
