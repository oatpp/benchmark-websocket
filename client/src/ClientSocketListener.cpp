//
// Created by Leonid  on 2019-03-28.
//

#include "ClientSocketListener.hpp"

std::atomic<v_int32> ClientSocketListener::MESSAGES(0);
std::atomic<v_int32> ClientSocketListener::FRAMES(0);

std::atomic<v_int32> ClientCoroutine::SOCKETS(0);

std::list<std::shared_ptr<oatpp::websocket::AsyncWebSocket>> ClientCoroutine::SOCKETS_LIST;
std::mutex ClientCoroutine::SOCKETS_LIST_MUTEX;