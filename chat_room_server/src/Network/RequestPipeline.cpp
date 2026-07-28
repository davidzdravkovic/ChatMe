#include "../../include/Network/RequestPipeline.h"
#include "../../include/Network/RequestParser.h"
#include "../../include/Network/WsAuthMiddleware.h"
#include "../../include/Network/TrafficReadPolicy.h"
#include <string_view>

namespace ingress_pipeline {

RequestPipelineOutcome RequestPipeline::run(
    RequestContext& ctx,
    const std::function<std::shared_ptr<Client>(int sessionId)>& getClient,
    const std::function<void(int sessionId, const std::vector<uint8_t>& payload)>& sendIngressError,
    const std::function<void(int sessionId)>& scheduleReadWs) {

   //Validation of the JSON on basic fields with parse:sessionId, data, action, token
   //If this structure is broken is returned error
    try {
        ctx.request = RequestParser::parse(std::string_view(ctx.rawJson));
    } catch (const std::exception&) {
        return RequestPipelineOutcome::RescheduleRead;
    } catch (...) {
        return RequestPipelineOutcome::RescheduleRead;
    }

    if (ctx.request.rType == RequestType::ERROR_REQUEST)
        return RequestPipelineOutcome::RescheduleRead;
       
    //The client here is needed cause the pipeline itslef is writing to the client and i removing the client
    //Decision needs to be made outside this
    ctx.client = getClient(ctx.sessionId);

    if (!ctx.client)
        return RequestPipelineOutcome::Idle;

   //The client is invalid for sending further requests, one bool is the facade for now both cases, that can be extended after
    if (ctx.client->blocksIngress())
        return RequestPipelineOutcome::Idle;

    if(ctx.request.rType == RequestType::LOGOUT_REQUEST)
     //Block every request after this request 
        ctx.client->setLogOutPending(true);   

    //Authentication needs to be moved to the working thread
    if (!WsAuthMiddleware::authenticate(ctx.request, ctx.sessionId, ctx.client, sendIngressError))
        return RequestPipelineOutcome::Idle;

    if (!TrafficReadPolicy::acceptOrScheduleReorderedRead(ctx.request, *ctx.client, ctx.sessionId, scheduleReadWs))
        return RequestPipelineOutcome::Idle;



    return RequestPipelineOutcome::Dispatch;
}

} // namespace ingress_pipeline