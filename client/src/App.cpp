//
//  main.cpp
//  web-starter-project
//
//  Created by Leonid on 2/12/18.
//  Copyright © 2018 oatpp. All rights reserved.
//

#include "./ClientSocketListener.hpp"
#include "./AppComponent.hpp"
#include "./Logger.hpp"
#include "./Meter.hpp"

#include <iostream>
#include <thread>

/**
 * Print Stats.
 */
void printStats() {

  OATPP_LOGD("Status", "\n\n\n\n\n");

  Meter framesMeter(60 * 2);
  Meter messagesMeter(60 * 2);

  while(true) {

    v_int64 tickCount = oatpp::base::Environment::getMicroTickCount();
    framesMeter.addPoint(tickCount, ClientSocketListener::FRAMES.load());
    messagesMeter.addPoint(tickCount, ClientSocketListener::MESSAGES.load());

    OATPP_LOGD("\33[2K\r\033[A\033[A\033[A\033[A\033[A"
               "SOCKETS", "          %d              ", ClientCoroutine::SOCKETS.load());
    OATPP_LOGD("FRAMES_TOTAL", "     %d              ", ClientSocketListener::FRAMES.load());
    OATPP_LOGD("MESSAGES_TOTAL", "   %d              ", ClientSocketListener::MESSAGES.load());
    OATPP_LOGD("FRAMES_PER_MIN", "   %f              ", framesMeter.perMinute());
    OATPP_LOGD("MESSAGES_PER_MIN", " %f              ", messagesMeter.perMinute());
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
  }

};

void run(const oatpp::base::CommandLineArguments& args) {
  
  AppComponent components(args); // Create scope Environment components

  /* Get list of connection providers */
  OATPP_COMPONENT(std::shared_ptr<std::list<std::shared_ptr<oatpp::network::ClientConnectionProvider>>>, connectionProviders);

  /* Get AsyncExecutor */
  OATPP_COMPONENT(std::shared_ptr<oatpp::async::Executor>, executor);

  v_int32 maxSocketsNumber = oatpp::utils::conversion::strToInt32(args.getNamedArgumentValue("--socks-max", "1000"));
  v_int32 maxSocketsPerPort = oatpp::utils::conversion::strToInt32(args.getNamedArgumentValue("--socks-port", "10000"));
  v_int32 sleepIteration = oatpp::utils::conversion::strToInt32(args.getNamedArgumentValue("--si", "5"));
  v_int32 sleepFor = oatpp::utils::conversion::strToInt32(args.getNamedArgumentValue("--sf", "5"));

  v_int32 clientsCounter = 0;

  for(auto& provider : *connectionProviders) {

    OATPP_LOGD("Client", "add clients for port %s", provider->getProperty("port").toString()->c_str());

    auto connector = oatpp::websocket::Connector::createShared(provider);

    v_int32 clientsToAdd = maxSocketsNumber - clientsCounter;
    if(clientsToAdd > maxSocketsPerPort) {
      clientsToAdd = maxSocketsPerPort;
    }

    for(v_int32 i = 0; i < clientsToAdd; i++) {
      executor->execute<ClientCoroutine>(connector);
      if(i % sleepIteration == 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(sleepFor));
      }
    }

    clientsCounter += clientsToAdd;

    if(clientsCounter >= maxSocketsNumber) {
      break;
    }

  }

  OATPP_LOGD("Client", "Waiting clients to connect...");

  std::thread statThread([]{
    printStats();
  });

  /* Wait all clients to connect */
  /* Do not start load until all clients are connected */
  while(ClientCoroutine::SOCKETS < maxSocketsNumber) {
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
  }


  /* Once all clients are connected - start load */

  oatpp::websocket::Config config;

  config.maskOutgoingMessages = true;
  config.readBufferSize = 64;

  for(auto& socket : ClientCoroutine::SOCKETS_LIST) {

    socket->setConfig(config);
    executor->execute<ClientSenderCoroutine>(socket);

  }

  executor->join();
  statThread.join();
  
}

/**
 *  main
 */
int main(int argc, const char * argv[]) {

  oatpp::base::Environment::init();

  run(oatpp::base::CommandLineArguments(argc, argv));
  
  /* Print how much objects were created during app running, and what have left-probably leaked */
  /* Disable object counting for release builds using '-D OATPP_DISABLE_ENV_OBJECT_COUNTERS' flag for better performance */
  std::cout << "\nEnvironment:\n";
  std::cout << "objectsCount = " << oatpp::base::Environment::getObjectsCount() << "\n";
  std::cout << "objectsCreated = " << oatpp::base::Environment::getObjectsCreated() << "\n\n";
  
  oatpp::base::Environment::destroy();
  
  return 0;
}
