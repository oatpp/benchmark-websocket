//
//  main.cpp
//  web-starter-project
//
//  Created by Leonid on 2/12/18.
//  Copyright © 2018 oatpp. All rights reserved.
//

#include "./controller/WebSocketListener.hpp"
#include "./controller/MyController.hpp"
#include "./AppComponent.hpp"
#include "./Logger.hpp"
#include "./Meter.hpp"

#include "oatpp/network/server/Server.hpp"

#include <iostream>
#include <thread>

void printStats() {

  OATPP_LOGD("Status", "\n\n\n\n\n");

  v_int64 lastTickCount = oatpp::base::Environment::getMicroTickCount();

  Meter framesMeter(10);
  Meter messagesMeter(10);

  while(true) {

    v_int64 tickCount = oatpp::base::Environment::getMicroTickCount();
    framesMeter.addPoint(tickCount, WebSocketListener::FRAMES.load());
    messagesMeter.addPoint(tickCount, WebSocketListener::MESSAGES.load());

    OATPP_LOGD("\33[2K\r\033[A\033[A\033[A\033[A\033[A"
               "SOCKETS", "          %d              ", WebSocketInstanceListener::SOCKETS.load());
    OATPP_LOGD("FRAMES_TOTAL", "     %d              ", WebSocketListener::FRAMES.load());
    OATPP_LOGD("MESSAGES_TOTAL", "   %d              ", WebSocketListener::MESSAGES.load());
    OATPP_LOGD("FRAMES_PER_SEC", "   %f              ", framesMeter.perSecond());
    OATPP_LOGD("MESSAGES_PER_SEC", " %f              ", messagesMeter.perSecond());
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

  }

};

void run() {
  
  AppComponent components; // Create scope Environment components
  
  /* create ApiControllers and add endpoints to router */
  
  auto router = components.httpRouter.getObject();
  
  auto myController = MyController::createShared();
  myController->addEndpointsToRouter(router);
  
  /* create servers */
  
  //oatpp::network::server::Server server(components.serverConnectionProvider.getObject(),
  //                                      components.serverConnectionHandler.getObject());
  
  //OATPP_LOGD("Server", "Running on port %s...", components.serverConnectionProvider.getObject()->getProperty("port").toString()->c_str());

  OATPP_COMPONENT(std::shared_ptr<std::list<std::shared_ptr<oatpp::network::ServerConnectionProvider>>>, connectionProviders);

  std::list<std::thread> threads;

  for(auto& provider : *connectionProviders) {
    threads.push_back(std::thread([provider]{
      OATPP_COMPONENT(std::shared_ptr<oatpp::network::server::ConnectionHandler>, handler);
      oatpp::network::server::Server server(provider, handler);
      server.run();
    }));
  }

  std::thread statThread([]{
    printStats();
  });
  
  for(auto& thread : threads) {
    thread.join();
  }

  statThread.join();
  
}

/**
 *  main
 */
int main(int argc, const char * argv[]) {
  
  oatpp::base::Environment::setLogger(new Logger());
  oatpp::base::Environment::init();

  run();
  
  oatpp::base::Environment::setLogger(nullptr); ///< free Logger
  
  /* Print how much objects were created during app running, and what have left-probably leaked */
  /* Disable object counting for release builds using '-D OATPP_DISABLE_ENV_OBJECT_COUNTERS' flag for better performance */
  std::cout << "\nEnvironment:\n";
  std::cout << "objectsCount = " << oatpp::base::Environment::getObjectsCount() << "\n";
  std::cout << "objectsCreated = " << oatpp::base::Environment::getObjectsCreated() << "\n\n";
  
  oatpp::base::Environment::destroy();
  
  return 0;
}
