#ifndef SERVER_H
#define SERVER_H
#include "./Network/Network.h"
#include "./Network/Connections.h"
#include "./services/PresenceService.h"
#include "./TrafficController/TrafficController.h"
#include "./Configuration/Configuration.h"
#include "./Configuration/AppConfig.h"
#include "./Dispatcher/Dispatcher.h"
#include "./services/Manager.h"
#include "./SharedContext/OnlineUserRegistry.h"
#include "./services/UserService.h"
#include "./services/MessageService.h"
#include "./DataBase/DataBase.h"
#include "./SharedContext/SharedContext.h"
#include "./Handler/handler.h"
#include <pqxx/pqxx> 
#include <thread>

class Server {
   
    AppConfig configs;
    SharedContext slowContext;
    SharedContext fastContext;
    DataBasePool database; 
    TrafficController trafficController;
    Connections connects;

    OnlineUserRegistry userRegistry;
    PresenceService presenceService;
    Manager manager;
    UserService userService;
    MessageService messageService;

    Handler handler;
    Dispatcher dispatcher;
    Network network;


public :

Server()
    : configs(Configuration(Configuration::resolveConfigDirectory()).loadConfig()),
      database(configs.database,9),
      trafficController(fastContext, slowContext),
      connects(trafficController),
      manager(database),
      presenceService(userRegistry),
      userService(manager, userRegistry, presenceService),
      messageService(manager, userRegistry,presenceService),
      handler(manager, userService, messageService,userRegistry),
      dispatcher(handler, fastContext, slowContext),
      network(configs.network, connects, trafficController)
{
    slowContext.setDispatcher(network.makeDispatcher());
    fastContext.setDispatcher(network.makeDispatcher());
}

       void run() 
       {       
            dispatcher.workingThreadsRun();
            network.run();
       };   


 
    
};


#endif