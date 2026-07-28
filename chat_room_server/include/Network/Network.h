#ifndef NETWORK_H
#define NETWORK_H
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/beast/websocket.hpp>

#include "./Connections.h"
#if defined(_WIN32)
#include <winsock2.h>
#endif
#include <cstring>      
#include <cstdint>     
#include <iostream>
#include <unordered_map>
#include "./Deserialization/ExtractRequest.h"
#include "./NetworkAction/NetworkAction.h"
#include "./NetworkProtocol.h"
#include "./TrafficController/TrafficController.h"
#include "./Dispatcher/Dispatcher.h"
#include "./Configuration/AppConfig.h"
#include "./SharedContext/SharedContext.h"
#include <memory>


class Network {
private:
    void processRequest(int sessionId, std::string json);
    /** Single place to resolve session → Client (wraps Connections). */
    std::shared_ptr<Client> client(int sessionId);
    boost::asio::io_context io;
    boost::asio::ip::tcp::acceptor wsAcceptor;
    boost::asio::ip::tcp::endpoint wsEndpoint;

    TrafficController &traffic;
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type> workGuard;
    // io_context is run on kNetworkIoThreads worker threads (see Network::run()).
    Connections &activeConnects;

  
public:
    
    Network(const NetworkConfig& cfg, Connections &connects, TrafficController &trafficController)
    : activeConnects(connects),
      traffic(trafficController),
      wsEndpoint(boost::asio::ip::make_address(cfg.ip), cfg.wsPort),
      wsAcceptor(io),
      workGuard(boost::asio::make_work_guard(io)) {
        wsAcceptor.open(wsEndpoint.protocol());
        wsAcceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
        wsAcceptor.bind(wsEndpoint);
        wsAcceptor.listen();
    }

  
    void acceptWs();
    void readWs(int sessionId);
    void writeWs(int sessionId);
    void applyActions(const std::vector<NetworkAction> &actions);
    std::function<void(std::vector<NetworkAction>)> makeDispatcher();
    void run();
    void enqueueClientPayload(const int &sessionId, const std::vector<uint8_t> &payload);

    
};
#endif

//Constructor that has to be initialized with the IP address and port 
//On initializing the bind will be proceed


