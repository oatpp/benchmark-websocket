//
//  AppComponent.hpp
//  oatpp-web-starter
//
//  Created by Leonid on 3/2/18.
//  Copyright © 2018 lganzzzo. All rights reserved.
//

#ifndef AppComponent_hpp
#define AppComponent_hpp

#include "oatpp/core/async/Executor.hpp"

#include "oatpp/network/client/SimpleTCPConnectionProvider.hpp"
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

  OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::async::Executor>, executor)([this] {
    v_int32 threads = oatpp::utils::conversion::strToInt32(m_cmdArgs.getNamedArgumentValue("-t", "2"));
    return std::make_shared<oatpp::async::Executor>(threads);
  }());

  OATPP_CREATE_COMPONENT(std::shared_ptr<std::list<std::shared_ptr<oatpp::network::ClientConnectionProvider>>>, connectionProviders)([this] {
    auto providers = std::make_shared<std::list<std::shared_ptr<oatpp::network::ClientConnectionProvider>>>();
    const char* host = m_cmdArgs.getNamedArgumentValue("-h", "127.0.0.1");
    v_int32 basePort = oatpp::utils::conversion::strToInt32(m_cmdArgs.getNamedArgumentValue("-p", "8000"));
    v_int32 portsCount = oatpp::utils::conversion::strToInt32(m_cmdArgs.getNamedArgumentValue("--ports-count", "100"));
    for(v_int32 i = 0; i < portsCount; i++) {
      auto provider = oatpp::network::client::SimpleTCPConnectionProvider::createShared(host, basePort + i);
      providers->push_back(provider);
    }
    return providers;
  }());


};

#endif /* AppComponent_hpp */
