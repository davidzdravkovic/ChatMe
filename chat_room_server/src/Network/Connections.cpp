#include "../include/Network/Connections.h"
#include <boost/beast/core.hpp>

int Connections::createClient(std::shared_ptr<WsStream> ws) {
    std::lock_guard<std::mutex> lock(mtx);
    if (connections.size() >= 1500) {
        boost::beast::get_lowest_layer(*ws).close();
        return 0;
    }
    ++sessionID;
    connections.emplace(
        sessionID,
        std::make_shared<Client>(std::move(ws), sessionID)
    );
    return sessionID;
}


std::shared_ptr<Client>  Connections :: getClient (const int &sessionId) {
    std::lock_guard<std::mutex> lock(mtx);
    auto it = connections.find(sessionId);
    if(it != connections.end()) {
  
      return it->second;

    }
    
   return nullptr;
}

std::shared_ptr<Client> Connections::extractClient(int sessionId) {
    std::lock_guard<std::mutex> lock(mtx);
    auto it = connections.find(sessionId);
    if (it == connections.end()) {
        return nullptr;
    }
    std::shared_ptr<Client> c = it->second;
    connections.erase(it);
    return c;
}

void Connections::emitDisconnect(const int& sessionId) {
    // Remove from the map first so new work keyed only by the map (getClient) cannot start.
    std::shared_ptr<Client> client = extractClient(sessionId);
    if (!client) {
        return;
    }

    // Pass the same `shared_ptr` as `c->beginClose(c, ...)` so the Client outlives `emitDisconnect`
    // and the async_close completion (the map no longer holds a ref).
    client->beginClose(client, [this](int sid) {
        RequestStruct req;
        req.rType = RequestType::DISCONNECT_REQUEST;
        req.sessionID = sid;
        req.data = std::to_string(sid);
        trafficController.route(req);
    });
}