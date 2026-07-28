#ifndef DESERIALIZETYPING_H
#define DESERIALIZETYPING_H
#include "./DTO/TypingDTO.h"
#include "./DTO/RequestStruct.h"
#include <string>

class DeserializeTyping {
public:
    TypingRequest deserializeTyping(const RequestStruct &reqStruct);
};

#endif
