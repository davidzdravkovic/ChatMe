#include "../include/Dispatcher/Dispatcher.h"
#include "../include/Deserialization/DeserializeLogIn.h"
#include "../include/Deserialization/DTO/LogStruct.h"
#include "../include/Serialization/Stringify.h"
#include "../include/Serialization/Serialization.h"
#include "../include/Policy/TrafficPolicy.h"
#include "../include/models/User.h"
#include <iostream>

  void Dispatcher :: dispatchRequest (Lane lane,int threadNumber) {
   //**I need to add here authentication later to unload the IO thread cpu cost**

   //fast and slow are shared contexts they are only 2 instances with 2 queues from which the dispatcher can pop up messages requests.
   //The identifier which one is going to be used is based on the enum  FAST enum -> fast shared context, SLOW enum -> slow shared context
   //Eventually 2 threads in the same time are going to seek from the fast queue which is 1, and 7 threads are seeking messages from slow queue which is also 1 queue.
   auto &laneType = (lane == Lane :: Fast) ? fast : slow;
   RequestStruct request;
   
    while(laneType.pop(request)) { 
    auto decision = Policy::TrafficPolicy::decide(request.rType);
    std::vector<NetworkAction> actions = handler.routeToManager(request);

    //If it is ordering type add extra network action that is going to dequeue next ordered message for the same client
    if (decision.requires_ordering) {
        for (auto& action : actions)
            action.ordered = true;

        NetworkAction release;
        release.type = NetworkAction::ActionType::RELEASE_ORDERED_GATE;
        release.sessionId = request.sessionID;
        actions.push_back(release);
    }

    laneType.dispatch(actions);
    }
    
  }

void Dispatcher::workingThreadsRun() {

    for (int i = 0; i < 2; ++i) {
        int threadNumber=i;
        fastWorkers.emplace_back([this,threadNumber] {
            dispatchRequest(Lane::Fast,threadNumber);
        });
        fastWorkers.back().detach();
    }

    for (int i = 0; i < 7; ++i) {
         int threadNumber=i;
        slowWorkers.emplace_back([this,threadNumber] {
            dispatchRequest(Lane::Slow,threadNumber);
        });
        slowWorkers.back().detach();
    }
}
