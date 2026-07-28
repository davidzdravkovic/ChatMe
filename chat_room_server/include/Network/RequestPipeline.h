#pragma once

#include "./RequestContext.h"
#include <functional>
#include <memory>

namespace ingress_pipeline {
    

/**
 * Result of running the ingress pipeline (drives Network::processRequest control flow).
 */
enum class RequestPipelineOutcome {
    /** Parsed, authenticated, ordering OK — caller should traffic.route(ctx.request) then readWs. */
    Dispatch,
    /** Auth rejected or ordering deferred (readWs may already be scheduled) — caller does nothing more. */
    Idle,
    /** Parse failure or missing client — caller should readWs once to keep the loop alive. */
    RescheduleRead,
};

struct RequestPipeline {
    /**
     * parse → auth → ordered-read gate.
     * @param getClient fresh lookup after parse (session may disappear mid-flight).
     */
    static RequestPipelineOutcome run(
        RequestContext& ctx,
        const std::function<std::shared_ptr<Client>(int sessionId)>& getClient,
        const std::function<void(int sessionId, const std::vector<uint8_t>& payload)>& sendIngressError,
        const std::function<void(int sessionId)>& scheduleReadWs);
};

} // namespace ingress_pipeline
