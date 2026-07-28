#pragma once

#include "../Deserialization/DTO/RequestStruct.h"
#include "./Client.h"
#include <functional>

namespace ingress_pipeline {

/**
 * Per-read ordering gate (TrafficPolicy + Client ordered-read queue).
 * If deferred, calls scheduleReadWs(sessionId) and returns false.
 */
struct TrafficReadPolicy {
    static bool acceptOrScheduleReorderedRead(RequestStruct& req,
                                              Client& client,
                                              int sessionId,
                                              const std::function<void(int sid)>& scheduleReadWs);
};

} // namespace ingress_pipeline
