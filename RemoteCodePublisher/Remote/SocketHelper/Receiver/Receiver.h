#pragma once
///////////////////////////////////////////////////////////////////////////////////
// Receiver.h  : Receives messages/files from multiple senders                   //
// ver 1.0                                                                       //
//                                                                               //
// Platform    : Dell Inspiron 5520. Windows 10 Professional. Visual Studio 15   //
// Application : CSE-687 OOD Project 4                                           //
// Author      : Ammar Salman, EECS Department, Syracuse University              //
//               (313)-788-4694, hoplite.90@hotmail.com                          //
///////////////////////////////////////////////////////////////////////////////////
/*
*  Package Description: this package uses Sockets.h to receive messages from 
*  differnet senders and it enQs them as strings to an outgoing queue.
*  The outgoing queue is essentially for the user of this package.
*  It defines ClientHandler which is activated when something connects to Receiver.
*
*  Public Interface:
* =================================
*  Receiver receiver;
*  receiver.save_directory("C:\\test");
*  receiver.Initialize("localhost", 9090);
*  HttpMessage msg = receiver.Receive();
*  receiver.Good();
*  receiver.port();
*/
/*
*  Required Files:
* =================================
*  Receiver.h Receiver.cpp
*  Cpp11-BlockingQueue.h
*  Sockets.h Sockets.cpp
*  HttpMessage.h HttpMessage.cpp
*  FileSystem.h FileSystem.cpp
*
*  Build Instructions:
* =================================
*  devenv Receiver.vcxproj /rebuild
*
*  Maintainence History:
* =================================
*  ver 1.0 - 29, April, 2017
*    - first release
*/

#include "../Sockets/Sockets.h"
#include "../../../SharedProjects/HttpMessage/HttpMessage.h"

namespace Communication {

  class ClientHandler {
  public:
    ClientHandler(BlockingQueue<HttpMessage>& inQ, std::string& sDir);
    void operator()(Socket socket);
    void Terminate();

  private:
    HttpMessage ReadMessage(Socket& socket);
    bool ReadAttributes(Socket& socket, HttpMessage& msg);
    bool ReadFile(Socket& socket, HttpMessage& msg);
    BlockingQueue<HttpMessage>& inQ_;
    bool terminate_;
    std::string& sDir;
  };

  class Receiver {
  public:
    Receiver();
    Receiver(const std::string& address, size_t port);
    Receiver(const Receiver&) = delete;
    Receiver& operator=(const Receiver&) = delete;
    Receiver(Receiver&&);
    Receiver& operator=(Receiver&&);
    ~Receiver();

    bool Initialize(const std::string& address, size_t port);
    HttpMessage Receive();
    bool Good();

    const std::string& save_directory() const;
    void save_directory(const std::string& sDir);

    const size_t& port() const { return port_; }

  private:
    SocketSystem socketSystem_;
    SocketListener* socketListener_;
    ClientHandler clientHandler_;
    BlockingQueue<HttpMessage> inQ_;
    std::string sDir;
    size_t port_;
  };

}