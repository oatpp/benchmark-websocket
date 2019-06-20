//
// Created by Leonid  on 2019-03-25.
//

#include "WebSocketListener.hpp"

std::atomic<v_int32> WebSocketListener::MESSAGES(0);
std::atomic<v_int32> WebSocketListener::FRAMES(0);

std::atomic<v_int32> WebSocketInstanceListener::SOCKETS(0);