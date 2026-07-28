#include "../include/Server/Server.h"

Server::Server()
    : configs(Configuration(Configuration::resolveConfigDirectory()).loadConfig()),
      database(configs.database, 9),
      trafficController(fastContext, slowContext),
      connects(trafficController),
      manager(database),
      presenceService(userRegistry),
      userService(manager, userRegistry, presenceService),
      messageService(manager, userRegistry, presenceService),
      handler(manager, userService, messageService, userRegistry),
      dispatcher(handler, fastContext, slowContext),
      network(configs.network, connects, trafficController)
{
    slowContext.setDispatcher(network.makeDispatcher());
    fastContext.setDispatcher(network.makeDispatcher());
}

void Server::run()
{
    dispatcher.workingThreadsRun();
    network.run();
}
