/**
 * @file client.h
 *
 * @brief Implementation of client application.
 * 
 */

#pragma once

#include <iostream>
#include <sstream>
#include <WS2tcpip.h>
#include <thread>

namespace client_handler {

class ClientHandler {
 public:
  /**
   * @brief Constructor
   */
  ClientHandler() = default;

  /**
   * @brief Destructor
   */
  ~ClientHandler();

  /**
   * @brief Initializes Client handler.
   *
   * @return bool - True on success, false otherwise.
   */
  bool Init();

 private:
  /**
   * @brief Initialize WinSock.
   *
   * @return bool - true on success, false otherwise.
   */
  bool InitializeWinSock();

  /**
   * @brief Create Socket.
   *
   * @return bool - true on success, false otherwise.
   */
  bool CreateSocket();

  /**
   * @brief Connect to server.
   */
  void ConnectToServer();

  /**
   * @brief Client Thread for connecting to server and read/send message from/to server.
   */
  void ClientThread();

  SOCKET _sock;
  std::thread _client_thread;
};

}  // namespace client_handler

