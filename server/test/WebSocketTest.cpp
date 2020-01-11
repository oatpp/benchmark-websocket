//
// Created by Leonid  on 2019-03-25.
//

#include "WebSocketTest.hpp"

#include "controller/MyController.hpp"
#include "controller/WebSocketListener.hpp"

#include "oatpp-websocket/Connector.hpp"
#include "oatpp-websocket/AsyncConnectionHandler.hpp"

#include "oatpp/web/server/AsyncHttpConnectionHandler.hpp"

#include "oatpp/network/virtual_/client/ConnectionProvider.hpp"
#include "oatpp/network/virtual_/server/ConnectionProvider.hpp"
#include "oatpp/network/virtual_/Interface.hpp"

#include "oatpp-test/web/ClientServerTestRunner.hpp"

#include "oatpp/core/macro/component.hpp"

namespace {

class TestComponent {
public:

  OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::async::Executor>, executor)([] {
    return std::make_shared<oatpp::async::Executor>(2);
  }());

  OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::network::virtual_::Interface>, virtualInterface)([] {
    return oatpp::network::virtual_::Interface::obtainShared("virtualhost");
  }());

  OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::network::ServerConnectionProvider>, serverConnectionProvider)([] {
#ifdef OATPP_TEST_USE_PORT
    return oatpp::network::server::SimpleTCPConnectionProvider::createShared(OATPP_TEST_USE_PORT);
#else
    OATPP_COMPONENT(std::shared_ptr<oatpp::network::virtual_::Interface>, interface);
    return oatpp::network::virtual_::server::ConnectionProvider::createShared(interface);
#endif
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
    OATPP_COMPONENT(std::shared_ptr<oatpp::async::Executor>, executor);
    return oatpp::web::server::AsyncHttpConnectionHandler::createShared(router, executor);
  }());

  /**
   *  Create ObjectMapper component to serialize/deserialize DTOs in Contoller's API
   */
  OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::data::mapping::ObjectMapper>, apiObjectMapper)([] {
    auto serializerConfig = oatpp::parser::json::mapping::Serializer::Config::createShared();
    auto deserializerConfig = oatpp::parser::json::mapping::Deserializer::Config::createShared();
    deserializerConfig->allowUnknownFields = false;
    auto objectMapper = oatpp::parser::json::mapping::ObjectMapper::createShared(serializerConfig, deserializerConfig);
    return objectMapper;
  }());

  /**
   *  Create websocket connection handler
   */
  OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::websocket::AsyncConnectionHandler>, websocketConnectionHandler)([] {
    auto connectionHandler = oatpp::websocket::AsyncConnectionHandler::createShared();
    connectionHandler->setSocketInstanceListener(std::make_shared<WebSocketInstanceListener>());
    return connectionHandler;
  }());

  OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::network::ClientConnectionProvider>, clientConnectionProvider)([this] {
#ifdef OATPP_TEST_USE_PORT
    return oatpp::network::client::SimpleTCPConnectionProvider::createShared("127.0.0.1", OATPP_TEST_USE_PORT);
#else
    OATPP_COMPONENT(std::shared_ptr<oatpp::network::virtual_::Interface>, interface);
    return oatpp::network::virtual_::client::ConnectionProvider::createShared(interface);
#endif
  }());

  OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::websocket::Connector>, connector)([] {
    OATPP_COMPONENT(std::shared_ptr<oatpp::network::ClientConnectionProvider>, clientConnectionProvider);
    return oatpp::websocket::Connector::createShared(clientConnectionProvider);
  }());

};

class ClientSocketListener : public oatpp::websocket::AsyncWebSocket::Listener{
private:
  oatpp::data::stream::ChunkedBuffer m_messageBuffer;
public:

  CoroutineStarter onPing(const std::shared_ptr<AsyncWebSocket>& socket, const oatpp::String& message) override {
    return socket->sendPongAsync(message);
  }

  CoroutineStarter onPong(const std::shared_ptr<AsyncWebSocket>& socket, const oatpp::String& message) override {
    return nullptr;
  }

  CoroutineStarter onClose(const std::shared_ptr<AsyncWebSocket>& socket, v_word16 code, const oatpp::String& message) override {
    return nullptr;
  }

  CoroutineStarter readMessage(const std::shared_ptr<AsyncWebSocket>& socket, v_word8 opcode, p_char8 data, oatpp::v_io_size size) override {
    if(size == 0) {
      auto wholeMessage = m_messageBuffer.toString();
      m_messageBuffer.clear();
      OATPP_LOGD("client", "received %s", wholeMessage->c_str());
    } else if(size > 0) {
      m_messageBuffer.write(data, size);
    }
    return nullptr;
  }

};

class ClientSenderCoroutine : public oatpp::async::Coroutine<ClientSenderCoroutine> {
private:
  std::shared_ptr<oatpp::websocket::AsyncWebSocket> m_socket;
public:

  ClientSenderCoroutine(const std::shared_ptr<oatpp::websocket::AsyncWebSocket>& socket)
    : m_socket(socket)
  {}

  Action act() override {
    return waitRepeat(std::chrono::milliseconds(100));
    //return m_socket->sendOneFrameTextAsync(this, waitRetry(), "hello!");
  }

};

class ClientCoroutine : public oatpp::async::Coroutine<ClientCoroutine> {
private:
  OATPP_COMPONENT(std::shared_ptr<oatpp::async::Executor>, executor);
  OATPP_COMPONENT(std::shared_ptr<oatpp::websocket::Connector>, connector);
  std::shared_ptr<oatpp::websocket::AsyncWebSocket> socket;
public:

  Action act() override {
    return connector->connectAsync("ws").callbackTo(&ClientCoroutine::onConnected);
  }

  Action onConnected(const std::shared_ptr<oatpp::data::stream::IOStream>& connection) {
    socket = oatpp::websocket::AsyncWebSocket::createShared(connection, true);
    socket->setListener(std::make_shared<ClientSocketListener>());
    executor->execute<ClientSenderCoroutine>(socket);
    return socket->listenAsync().next(finish());
  }

};

}

void WebSocketTest::onRun() {

//  TestComponent component;
//
//  OATPP_COMPONENT(std::shared_ptr<oatpp::async::Executor>, executor);
//
//  oatpp::test::web::ClientServerTestRunner runner;
//
//  runner.addController(MyController::createShared());
//
//  runner.run([] {
//
//    OATPP_COMPONENT(std::shared_ptr<oatpp::async::Executor>, executor);
//
//    executor->execute<ClientCoroutine>();
//
//    executor->join();
//
//
//  }, std::chrono::minutes(10));

OATPP_LOGD(TAG, "TODO - write tests");

}