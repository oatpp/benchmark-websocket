//
// Created by Leonid  on 2019-03-28.
//

#ifndef ClientSocketListener_hpp
#define ClientSocketListener_hpp

#include "oatpp/core/async/Executor.hpp"

#include "oatpp-websocket/Connector.hpp"
#include "oatpp-websocket/AsyncWebSocket.hpp"

#include "oatpp/core/macro/component.hpp"

#include <mutex>

class ClientSocketListener : public oatpp::websocket::AsyncWebSocket::Listener{
public:
  static std::atomic<v_int32> MESSAGES;
  static std::atomic<v_int32> FRAMES;
private:
  oatpp::data::stream::ChunkedBuffer m_messageBuffer;
public:

  virtual Action onPing(oatpp::async::AbstractCoroutine* parentCoroutine, const Action& actionOnReturn,
                        const std::shared_ptr<AsyncWebSocket>& socket, const oatpp::String& message) override
  {
    ++ FRAMES;
    return socket->sendPongAsync(parentCoroutine, actionOnReturn, message);
  }

  virtual Action onPong(oatpp::async::AbstractCoroutine* parentCoroutine, const Action& actionOnReturn,
                        const std::shared_ptr<AsyncWebSocket>& socket, const oatpp::String& message) override
  {
    ++ FRAMES;
    return actionOnReturn;
  }

  virtual Action onClose(oatpp::async::AbstractCoroutine* parentCoroutine, const Action& actionOnReturn,
                         const std::shared_ptr<AsyncWebSocket>& socket, v_word16 code, const oatpp::String& message) override
  {
    ++ FRAMES;
    return actionOnReturn;
  }

  virtual Action readMessage(oatpp::async::AbstractCoroutine* parentCoroutine, const Action& actionOnReturn,
                             const std::shared_ptr<AsyncWebSocket>& socket, p_char8 data, oatpp::data::v_io_size size) override
  {
    if(size == 0) {
      auto wholeMessage = m_messageBuffer.toString();
      // TODO do something with message
      m_messageBuffer.clear();
      ++ MESSAGES;
    } else if(size > 0) {
      ++ FRAMES;
      m_messageBuffer.write(data, size);
    }
    return actionOnReturn;
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
    return m_socket->sendOneFrameTextAsync(this, yieldTo(&ClientSenderCoroutine::doRepeat), "hello!");
  }

  Action doRepeat() {
    return yieldTo(&ClientSenderCoroutine::act);
  }

};

class ClientCoroutine : public oatpp::async::Coroutine<ClientCoroutine> {
public:
  static std::atomic<v_int32> SOCKETS;
  static std::list<std::shared_ptr<oatpp::websocket::AsyncWebSocket>> SOCKETS_LIST;
private:
  static std::mutex SOCKETS_LIST_MUTEX;
private:
  std::shared_ptr<oatpp::websocket::Connector> m_connector;
  std::shared_ptr<oatpp::websocket::AsyncWebSocket> m_socket;
  bool m_printLog;
public:

  ClientCoroutine(const std::shared_ptr<oatpp::websocket::Connector>& connector)
    : m_connector(connector)
    , m_socket(nullptr)
  {}

  ~ClientCoroutine() {
    if(m_socket) {
      -- SOCKETS;
    }
  }

  Action act() override {
    auto callback = static_cast<oatpp::websocket::Connector::AsyncCallback>(&ClientCoroutine::onConnected);
    return m_connector->connectAsync(this, callback, "ws");
  }

  Action onConnected(const std::shared_ptr<oatpp::data::stream::IOStream>& connection) {
    ++ SOCKETS;
    m_socket = oatpp::websocket::AsyncWebSocket::createShared(connection, true);
    m_socket->setListener(std::make_shared<ClientSocketListener>());
    {
      std::lock_guard<std::mutex> guard(SOCKETS_LIST_MUTEX);
      SOCKETS_LIST.push_back(m_socket);
    }
    return m_socket->listenAsync(this, yieldTo(&ClientCoroutine::onFinishListen));
  }

  Action onFinishListen() {
    OATPP_LOGD("aa", "finished listen--------------------------------------------------");
    OATPP_ASSERT(false && "onFinishListen");
    return finish();
  }

  Action handleError(const oatpp::async::Error& error) override {
    if(error.isExceptionThrown) {
      try{
        throw;
      } catch(std::runtime_error e) {
        OATPP_LOGD("aa", "error listen------------------------------------!!!!!!!!!!!!!!!!!! %s", e.what());
      } catch(...) {
        OATPP_LOGD("aa", "error listen------------------------------------!!!!!!!!!!!!!!!!!! %s", "unknown");
      }

    } else {
      OATPP_LOGD("aa", "error listen------------------------------------!!!!!!!!!!!!!!!!!! %s", error.message);
    }
    OATPP_ASSERT(false && "handleError");
    return error;
  }

};


#endif //ClientSocketListener_hpp
