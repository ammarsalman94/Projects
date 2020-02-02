#pragma once
///////////////////////////////////////////////////////////////////////////////////
// Sender.h  : Uses multiple sockets to send messages to multiple destinations   //
// ver 1.0                                                                       //
//                                                                               //
// Platform    : Dell Inspiron 5520. Windows 10 Professional. Visual Studio 15   //
// Application : CSE-687 OOD Project 4                                           //
// Author      : Ammar Salman, EECS Department, Syracuse University              //
//               (313)-788-4694, hoplite.90@hotmail.com                          //
///////////////////////////////////////////////////////////////////////////////////
/*
*  Package Description: this package uses Sockets.h to create multiple instances
*  of SocketConnecter class. It stores them in a map which allows for better
*  performance if there were multiple destinations. 
*  It has a blocking call (SendTestMessage) which returns a boolean indicating
*  whether it is possible to connect to the given address or not.
*  This is useful especially for the GUI client.
*
*  Public Interface:
* =================================
*  Sender sender;
*  sender.ReInitialize();
*  sender.PutMessage(msg);
*  if(sender.working()) ...;
*  sender.send_trials(5);
*  sender.working_directory("C:\\test");
*  sender.Wait();
*  sender.SendTestMessage("localhost:9090");
*/
/*
*  Required Files:
* =================================
*  Sender.h Sender.cpp 
*  Cpp11-BlockingQueue.h 
*  Sockets.h Sockets.cpp
*  HttpMessage.h HttpMessage.cpp
*  FileSystem.h FileSystem.cpp
*
*  Build Instructions: 
* =================================
*  devenv Sender.vcxproj /rebuild
*
*  Maintainence History:
* =================================
*  ver 1.0 - 29, April, 2017
*    - first release
*/


#include "../../../SharedProjects/Cpp11-BlockingQueue/Cpp11-BlockingQueue.h"
#include "../../../SharedProjects/HttpMessage/HttpMessage.h"
#include "../Sockets/Sockets.h"

#include <unordered_map>

namespace Communication {
  using EndPoint = std::pair<std::string, size_t>;

  class Sender {
  public:
    Sender();
    ~Sender();

    void ReInitialize();
    bool PutMessage(HttpMessage& msg);
    const bool& working() const { return working_; }
    const size_t& send_trials() const { return send_trials_; }
    void send_trials(const size_t& st) { send_trials_ = st; }

    const std::string& working_directory() const { return working_directory_; }
    void working_directory(const std::string& wdir) { working_directory_ = wdir; }

    void Wait();
    bool SendTestMessage(const std::string& address);

  private:
    BlockingQueue<HttpMessage> outQ_;
    std::unordered_map<std::string, SocketConnecter> endpoints_;
    SocketSystem socketSystem_;
    bool working_;
    size_t send_trials_;
    std::thread sThread;
    void ThreadProc_();
    std::string working_directory_;

    EndPoint ParseAddress(std::string& Address);
    bool SendFile(Socket& socket, HttpMessage& msg);
  };

}