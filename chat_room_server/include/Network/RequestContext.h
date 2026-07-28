#pragma once

#include "../Deserialization/DTO/RequestStruct.h"
#include "./Client.h"
#include <memory>
#include <string>

namespace ingress_pipeline {

/** Carries one ingress WebSocket message through the pipeline (parse → auth → policy). */
struct RequestContext {
    int sessionId{};
    //The whole JSON request 
    std::string rawJson;
    RequestStruct request{};
    std::shared_ptr<Client> client;
    
};

} // namespace ingress_pipeline
