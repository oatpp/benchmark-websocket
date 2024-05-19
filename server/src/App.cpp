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

#include "oatpp/network/Server.hpp"

#include <iostream>
#include <thread>

/**
 * Print Stats.
 */
void printStats() {

  OATPP_LOGd("Status", "\n\n\n\n\n");

  v_int64 lastTickCount = oatpp::Environment::getMicroTickCount();

  Meter framesMeter(60 * 2);
  Meter messagesMeter(60 * 2);

  while(true) {

    v_int64 tickCount = oatpp::Environment::getMicroTickCount();
    framesMeter.addPoint(tickCount, WebSocketListener::FRAMES.load());
    messagesMeter.addPoint(tickCount, WebSocketListener::MESSAGES.load());

    OATPP_LOGd("\33[2K\r\033[A\033[A\033[A\033[A\033[A"
               "SOCKETS", "          {}              ", WebSocketInstanceListener::SOCKETS.load());
    OATPP_LOGd("FRAMES_TOTAL", "     {}              ", WebSocketListener::FRAMES.load());
    OATPP_LOGd("MESSAGES_TOTAL", "   {}              ", WebSocketListener::MESSAGES.load());
    OATPP_LOGd("FRAMES_PER_MIN", "   {}              ", framesMeter.perMinute());
    OATPP_LOGd("MESSAGES_PER_MIN", " {}              ", messagesMeter.perMinute());
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

  }

};

void run(const oatpp::base::CommandLineArguments& args) {
  
  AppComponent components(args); // Create scope Environment components
  
  /* create ApiControllers and add endpoints to router */
  auto router = components.httpRouter.getObject();

  router->addController(MyController::createShared());
  
  /* create servers */
  OATPP_COMPONENT(std::shared_ptr<std::list<std::shared_ptr<oatpp::network::ServerConnectionProvider>>>, connectionProviders);

  std::list<std::thread> threads;

  for(auto& provider : *connectionProviders) {
    threads.push_back(std::thread([provider]{
      OATPP_COMPONENT(std::shared_ptr<oatpp::network::ConnectionHandler>, handler);
      oatpp::network::Server server(provider, handler);
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

  oatpp::Environment::init();

  run(oatpp::base::CommandLineArguments(argc, argv));

  /* Print how much objects were created during app running, and what have left-probably leaked */
  /* Disable object counting for release builds using '-D OATPP_DISABLE_ENV_OBJECT_COUNTERS' flag for better performance */
  std::cout << "\nEnvironment:\n";
  std::cout << "objectsCount = " << oatpp::Environment::getObjectsCount() << "\n";
  std::cout << "objectsCreated = " << oatpp::Environment::getObjectsCreated() << "\n\n";
  
  oatpp::Environment::destroy();
  
  return 0;
}
