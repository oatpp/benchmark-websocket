//
// Created by Leonid  on 2019-03-25.
//

#ifndef MY_PROJECT_WEBSOCKETLISTENER_HPP
#define MY_PROJECT_WEBSOCKETLISTENER_HPP

#include "oatpp-websocket/AsyncConnectionHandler.hpp"
#include "oatpp-websocket/AsyncWebSocket.hpp"

class WebSocketListener : public oatpp::websocket::AsyncWebSocket::Listener {
public:
  static std::atomic<v_int32> MESSAGES;
  static std::atomic<v_int32> FRAMES;
private:
  oatpp::data::stream::ChunkedBuffer m_messageBuffer;
public:

  virtual Action onPing(oatpp::async::AbstractCoroutine* parentCoroutine,
                        Action&& actionOnReturn,
                        const std::shared_ptr<AsyncWebSocket>& socket,
                        const oatpp::String& message) override
  {
    ++ FRAMES;
    return socket->sendPongAsync(parentCoroutine, actionOnReturn, message);
  }

  virtual Action onPong(oatpp::async::AbstractCoroutine* parentCoroutine,
                        Action&& actionOnReturn,
                        const std::shared_ptr<AsyncWebSocket>& socket,
                        const oatpp::String& message) override
  {
    ++ FRAMES;
    return actionOnReturn;
  }

  virtual Action onClose(oatpp::async::AbstractCoroutine* parentCoroutine,
                         Action&& actionOnReturn,
                         const std::shared_ptr<AsyncWebSocket>& socket,
                         v_word16 code, const oatpp::String& message) override
  {
    ++ FRAMES;
    return actionOnReturn;
  }

  virtual Action readMessage(oatpp::async::AbstractCoroutine* parentCoroutine,
                             Action&& actionOnReturn,
                             const std::shared_ptr<AsyncWebSocket>& socket,
                             p_char8 data, oatpp::data::v_io_size size) override
  {
    if(size == 0) {
      auto wholeMessage = m_messageBuffer.toString();
      m_messageBuffer.clear();
      ++ MESSAGES;
      return socket->sendOneFrameTextAsync(parentCoroutine, actionOnReturn, "Hello from oatpp!: " + wholeMessage);
    } else if(size > 0) {
      ++ FRAMES;
      m_messageBuffer.write(data, size);
    }
    return actionOnReturn;
  }

};

class WebSocketInstanceListener : public oatpp::websocket::AsyncConnectionHandler::SocketInstanceListener {
public:
  static std::atomic<v_int32> SOCKETS;
private:
  static constexpr const char* const TAG = "WebSocketInstanceListener";
public:

  /**
   *  Called when socket is created
   */
  void onAfterCreate_NonBlocking(const std::shared_ptr<WebSocketListener::AsyncWebSocket>& socket) override {
    ++ SOCKETS;
    socket->setListener(std::make_shared<WebSocketListener>());
  }

  /**
   *  Called before socket instance is destroyed.
   */
  void onBeforeDestroy_NonBlocking(const std::shared_ptr<WebSocketListener::AsyncWebSocket>& socket) override {
    -- SOCKETS;
  }

};


#endif //MY_PROJECT_WEBSOCKETLISTENER_HPP
