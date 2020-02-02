#pragma once
///////////////////////////////////////////////////////////////////////////////////
// RemotePublisher.h  : Defines RemotePublisher class that receives requests     //
//                      from RemoteClients and acts accordingly                  //
// ver 1.0                                                                       //
//                                                                               //
// Platform    : Dell Inspiron 5520. Windows 10 Professional. Visual Studio 15   //
// Application : CSE-687 OOD Project 4                                           //
// Author      : Ammar Salman, EECS Department, Syracuse University              //
//               (313)-788-4694, hoplite.90@hotmail.com                          //
///////////////////////////////////////////////////////////////////////////////////
/*
*  Package Description: this package defines RemotePublisher class. This class 
*  is essentially the server-side in this software. It accepts connections from
*  different clients and acts accordingly. If clients wanted to send files or 
*  download files, they can do it. If the clients wanted to publish source files
*  to webpages they can also do it. In short, it is kind of a repository.
*
*  Public Interface:
* =================================
*  RemotePublisher rmtPublisher;
*  rmtPublisher.Listen("localhost", 9090);
*  rmtPublisher.working_directory("C:\\test");
*  rmtPublisher.Wait(); // blocking call to disable destruction of it
*/
/*
*  Required Files:
* =================================
*  Receiver.h Receiver.cpp
*  Sender.h Sender.cpp
*  HttpMessage.h HttpMessage.cpp
*  FileSystem.h FileSystem.cpp
*
*  Build Instructions:
* =================================
*  devenv RemotePublisher.vcxproj /rebuild
*
*  Maintainence History:
* =================================
*  ver 1.0 - 29, April, 2017
*    - first release
*/

#include "../SocketHelper/Receiver/Receiver.h"
#include "../SocketHelper/Sender/Sender.h"
#include "../../SharedProjects/HttpMessage/HttpMessage.h"
#include "../../CodeAnalysis/Publisher/Publisher.h"
#include "../../SharedProjects/FileSystem/FileSystem.h"

#include <unordered_map>
#include <set>

using namespace Communication;
using namespace CodePublisher;

// -----< Remote Publisher Class Declarations >----------------------------------
class RemotePublisher {
public:
  RemotePublisher();
  ~RemotePublisher();

  bool ProcessCommandLineArgs(int argc, char** argv);
  void ShowUsage(const std::string& message = "");
  bool Listen(size_t port);

  const std::string& working_directory() const;
  bool working_directory(const std::string& wdir);
  const std::string& iis_dir() const;
  bool iis_dir(const std::string& iis);
  void Wait();

private:
  Receiver receiver_;
  Sender sender_;
  std::unordered_map<std::string, std::vector<std::string>> categories_;
  std::vector<std::string> directories_;
  std::set<std::string> patterns_;
  std::string working_directory_;
  std::string iis_dir_;
  size_t iis_port_;
  std::vector<std::string> iis_webpages_;
  std::thread receiver_thread;


  void ScanRoot(const std::string& dir);
  void ScanWebpages();

  void receiveThreadProc();

  void HandleMessage(HttpMessage& msg);
  void Publish(HttpMessage& msg, HttpMessage& outMsg);
  void ManageGetCategories(HttpMessage& msg, HttpMessage& outMsg);
  void ManageGetDirectories(HttpMessage& msg, HttpMessage& outMsg);
  void ManageGetFileList(HttpMessage& msg, HttpMessage& outMsg);
  void ManageGetAllFiles(HttpMessage& msg, HttpMessage& outMsg);
  void ManageGetFile(HttpMessage& msg, HttpMessage& outMsg);
  void ManageFileMessage(HttpMessage& msg);
  void ManageDeleteFileMessage(HttpMessage& msg, HttpMessage& outMsg);
  void ManageGetPublished(HttpMessage& msg, HttpMessage& outMsg);
};

