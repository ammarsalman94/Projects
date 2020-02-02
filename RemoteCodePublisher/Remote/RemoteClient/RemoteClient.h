#pragma once
///////////////////////////////////////////////////////////////////////////////////
// RemoteClient.h  : Defines interface that exposes RemoteClient to outside      //
// ver 1.0                                                                       //
//                                                                               //
// Platform    : Dell Inspiron 5520. Windows 10 Professional. Visual Studio 15   //
// Application : CSE-687 OOD Project 4                                           //
// Author      : Ammar Salman, EECS Department, Syracuse University              //
//               (313)-788-4694, hoplite.90@hotmail.com                          //
///////////////////////////////////////////////////////////////////////////////////
/*
*  Package Description: this package only defines the interface of which the 
*  RemoteClient will implement. The need for the interface is to make it possible
*  that C++/CLI can use this package and expose it to the C# GUI client.
*
*  Public Interface:
* =================================
*  ObjectFactory factory;
*  IRemoteClient *rmtClient = factory.createClient();
*  rmtClient->GetMessageToCS(); // pulls receieved message for the CS client
*  rmtClient->PostMessageFromCS(msg); // receives message from CS client
*  rmtClient->Listen(9091); // starts listening on specified port
*  rmtClient->SendTestMessage("localhost:9090"); // blocking call to make sure 
*                                                // connection can be established
*/
/*
*  Required Files:
* =================================
*  RemoteClient.h RemoteClient.cpp
*
*  Build Instructions:
* =================================
*  devenv RemoteClient.vcxproj /rebuild
*
*  Maintainence History:
* =================================
*  ver 1.0 - 29, April, 2017
*    - first release
*/

#ifdef IN_DLL
#define DLL_DECL __declspec(dllexport)
#else
#define DLL_DECL __declspec(dllimport)
#endif

#include <string>

// -----< IRemoteClient >--------------------------------------------------
/* Defines the interface with pure virtual methods that the Shim will use*/
class IRemoteClient {
public:
  virtual std::string GetMessageToCS() = 0;
  virtual void PostMessageFromCS(const std::string& message) = 0;
  virtual bool Listen(size_t port) = 0;
  virtual void SetWorkingDirectory(const std::string& wDir) = 0;
  virtual bool SendTestMessage(const std::string& address) = 0;
};


// -----< Defines DLL function that returns RemoteClient >----------------
/* It returns RemoteClient as IRemoteClient pointer.
*  The need for extern "C" is essentially making a promiss to the 
*  compiler that this function wont be overloaded and can be referenced
*  to by its exact name. No mangled names issues. */
extern "C" {
  struct ObjectFactory {
    DLL_DECL IRemoteClient* createClient();
  };
} 