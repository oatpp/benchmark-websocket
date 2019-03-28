//
//  Logger.hpp
//  oatpp-web-starter
//
//  Created by Leonid on 3/2/18.
//  Copyright © 2018 lganzzzo. All rights reserved.
//

#include "Logger.hpp"

#include <iostream>

void Logger::log(v_int32 priority, const std::string& tag, const std::string& message) {
  oatpp::concurrency::SpinLock lock(m_atom);
  std::cout << tag << ":" << message << "\n";
}