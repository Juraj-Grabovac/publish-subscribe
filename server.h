/**
 * @file server.h
 *
 * @brief Implementation of server application.
 * 
 */

#pragma once

#include <iostream>
#include <sstream>
#include <WS2tcpip.h>
#include <thread>

using namespace std;

namespace server_handler {

class ServerHandler {
 public:
  /**
   * @brief Constructor
   */
  ServerHandler() = default;

  /**
   * @brief Destructor
   */
  ~ServerHandler();

  /**
   * @brief Initializes Server handler.
   * 
   * @param [in] port_num - port number
   *
   * @return bool - True on success, false otherwise.
   */
  bool Init(int port_num);

 private:
  /**
   * @brief Initialize WinSock.
   *
   * @return bool - true on success, false otherwise.
   */
  bool InitializeWinSock();

  /**
   * @brief Create Listening Socket.
   *
   * @return bool - true on success, false otherwise.
   */
  bool CreateListeningSocket();

  /**
   * @brief Server Thread for listening message from client.
   */
  void ServerThread();

  SOCKET _listening;
  std::thread _server_thread;

  int _port_num;
};

}  // namespace server_handler

