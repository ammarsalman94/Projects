///////////////////////////////////////////////////////////////////////////////////
// Receiver.cpp  : Provides defomotopms for Receiver.h                           //
// ver 1.0                                                                       //
//                                                                               //
// Platform    : Dell Inspiron 5520. Windows 10 Professional. Visual Studio 15   //
// Application : CSE-687 OOD Project 4                                           //
// Author      : Ammar Salman, EECS Department, Syracuse University              //
//               (313)-788-4694, hoplite.90@hotmail.com                          //
///////////////////////////////////////////////////////////////////////////////////

#include "Receiver.h"
#include "../../../SharedProjects/FileSystem/FileSystem.h"

using namespace Communication;

void createDirectory(const std::string& dPath) {
  size_t pos = dPath.find('\\');
  while (pos != std::string::npos) {
    FileSystem::Directory::create(dPath.substr(0, pos));
    pos = dPath.find('\\', pos + 1);
  }
  if (dPath[dPath.size() - 1] != '\\')
    FileSystem::Directory::create(dPath);
}

// -----< Receiver Constructor >---------------------------------------------------
Receiver::Receiver(): clientHandler_(inQ_,sDir) {
}

// -----< Receiver Constructor using address >-------------------------------------
Receiver::Receiver(const std::string & address, size_t port) : clientHandler_(inQ_, sDir) {
  Initialize(address, port);
  port_ = port;
}

// -----< Move Constructor >-------------------------------------------------------
Receiver::Receiver(Receiver && rcvr) : clientHandler_(inQ_, sDir) {
  socketListener_ = rcvr.socketListener_;
}

// -----< Move Assignment Operator >-----------------------------------------------
Receiver & Receiver::operator=(Receiver && rcvr) {
  socketListener_ = rcvr.socketListener_;
  return *this;
}

// -----< Receiver Destructor >----------------------------------------------------
Receiver::~Receiver() {
  clientHandler_.Terminate();
  //HttpMessage msg;
  //msg.addAttribute(HttpMessage::Attribute("Type", "Quit"));
  //inQ_.enQ(msg);
  delete socketListener_;

}

// -----< Initialization, start listening on given port >--------------------------
bool Receiver::Initialize(const std::string & address, size_t port) {
  port_ = port;
  if (socketListener_ != nullptr) {
   // socketListener_->stop();
   // socketListener_->close();
    delete socketListener_;
    socketListener_ = nullptr;
  }
  socketListener_ = new SocketListener(port, Socket::IP4);
  return socketListener_->start(clientHandler_);
}

// -----< Return message from the received messages >------------------------------
/* This is used by anyone creating an instance of Receiver. */
HttpMessage Receiver::Receive() {
  return inQ_.deQ();
}

// -----< Get State of the Socket >------------------------------------------------
bool Receiver::Good() {
  return socketListener_->validState();
}

// -----< Get Working Directory >--------------------------------------------------
const std::string & Communication::Receiver::save_directory() const {
  return sDir;
}

// -----< Set Working Directory >--------------------------------------------------
void Communication::Receiver::save_directory(const std::string & sDir) {
  this->sDir = sDir;
}



// ================================================================================
// =====<< CLIENT HANDLER DEFINITIONS >>===========================================
// ================================================================================

// -----< ClientHandler Constructor >----------------------------------------------
ClientHandler::ClientHandler(BlockingQueue<HttpMessage>& inQ, std::string& sDir) : inQ_(inQ) , sDir(sDir){

}

// -----< Operator taking Socket >-------------------------------------------------
/* This is used by SocketListener whenever a new connection is accepted. */
void ClientHandler::operator()(Socket socket) {
  while (!terminate_ && socket.validState()) {
    HttpMessage msg = ReadMessage(socket);
    if (msg.attributes().size() == 0)
      return;
    if (msg.findValue("Type") == "Quit") {
      return;
    }
    inQ_.enQ(msg);
  }
}

// -----< Break out of the loop >---------------------------------------------------
/* This should only be used when the Receiver is terminating */
void ClientHandler::Terminate() {
  terminate_ = true;
}

// -----< Read Message Function >---------------------------------------------------
/* Converts the read bytes into an HttpMessage */
HttpMessage ClientHandler::ReadMessage(Socket & socket) {
  HttpMessage msg;
  // read the attributes of the message. If no attributes found, throw it away
  if (!ReadAttributes(socket, msg))
    return msg;
  // if message type is File, then there was a file sent after the message
  // therefore, start receiving the file based on the message information
  if (msg.findValue("Type") == "File") {
    ReadFile(socket, msg);
    return msg;
  }

  // read the body of the message
  size_t numBytes = 0;
  size_t pos = msg.findAttribute("content-length");
  if (pos < msg.attributes().size()) {
    numBytes = static_cast<size_t>(std::stol(msg.attributes()[pos].second));
    Socket::byte* buffer = new Socket::byte[numBytes + 1];
    socket.recv(numBytes, buffer);
    buffer[numBytes] = '\0';
    std::string msgBody(buffer);
    msg.addBody(msgBody);
    delete[] buffer;
  }
  return msg;
}

// -----< Read HttpMessage Attributes from std::stirng >----------------------------
bool ClientHandler::ReadAttributes(Socket & socket, HttpMessage & msg) {
  while (true) {
    std::string attribString = socket.recvString('\n');
    if (attribString.size() > 1) {
      HttpMessage::Attribute attrib = HttpMessage::parseAttribute(attribString);
      msg.addAttribute(attrib);
    }
    else {
      break;
    }
  }
  return msg.attributes().size() > 0;
}

// ----< Read File from Socket given its info from HttpMessage >--------------------
/* files are saved based on the current working directory
* This is important to make it possible that senders can
* send multiple files and keep their structure the same.
* For example: C:\Test\cpps\file1.cpp
*              C:\Test\headers\file1.h
* And the working directory of the sender is C:\Test
* And the working directory of receiver is C:\TEST2
* The receiver will receive them like:
*              C:\Test2\cpps\file1.cpp
*              C:\Test2\headers\file1.h
*/
bool ClientHandler::ReadFile(Socket & socket, HttpMessage& msg) {
  // if there is no Filename attribute then message is invalid. return false
  std::string filename = msg.findValue("Filename");
  if (filename == "") 
    return false;

  // get file size based on "content-length" attribute
  // if there is no content-length then message is invalid, return false
  size_t filesize;
  std::string sizeString = msg.findValue("content-length");
  if (sizeString != "")
    filesize = static_cast<size_t>(stol(sizeString));
  else
    return false;
  std::string outFilename = sDir + filename;
  std::string copy = outFilename;

  std::string path = FileSystem::Path::getPath(outFilename);
  createDirectory(path);

  FileSystem::File inFile(outFilename);
  inFile.open(FileSystem::File::out, FileSystem::File::binary);

  
  bool fileIsGood = inFile.isGood();

  const size_t BlockSize = 2048;
  Socket::byte buffer[BlockSize];

  size_t bytesToRead;
  while (true) {
    if (filesize > BlockSize) bytesToRead = BlockSize;
    else bytesToRead = filesize;

    socket.recv(bytesToRead, buffer);

    // even if the input file is not valid, the buffer should be
    // emptied from the file content that was sent to it.
    if (fileIsGood) {
      FileSystem::Block blk;
      for (size_t i = 0; i < bytesToRead; ++i)
        blk.push_back(buffer[i]);
      inFile.putBlock(blk);
    }
    if (filesize < BlockSize) break;
    filesize -= BlockSize;
  }
  inFile.close();
  return fileIsGood;;
}


#ifdef TEST_RECEIVER

int main() {
  try {
    SocketSystem ss;
    Receiver rcvr("localhost", 9090);
    rcvr.save_directory(FileSystem::Directory::getCurrentDirectory());
    std::thread r([&] {
      while (rcvr.Good()) {
        HttpMessage msg = rcvr.Receive();
        std::cout << "\n  Receiver received:\n  " << msg.toString();
      }
    });
    r.join();
  }
  catch (std::exception& ex) {
    std::cout << ex.what();
  }
  return 0;

}

#endif