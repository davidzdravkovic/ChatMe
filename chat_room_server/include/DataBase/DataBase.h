#ifndef DATABASEPOOL_H
#define DATABASEPOOL_H
#include <pqxx/pqxx>
#include "./Configuration/AppConfig.h"
#include <vector>
#include <stack>
#include <mutex>
#include <condition_variable>
#include <memory>

class DataBasePool {
    std::vector<std::unique_ptr<pqxx::connection>> connections;
    std::stack<size_t> freeList;
    std::mutex mtx;
    std::condition_variable cv;

    static std::string makeConnString(const DatabaseConfig& cfg) {
        return
            "dbname=" + cfg.name +
            " user=" + cfg.user +
            " password=" + cfg.password +
            " host=" + cfg.host +
            " port=" + std::to_string(cfg.port);
    }

public:
    DataBasePool(const DatabaseConfig& cfg, size_t poolSize)  {
        auto connStr = makeConnString(cfg);

        connections.reserve(poolSize);
        for (size_t i = 0; i < poolSize; ++i) {
            connections.push_back(
                std::make_unique<pqxx::connection>(connStr)
            );
            freeList.push(i);  
        }
       
    }
   
    pqxx::connection& giveConn() {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [&] {return !freeList.empty();});
        size_t index = freeList.top();
        freeList.pop();
        return *connections[index];
    }
    void releaseConn(pqxx::connection& conn) {
        std::lock_guard<std::mutex> lock(mtx);
        for (size_t i = 0; i < connections.size(); ++i) {
            if (connections[i].get() == &conn) {
                freeList.push(i);
                cv.notify_one();
                return;
            }
        }
    }
    
};
#endif