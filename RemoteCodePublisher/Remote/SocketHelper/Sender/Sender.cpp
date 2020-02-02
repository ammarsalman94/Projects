///////////////////////////////////////////////////////////////////////////////////
// Sender.cpp  : Provides definitions for Sender.h                               //
// ver 1.0                                                                       //
//                                                                               //
// Platform    : Dell Inspiron 5520. Windows 10 Professional. Visual Studio 15   //
// Application : CSE-687 OOD Project 4                                           //
// Author      : Ammar Salman, EECS Department, Syracuse University              //
//               (313)-788-4694, hoplite.90@hotmail.com                          //
///////////////////////////////////////////////////////////////////////////////////

#include "Sender.h"
#include "../../../SharedProjects/FileSystem/FileSystem.h"


using namespace Communication;

// -----< Sender Constructor >-----------------------------------------------------
Sender::Sender() : working_(true), send_trials_(5) {
  sThread = std::thread(&Sender::ThreadProc_, this);
}


// -----< Sender Destructor >------------------------------------------------------
Sender::~Sender() {
  working_ = false;
  for (auto it = endpoints_.begin(); it != endpoints_.end(); ++it) {
    // send quit message to all the ClientHandlers on the connected servers
    HttpMessage msg;
    msg.addAttribute(HttpMessage::Attribute("Type", "Quit"));
    msg.addAttribute(HttpMessage::Attribute("ToAddr", it->first));
    outQ_.enQ(msg);
  }
  // tell sender's thread to quit
  HttpMessage msg;
  msg.addAttribute(HttpMessage::Attribute("Type", "Quit"));
  outQ_.enQ(msg);
  // wait until thread is finished to ensure all messages are sent
  if (sThread.joinable())
    sThread.join();
  // close all SocketConnecters
  for (auto it = endpoints_.begin(); it != endpoints_.end(); ++it)
    it->second.close();
}


// -----< Re-Initialize Sender >----------------------------------------------------
void Sender::ReInitialize() {
  working_ = true;
  sThread = std::thread(&Sender::ThreadProc_, this);
}

// -----< Put HttpMessage in the outgoing messages queue >--------------------------
bool Sender::PutMessage(HttpMessage & msg) {
  if (working_) {
    outQ_.enQ(msg);
    return true;
  }
  return false;
}

// -----< Send file to destination >------------------------------------------------
bool Sender::SendFile(Socket& socket, HttpMessage& msg) {
  // send HttpMessage telling the receiver to receive a file
  std::string fqname = working_directory_ + msg.findValue("Filename");
  FileSystem::FileInfo fi(fqname);
  size_t fileSize = fi.size();
  std::string sizeString = std::to_string(fileSize);
  FileSystem::File file(fqname);
  file.open(FileSystem::File::in, FileSystem::File::binary);
  if (!file.isGood())
    return false;

  msg.addAttribute(HttpMessage::Attribute("content-length", sizeString));
  socket.sendString(msg.toString());

  // send the file on the socket after notifying the receiver to receive it
  const size_t BlockSize = 2048;
  Socket::byte buffer[BlockSize];
  while (true) {
    FileSystem::Block blk = file.getBlock(BlockSize);
    if (blk.size() == 0)
      break;
    for (size_t i = 0; i < blk.size(); ++i)
      buffer[i] = blk[i];
    socket.send(blk.size(), buffer);
    if (!file.isGood())
      break;
  }
  file.close();
  return true;
}

// -----< Wait for sender's thread >------------------------------------------------
/* This function is not being used */
void Sender::Wait() {
  sThread.join();
}

// -----< Send test message confirming if connection is valid >---------------------
bool Communication::Sender::SendTestMessage(const std::string & address) {
  size_t pos = address.find(':');
  if (pos == std::string::npos) return false;

  std::string ip = address.substr(0, pos);
  size_t port = static_cast<size_t>(std::stoi(address.substr(pos + 1)));

  HttpMessage msg;
  msg.addAttribute(HttpMessage::Attribute("Type", "TestConnection"));
  std::string msgBody = "Test Connection Message Body";
  msg.addAttribute(HttpMessage::Attribute("content-length", std::to_string(msgBody.size())));
  msg.addBody(msgBody);

  if (endpoints_.find(address) == endpoints_.end()) {
    endpoints_[address] = SocketConnecter();
    for (int i = 1; i <= 3; i++) {
      if (!endpoints_[address].connect(ip, port)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
      }
      else break;
      if (i == 3) return false; // unable to connect after 3 trials
    }
  }
  return endpoints_[address].sendString(msg.toString()); // send if connected
}

// -----< Sender thread processing >-----------------------------------------------
void Sender::ThreadProc_() {
  while (working_ || outQ_.size() > 0) {
    HttpMessage msg = outQ_.deQ();
    std::string receiver_address = msg.findValue("ToAddr");
    EndPoint toAddr = ParseAddress(receiver_address);
    // if unable to parse receiver's address, treate message as corrupt and ignore
    if (toAddr.first == "" || toAddr.second == 0)
      continue;

    for (int i = 1; i <= send_trials_; ++i) {
      // check if connection was already established for that receiver
      if (endpoints_.find(receiver_address) == endpoints_.end())
        endpoints_[receiver_address] = SocketConnecter();
      if (endpoints_[receiver_address].connect(toAddr.first, toAddr.second)) {
        if (msg.findValue("Type") == "File")
          SendFile(endpoints_[receiver_address], msg);
        else
          endpoints_[receiver_address].sendString(msg.toString());
        break;
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
  }
}

// -----< Parse address >----------------------------------------------------------
EndPoint Sender::ParseAddress(std::string & Address) {
  EndPoint ep;
  ep.first = "";
  ep.second = 0;
  size_t pos = Address.find(':');
  if (pos != std::string::npos) {
    ep.first = Address.substr(0, pos);
    ep.second = static_cast<size_t> (std::stoi(Address.substr(pos + 1)));
  }
  return ep;
}


// ===============================================================================
// =====<< TEST STUB >>===========================================================
// ===============================================================================
#ifdef TEST_SENDER

int main() {
  try {
    Sender sender;
    sender.working_directory(FileSystem::Directory::getCurrentDirectory());
    HttpMessage msg;
    msg.addAttribute(HttpMessage::Attribute("Type", "Text"));
    msg.addAttribute(HttpMessage::Attribute("ToAddr", "localhost:9090"));
    msg.addAttribute(HttpMessage::Attribute("FromAddr", "localhost:9091"));
    msg.addAttribute(HttpMessage::Attribute("content-length", "14"));
    msg.addBody("Hi from client");
    for (int i = 0; i < 10; ++i) {
      std::cout << "\n  Sender sending message #" << i;
      sender.PutMessage(msg);
    }
    std::string filename = "Sender.h";
    HttpMessage filemsg;
    filemsg.addAttribute(HttpMessage::Attribute("Type", "File"));
    filemsg.addAttribute(HttpMessage::Attribute("ToAddr", "localhost:9090"));
    filemsg.addAttribute(HttpMessage::Attribute("FromAddr", "localhost:9091"));
    filemsg.addAttribute(HttpMessage::Attribute("Filename", filename));
    sender.PutMessage(filemsg);
  }
  catch (std::exception& ex) {
    std::cout << ex.what();
  }
}

#endif