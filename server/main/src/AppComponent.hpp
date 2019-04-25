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
#include "oatpp/network/server/SimpleTCPConnectionProvider.hpp"

#include "oatpp/parser/json/mapping/ObjectMapper.hpp"

#include "oatpp/core/macro/component.hpp"

#include "oatpp/core/base/CommandLineArguments.hpp"
#include "oatpp/core/utils/ConversionUtils.hpp"

#include <list>

/**
 *  Class which creates and holds Application components and registers components in oatpp::base::Environment
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
   *  Create ConnectionProvider component which listens on the port
   */
  OATPP_CREATE_COMPONENT(std::shared_ptr<std::list<std::shared_ptr<oatpp::network::ServerConnectionProvider>>>, connectionProviders)([] {
    auto providers = std::make_shared<std::list<std::shared_ptr<oatpp::network::ServerConnectionProvider>>>();
    v_int32 basePort = 8000;
    for(v_int32 i = 0; i < 100; i++) {
      OATPP_LOGD("AppComponent", "Connection Provider for port: %d", basePort + i);
      auto provider = oatpp::network::server::SimpleTCPConnectionProvider::createShared(basePort + i);
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
   *  Create ConnectionHandler component which uses Router component to route requests
   */
  OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::network::server::ConnectionHandler>, serverConnectionHandler)([] {
    OATPP_COMPONENT(std::shared_ptr<oatpp::web::server::HttpRouter>, router); // get Router component
    return oatpp::web::server::AsyncHttpConnectionHandler::createShared(router, 1);
  }());
  
  /**
   *  Create ObjectMapper component to serialize/deserialize DTOs in Contoller's API
   */
  OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::data::mapping::ObjectMapper>, apiObjectMapper)([] {
    return oatpp::parser::json::mapping::ObjectMapper::createShared();
  }());

  OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::async::Executor>, executor)([this] {
    v_int32 threadsProc = oatpp::utils::conversion::strToInt32(m_cmdArgs.getNamedArgumentValue("--tp", "8"));
    v_int32 threadsIO = oatpp::utils::conversion::strToInt32(m_cmdArgs.getNamedArgumentValue("--tio", "2"));
    v_int32 threadsTimer = oatpp::utils::conversion::strToInt32(m_cmdArgs.getNamedArgumentValue("--tt", "1"));
    OATPP_LOGD("Server", "Executor: tp=%d, tio=%d, tt=%d", threadsProc, threadsIO, threadsTimer);
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
