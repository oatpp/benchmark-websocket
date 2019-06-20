//
// Created by Leonid  on 2019-03-25.
//

#ifndef MY_PROJECT_WEBSOCKETLISTENER_HPP
#define MY_PROJECT_WEBSOCKETLISTENER_HPP

#include "oatpp-websocket/AsyncConnectionHandler.hpp"
#include "oatpp-websocket/AsyncWebSocket.hpp"

/**
 * WebSocket listener listens on incoming WebSocket events.
 */
class WebSocketListener : public oatpp::websocket::AsyncWebSocket::Listener {
public:
  /**
   * Counter for received messages - counts complete messages only.
   */
  static std::atomic<v_int32> MESSAGES;

  /**
   * Counter for received frames.
   */
  static std::atomic<v_int32> FRAMES;
private:

  /**
   * Buffer for messages. Needed for multi-frame messages.
   */
  oatpp::data::stream::ChunkedBuffer m_messageBuffer;
public:

  /**
   * Called on "ping" frame.
   */
  CoroutineStarter onPing(const std::shared_ptr<AsyncWebSocket>& socket, const oatpp::String& message) override {
    ++ FRAMES;
    return socket->sendPongAsync(message);
  }

  /**
   * Called on "pong" frame
   */
  CoroutineStarter onPong(const std::shared_ptr<AsyncWebSocket>& socket, const oatpp::String& message) override {
    ++ FRAMES;
    return nullptr; // do nothing
  }

  /**
   * Called on "close" frame
   */
  CoroutineStarter onClose(const std::shared_ptr<AsyncWebSocket>& socket, v_word16 code, const oatpp::String& message) override {
    ++ FRAMES;
    return nullptr; // do nothing
  }

  /**
   * Called on each message frame. After the last message will be called once-again with size == 0 to designate end of the message.
   */
  CoroutineStarter readMessage(const std::shared_ptr<AsyncWebSocket>& socket, v_word8 opcode, p_char8 data, oatpp::data::v_io_size size) override {

    if(size == 0) { // message transfer finished

      auto wholeMessage = m_messageBuffer.toString();
      m_messageBuffer.clear();
      ++ MESSAGES;

      /* Send message in reply */
      return socket->sendOneFrameTextAsync( "Hello from oatpp!: " + wholeMessage);

    } else if(size > 0) { // message frame received
      ++ FRAMES;
      m_messageBuffer.write(data, size);
    }

    return nullptr; // do nothing

  }

};

/**
 * Listener on new WebSocket connections.
 */
class WebSocketInstanceListener : public oatpp::websocket::AsyncConnectionHandler::SocketInstanceListener {
public:
  /**
   * Counter for connected clients.
   */
  static std::atomic<v_int32> SOCKETS;
public:

  /**
   *  Called when socket is created
   */
  void onAfterCreate_NonBlocking(const std::shared_ptr<AsyncWebSocket>& socket, const std::shared_ptr<const ParameterMap>& params) override {
    ++ SOCKETS;

    /* In this particular case we create one WebSocketListener per each connection */
    /* Which may be redundant in many cases */
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
