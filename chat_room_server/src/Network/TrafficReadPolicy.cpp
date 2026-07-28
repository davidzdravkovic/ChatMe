#include "../../include/Network/TrafficReadPolicy.h"
#include "../../include/Policy/TrafficPolicy.h"

namespace ingress_pipeline {

bool TrafficReadPolicy::acceptOrScheduleReorderedRead(RequestStruct& req,
                                                     Client& client,
                                                     int sessionId,
                                                     const std::function<void(int sid)>& scheduleReadWs) {
    const auto decision = Policy::TrafficPolicy::decide(req.rType);
    if (!decision.requires_ordering)
        return true;
    if (client.tryAcquireOrderedRead(req))
        return true;
    scheduleReadWs(sessionId);
    return false;
}

} // namespace ingress_pipeline
