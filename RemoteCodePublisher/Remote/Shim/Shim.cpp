///////////////////////////////////////////////////////////////////////////////
// Shim.cpp  :  Provides C++/CLI definitions for Shim.h                      //
// ver 1.0                                                                   //
//                                                                           //
// Platform     : Dell Inspiron 5520, Windows 10 Pro, Visual Studio 2015     //
// Application  : CSE-687 OOD, Project 4                                     //
// Author       : Ammar Salman, EECS Department, Syracuse University         //
//                (313)-788-4694, hoplite.90@hotmail.com                     //
///////////////////////////////////////////////////////////////////////////////

#include "Shim.h"
#include <iostream>

// -----< Shim Constructor >---------------------------------------------
Shim::Shim() {
  // craete RemoteClient instance as IRemoteClient pointer
  ObjectFactory factory;
  rmtClient = factory.createClient();
}

// -----< Shim Destructor >----------------------------------------------
Shim::~Shim() {
  delete rmtClient;
}

// -----< Put message for the RemoteClient to send >---------------------
void Shim::PostMessage(String^ msg) {
  rmtClient->PostMessageFromCS(sysStrToStdStr(msg));
}

// -----< Get message received by RemoteClient >-------------------------
String ^ Shim::GetMessage() {
  return stdStrToSysStr(rmtClient->GetMessageToCS());
}

// -----< Set RemoteClient to listen on the given port >-----------------
bool Shim::Listen(size_t port) {
  return rmtClient->Listen(port);
}

// -----< Set RemoteClient's directory to given directory >--------------
void Shim::SetWorkingDirectory(String^ wDir) {
  rmtClient->SetWorkingDirectory(sysStrToStdStr(wDir));
}

// -----< Send connection test message to a given address >--------------
bool Shim::SendTestMessage(String^ address) {
  return rmtClient->SendTestMessage(sysStrToStdStr(address));
}

//----< convert std::string to System.String >---------------------------
String^ Shim::stdStrToSysStr(const std::string& str) {
  return gcnew String(str.c_str());
}

//----< convert System.String to std::string >---------------------------
std::string Shim::sysStrToStdStr(String^ str) {
  std::string temp;
  for (int i = 0; i < str->Length; ++i)
    temp += char(str[i]);
  return temp;
}

// ---------------------------------------------------------------------
// TEST STUB FOR SHIM
// ---------------------------------------------------------------------

#ifdef TEST_SHIM
int WinMain() {
  Shim^ shim = gcnew Shim();
  shim->Listen(10000);
  //shim->PostMessage("blablal\n\n\nasndal\n\n");
  return 0;
}
#endif // TEST_SHIM

