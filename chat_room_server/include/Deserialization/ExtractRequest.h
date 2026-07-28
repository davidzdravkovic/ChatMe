#ifndef EXTRACTREQUEST_H
#define EXTRACTREQUEST_H
#include "./DTO/RequestStruct.h"
#include <string>

class ExtractRequest {

void setType(RequestStruct &rStruct);

public:
RequestStruct extract(const std::string &json);

};

#endif