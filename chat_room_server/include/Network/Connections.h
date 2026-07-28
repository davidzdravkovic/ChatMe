#ifndef CONNECTIONS_H
#define CONNECTIONS_H
#include <boost/asio.hpp>
#include <unordered_map>
#include "../TrafficController/TrafficController.h"
#include "./Client.h"
#include <mutex>

class Connections {
   
      int sessionID = 0;
      std::unordered_map<int, std::shared_ptr<Client>> connections;
      
     std::mutex mtx;
     TrafficController& trafficController;
     std::shared_ptr<Client> extractClient(int sessionId);

  public:
  
      Connections(TrafficController& controller)
        : trafficController(controller) { connections.reserve(1500); }

        
        int createClient(std::shared_ptr<WsStream> ws);
        void emitDisconnect (const int &sessionId); 
        std::shared_ptr<Client> getClient (const int &sessionId);

    
};

#endif