//
// Created by Leonid  on 2019-03-25.
//

#ifndef MY_PROJECT_WEBSOCKETTEST_HPP
#define MY_PROJECT_WEBSOCKETTEST_HPP


#include "oatpp-test/UnitTest.hpp"

class WebSocketTest : public oatpp::test::UnitTest {
public:

  WebSocketTest():UnitTest("TEST[WebSocketTest]"){}
  void onRun() override;

};


#endif //MY_PROJECT_WEBSOCKETTEST_HPP
