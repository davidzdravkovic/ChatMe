#ifndef TRAFICCONTROLLER_H
#define TRAFICCONTROLLER_H
#include "../SharedContext/SharedContext.h"
#include "../Policy/TrafficPolicy.h"
#include "../Deserialization/DTO/RequestStruct.h"
#include "../include/Deserialization/ExtractRequest.h"
#include <string>

class TrafficController {
public:
    TrafficController(SharedContext& fastCtx, SharedContext& slowCtx)
        : fast(fastCtx), slow(slowCtx) {}

    void route(const RequestStruct& req )
    {  
        auto decision = Policy::TrafficPolicy::decide(req.rType);

        if (decision.lane == Policy::TrafficLane::FAST) {
            fast.push(req);
        } else {
            slow.push(req);
        }

       
    }

private:
    SharedContext& fast;
    SharedContext& slow;
    ExtractRequest extractor;
};
#endif