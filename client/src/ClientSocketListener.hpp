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

/**
 * WebSocket listener listens on incoming WebSocket events.
 */
class ClientSocketListener : public oatpp::websocket::AsyncWebSocket::Listener{
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
  CoroutineStarter readMessage(const std::shared_ptr<AsyncWebSocket>& socket, v_word8 opcode, p_char8 data, oatpp::v_io_size size) override {

    if(size == 0) { // message transfer finished

      auto wholeMessage = m_messageBuffer.toString();
      // TODO do something with message
      m_messageBuffer.clear();
      ++ MESSAGES;

    } else if(size > 0) { // message frame received
      ++ FRAMES;
      m_messageBuffer.writeSimple(data, size);
    }

    return nullptr; // do nothing

  }

};

/**
 * Coroutine which generates load.
 */
class ClientSenderCoroutine : public oatpp::async::Coroutine<ClientSenderCoroutine> {
private:
  /**
   * WebSocket connection to load.
   */
  std::shared_ptr<oatpp::websocket::AsyncWebSocket> m_socket;
public:

  ClientSenderCoroutine(const std::shared_ptr<oatpp::websocket::AsyncWebSocket>& socket)
    : m_socket(socket)
  {}

  Action act() override {
    /* Send "hello!" message. Once message sent, send another one. */
    return m_socket->sendOneFrameTextAsync("hello!").next(yieldTo(&ClientSenderCoroutine::act));
  }

};

/**
 * Establish WebSocket connection and listen on WebSocket.
 */
class ClientCoroutine : public oatpp::async::Coroutine<ClientCoroutine> {
public:
  /**
   * Counter for established WebSocket connections.
   */
  static std::atomic<v_int32> SOCKETS;

  /**
   * List of established WebSocket connections.
   */
  static std::list<std::shared_ptr<oatpp::websocket::AsyncWebSocket>> SOCKETS_LIST;
private:
  static std::mutex SOCKETS_LIST_MUTEX;
private:
  /**
   * WebSocket connector. Used to connect on server/ws endpoint and upgrade connection to WebSocket.
   */
  std::shared_ptr<oatpp::websocket::Connector> m_connector;

  /**
   * Established WebSocket connection.
   */
  std::shared_ptr<oatpp::websocket::AsyncWebSocket> m_socket;
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
    /* Establish WebSocket connection */
    return m_connector->connectAsync("ws").callbackTo(&ClientCoroutine::onConnected);
  }


  Action onConnected(const std::shared_ptr<oatpp::data::stream::IOStream>& connection) {
    ++ SOCKETS;
    m_socket = oatpp::websocket::AsyncWebSocket::createShared(connection, true /* maskOutgoingMessages for clients always true */);

    /* In this particular case we create one WebSocketListener per each connection */
    /* Which may be redundant in many cases */
    m_socket->setListener(std::make_shared<ClientSocketListener>());

    {
      std::lock_guard<std::mutex> guard(SOCKETS_LIST_MUTEX);
      SOCKETS_LIST.push_back(m_socket);
    }

    /* Listen on WebSocket. */
    /* When WebSocket is closed - call onFinishListen() */
    return m_socket->listenAsync().next(yieldTo(&ClientCoroutine::onFinishListen));
  }

  Action onFinishListen() {
    OATPP_ASSERT(false && "onFinishListen - closed WebSocket considered to be an error in this benchmark");
    return finish();
  }

  Action handleError(Error* error) override {
    if(error) {
      OATPP_LOGD("ClientCoroutine", "Error. %s", error->what());
    }

    OATPP_ASSERT(false && "handleError - any error in WebSocket communication is considered to be fatal in this benchmark");
    return error;
  }

};


#endif //ClientSocketListener_hpp
