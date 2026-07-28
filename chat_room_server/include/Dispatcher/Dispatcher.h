#ifndef DISPATCHER_H
#define DISPATCHER_H
#include "./Deserialization/ExtractRequest.h"
#include "./SharedContext/SharedContext.h"
#include "./Handler/handler.h"
#include <vector>
#include <thread>
#include <queue>

class Dispatcher {
ExtractRequest extractRequest;
SharedContext &fast;
SharedContext &slow;
std::vector<std::thread> fastWorkers;
std::vector<std::thread> slowWorkers;
Handler &handler;
  enum class Lane { Fast, Slow };

public:

Dispatcher(Handler &hand, SharedContext &fastContext, SharedContext &slowContext)
    : handler(hand), fast(fastContext), slow(slowContext) {};

//working thread function
void dispatchRequest(Lane lane,int threadNumber);
//making threads and labeling them
void workingThreadsRun();



};

#endif