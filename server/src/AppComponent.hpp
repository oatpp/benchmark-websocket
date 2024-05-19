//
//  AppComponent.hpp
//  oatpp-web-starter
//
//  Created by Leonid on 3/2/18.
//  Copyright © 2018 lganzzzo. All rights reserved.
//

#ifndef AppComponent_hpp
#define AppComponent_hpp

#include "controller/WebSocketListener.hpp"

#include "oatpp-websocket/AsyncConnectionHandler.hpp"

#include "oatpp/web/server/AsyncHttpConnectionHandler.hpp"
#include "oatpp/web/server/HttpRouter.hpp"
#include "oatpp/network/tcp/server/ConnectionProvider.hpp"

#include "oatpp/json/ObjectMapper.hpp"

#include "oatpp/macro/component.hpp"

#include "oatpp/base/CommandLineArguments.hpp"
#include "oatpp/utils/Conversion.hpp"

#include <list>

/**
 *  Class which creates and holds Application components and registers components in oatpp::Environment
 *  Order of components initialization is from top to bottom
 */
class AppComponent {
private:
  oatpp::base::CommandLineArguments m_cmdArgs;
public:
  AppComponent(const oatpp::base::CommandLineArguments& cmdArgs)
    : m_cmdArgs(cmdArgs)
  {}
public:
  
  /**
   *  Create List of 100 Connection Providers listening on ports from 8000..8099
   */
  OATPP_CREATE_COMPONENT(std::shared_ptr<std::list<std::shared_ptr<oatpp::network::ServerConnectionProvider>>>, connectionProviders)([this] {
    auto providers = std::make_shared<std::list<std::shared_ptr<oatpp::network::ServerConnectionProvider>>>();
    v_uint16 basePort = oatpp::utils::Conversion::strToInt32(m_cmdArgs.getNamedArgumentValue("--bp", "8000"));
    v_uint16 portsCount = oatpp::utils::Conversion::strToInt32(m_cmdArgs.getNamedArgumentValue("--pc", "100"));
    for(v_uint16 i = 0; i < portsCount; i++) {
      OATPP_LOGd("AppComponent", "Connection Provider for port: {}", basePort + i);
      auto provider = oatpp::network::tcp::server::ConnectionProvider::createShared({"0.0.0.0", (v_uint16)(basePort + i)});
      providers->push_back(provider);
    }
    return providers;
  }());
  
  /**
   *  Create Router component
   */
  OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::web::server::HttpRouter>, httpRouter)([] {
    return oatpp::web::server::HttpRouter::createShared();
  }());
  
  /**
   *  Create HTTP ConnectionHandler component which uses Router component to route requests
   */
  OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::network::ConnectionHandler>, serverConnectionHandler)([] {
    OATPP_COMPONENT(std::shared_ptr<oatpp::web::server::HttpRouter>, router); // get Router component
    return oatpp::web::server::AsyncHttpConnectionHandler::createShared(router, 4);
  }());

  /**
   *  Create ObjectMapper component to serialize/deserialize DTOs in Contoller's API
   */
  OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::data::mapping::ObjectMapper>, apiObjectMapper)([] {
    return std::make_shared<oatpp::json::ObjectMapper>();
  }());

  /**
   * Create AsyncExecutor for WebSocket Asynchronous Connection Handler
   */
  OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::async::Executor>, executor)([this] {
    v_int32 threadsProc = oatpp::utils::Conversion::strToInt32(m_cmdArgs.getNamedArgumentValue("--tp", "8"));
    v_int32 threadsIO = oatpp::utils::Conversion::strToInt32(m_cmdArgs.getNamedArgumentValue("--tio", "2"));
    v_int32 threadsTimer = oatpp::utils::Conversion::strToInt32(m_cmdArgs.getNamedArgumentValue("--tt", "1"));
    OATPP_LOGd("Server", "Executor: tp={}, tio={}, tt={}", threadsProc, threadsIO, threadsTimer);
    return std::make_shared<oatpp::async::Executor>(threadsProc, threadsIO, threadsTimer);
  }());

  /**
   *  Create websocket connection handler
   */
  OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::websocket::AsyncConnectionHandler>, websocketConnectionHandler)([] {
    OATPP_COMPONENT(std::shared_ptr<oatpp::async::Executor>, executor);
    auto connectionHandler = oatpp::websocket::AsyncConnectionHandler::createShared(executor);
    connectionHandler->setSocketInstanceListener(std::make_shared<WebSocketInstanceListener>());
    return connectionHandler;
  }());

};

#endif /* AppComponent_hpp */
