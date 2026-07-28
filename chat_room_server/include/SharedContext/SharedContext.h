#ifndef SHAREDCONTEXT_H
#define SHAREDCONTEXT_H
#include "./NetworkAction/NetworkAction.h" 
#include "./Deserialization/DTO/RequestStruct.h"
#include <queue>
#include <string>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <iostream>
#include <functional>


class SharedContext {
public:
  

    void setDispatcher(std::function<void(std::vector<NetworkAction>)> fn)
    {
        dispatcher = std::move(fn);
    }

  
    void push(RequestStruct req)
    {
        {
            std::lock_guard<std::mutex> lock(mutex);
            queue.push(std::move(req));
        }
        
        cv.notify_one();
    }

 
    bool pop(RequestStruct &req)
    {
        std::unique_lock<std::mutex> lock(mutex);
        cv.wait(lock,[&] {
            return !queue.empty() || stopped;
        });

        if (stopped && queue.empty())
            return false;

        req = std::move(queue.front());
        queue.pop();
        return true;
    }


    void dispatch(std::vector<NetworkAction> actions)
    {    
        
            if (dispatcher)
            dispatcher(std::move(actions));
    }

    void stop()
    {
        {
            std::lock_guard<std::mutex> lock(mutex);
            stopped = true;
        }
        cv.notify_all();
    }

private:
    std::queue<RequestStruct> queue;
    std::mutex mutex;
    std::condition_variable cv;
    bool stopped = false;

     std::function<void(std::vector<NetworkAction>)> dispatcher;
};

#endif
