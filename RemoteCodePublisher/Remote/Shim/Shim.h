#pragma once
///////////////////////////////////////////////////////////////////////////////
// Shim.h  :  Defines C++/CLI wrapper for RemoteClient project               //
// ver 1.0                                                                   //
//                                                                           //
// Platform     : Dell Inspiron 5520, Windows 10 Pro, Visual Studio 2015     //
// Application  : CSE-687 OOD, Project 4                                     //
// Author       : Ammar Salman, EECS Department, Syracuse University         //
//                (313)-788-4694, hoplite.90@hotmail.com                     //
///////////////////////////////////////////////////////////////////////////////
/*
* Package description: this package defines a wrapper for the RemoteClient
* package. This wrapper is built as DLL that is used by .NET application.
* The application is in C# and defines GUI for this client.
*
*/
/*
* Public Interface (calls in C# GUI client):
* Shim shim = new Shim();
* shim.Listen(9091);
* shim.PostMessage(msg);
* shim.GetMessage();
* shim.SetWorkingDirectory("C:\\TEST");
* shim.SendTestMessage("localhost:9090);
*
* Required Files:
* =====================
* Shim.h Shim.cpp RemoteClient.h
*
* Build Proceedure:
* =====================
* devenv Shim.vcxproj /rebuild
*
* Maintainence History:
* =====================
* ver 1.0 04, April, 2017
*   - first release
*/

#include "../RemoteClient/RemoteClient.h"

using namespace System;

// -----< Define Shim Class >--------------------------------------------------
public ref class Shim {
public:
  Shim();
  ~Shim();
  void PostMessage(String^ msg);
  String^ GetMessage();
  bool Listen(size_t port);
  void SetWorkingDirectory(String^ dir);
  bool SendTestMessage(String^ address);

private:
  IRemoteClient* rmtClient;
  String^ stdStrToSysStr(const std::string& str);
  std::string sysStrToStdStr(String^ str);
};
