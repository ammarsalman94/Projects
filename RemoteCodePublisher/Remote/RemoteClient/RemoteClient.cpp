///////////////////////////////////////////////////////////////////////////////////
// RemoteClient.cpp  : Defines RemoteClient class that sends/receives from/to    //
//                     RemotePublisher server                                    //
// ver 1.0                                                                       //
//                                                                               //
// Platform    : Dell Inspiron 5520. Windows 10 Professional. Visual Studio 15   //
// Application : CSE-687 OOD Project 4                                           //
// Author      : Ammar Salman, EECS Department, Syracuse University              //
//               (313)-788-4694, hoplite.90@hotmail.com                          //
///////////////////////////////////////////////////////////////////////////////////
/*
*  Package Description: this package defines RemoteClient class that is responsible
*  for contacting the RemotePublisher. It is worth to note that RemoteClients can 
*  send/receive files between each other. Other than files, they do not understand
*  the messages. For example, RemoteClient sends "GetCategories" message and that 
*  is well understood on RemotePublisher, but if that message was sent to another
*  RemoteClient instance, it does not understand it as RemoteClients are not supposed
*  to deal with such messages. Files are the only ones they understand from each 
*  other because they are supposed to send/receive files from RemotePublisher.
*
*  The need of defining the RemoteClient inside the CPP file, and not the header,
*  comes from the C++/CLI conversion. The C++/CLI Shim cannot compile many native
*  C++ libraries, like <thread>, <mutex>, <condition_variable> and others. Therefore,
*  I have followed the rule of Pointer to Implementation. The Shim includes the file
*  RemoteClient.h which only defines an interface and an object factory that returns
*  pointer to the IRemoteClient interface. So the Shim wouldnt even know that there is
*  a type called RemoteClient, still, it will be able to use it because Object Factory
*  returned an instance of it as IRemoteClient pointer.
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
*  Receiver.h Receiver.cpp
*  Sender.h Sender.cpp
*  Cpp11-BlockingQueue.h
*  HttpMessage.h HttpMessage.cpp
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
#define IN_DLL

#include "RemoteClient.h"

#include "../SocketHelper/Receiver/Receiver.h"
#include "../SocketHelper/Sender/Sender.h"
#include "../../SharedProjects/HttpMessage/HttpMessage.h"


using namespace Communication;

// -----< RemoteClient class, implements IRemoteClient >------------------------
class RemoteClient : public IRemoteClient {
public:
  RemoteClient();
  ~RemoteClient();

  virtual std::string GetMessageToCS();
  
  virtual void PostMessageFromCS(const std::string& message) override;
  virtual bool Listen(size_t port) override;
  virtual void SetWorkingDirectory(const std::string& wDir) override;
  virtual bool SendTestMessage(const std::string& address) override;


private:
  Receiver receiver_;
  Sender sender_;
  void ReceiveThreadProc();
  // this is used so the C# client can get the messages... as strings
  BlockingQueue<std::string> receivedMessages_;

  HttpMessage ConvertStringToHttpMessage(const std::string& msg);

  // the following methods were not used in this project. But they should be defined
  // because the RemoteClient should also be usable in native C++ code and not only
  // throw C# clients.

  HttpMessage PreparePublishRequest(const std::string& endpoint, const std::string& dir
    , const std::string& patterns, const std::string& outDir, bool incsubs = true);
  HttpMessage PrepareCategoriesRequest(const std::string& endpoint);
  HttpMessage PrepareFileListRequest(const std::string& endpoint, const std::string& category);
  HttpMessage PrepareAllFilesRequest(const std::string& endpoint);
  HttpMessage PrepareFileRequest(const std::string& endpoint, const std::string& filename);
  HttpMessage PrepareDeleteFileRequest(const std::string& endpoint, const std::string& filename);
  HttpMessage PrepareSendFileMessage(const std::string& endpoint, const std::string& filename);
};

// -----< RemoteClient Constructor >---------------------------------------------
RemoteClient::RemoteClient() {
  std::thread receiver(&RemoteClient::ReceiveThreadProc, this);
  receiver.detach();
  
}

// -----< RemoteClient Destructor >----------------------------------------------
RemoteClient::~RemoteClient() {
}

// -----< Send received message to CS client on demand >-------------------------
std::string RemoteClient::GetMessageToCS() {
  return receivedMessages_.deQ();
}

// -----< Get message from CS client and put it in Sender's queue >--------------
/* This takes string as it is easier to transfer strings from/to CS client.
 * and this is why I needed a function to convert it form string to HttpMessage*/
void RemoteClient::PostMessageFromCS(const std::string & message) {
  HttpMessage msg = ConvertStringToHttpMessage(message);
  std::cout << "\nSending Message:\n========================\n" << msg.toString() << "\n========================\n";
  sender_.PutMessage(msg);
}

// -----< Listen on given port >-------------------------------------------------
bool RemoteClient::Listen(size_t port) {
  return receiver_.Initialize("localhost", port);
}

// -----< Set Working Directory for both Sender/Receiver >-----------------------
void RemoteClient::SetWorkingDirectory(const std::string & wDir) {
  sender_.working_directory(wDir);
  receiver_.save_directory(wDir);
}

// -----< Send Test Message - blocking call >------------------------------------
/* This function is only called on connection initialization. The C# client has
 * to know whether it is really connected to the server or not and it can do this
 * using this function. This will attempt to send message to the given address
 * three times, if it couldnt send the message it will return false and the C#
 * client will know it cannot establish connection with the server */
bool RemoteClient::SendTestMessage(const std::string& address) {
  return sender_.SendTestMessage(address);
}

// -----< Receiver Thread Processing >-------------------------------------------
void RemoteClient::ReceiveThreadProc() {
  while (true) {
    HttpMessage msg = receiver_.Receive();
    std::string msgType = msg.findValue("Type");
    std::string receivedMessage;
    // for the moment, this client does not need to directly deel with received
    // messages. So it forwards them to the C# client which deals with them on
    // the GUI and make display. Therefore this just converts them to string
    // and puts them in the outgoing queue for the GUI client


    //if (msgType == "PublishResults");
    //if (msgType == "Categories");
    //if (msgType == "FileList");
    //if (msgType == "AllFiles");
    //if (msgType == "File");

    receivedMessages_.enQ(msg.toString());
  }
}

// -----< Convert string to HttpMessage >---------------------------------------
/* Returns HttpMessage out of std::string */
HttpMessage RemoteClient::ConvertStringToHttpMessage(const std::string & msg) {
  std::istringstream in(msg);
  HttpMessage hmsg;
  std::string temp, msgBody="";
  // false to read attributes
  bool bodyturn = false;
  while (std::getline(in, temp, '\n')) {
    // check if reading attributes is over
    if (temp == "") {
      bodyturn = true;
      continue;
    }
    // read the body if its turn has come up
    if (bodyturn) {
      msgBody += temp;  
      continue;
    }
    // read attribute
    std::string attr, value;
    size_t pos = temp.find(':');
    if (pos == std::string::npos) continue;
    attr = temp.substr(0, pos);
    value = temp.substr(pos + 1);
    hmsg.addAttribute(HttpMessage::Attribute(attr, value));
  }
  hmsg.addBody(msgBody);
  return hmsg;
}


// -----< Publish Request Message >-------------------------------------------------
HttpMessage RemoteClient::PreparePublishRequest(const std::string & endpoint, 
  const std::string& dir, const std::string& patterns, 
  const std::string& outDir, bool incsubs) {
  HttpMessage msg;
  std::string msgBody = "Publish Message Body";
  msg.addAttribute(HttpMessage::Attribute("Type", "Publish"));
  msg.addAttribute(HttpMessage::Attribute("ToAddr", endpoint));
  msg.addAttribute(HttpMessage::Attribute("FromAddr", "localhost:"+std::to_string(receiver_.port())));
  msg.addAttribute(HttpMessage::Attribute("Directory", dir));
  msg.addAttribute(HttpMessage::Attribute("Patterns", patterns));
  msg.addAttribute(HttpMessage::Attribute("Sub-Dirs", std::to_string(incsubs)));
  msg.addAttribute(HttpMessage::Attribute("OutDir", outDir));
  msg.addAttribute(HttpMessage::Attribute("content-length", std::to_string(msgBody.size())));
  msg.addBody(msgBody);
  return msg;
}

// -----< Get Categories Request Message >-----------------------------------------
HttpMessage RemoteClient::PrepareCategoriesRequest(const std::string & endpoint) {
  HttpMessage msg;
  std::string msgBody = "Categories List Request Message Body";
  msg.addAttribute(HttpMessage::Attribute("Type", "GetCategories"));
  msg.addAttribute(HttpMessage::Attribute("ToAddr", endpoint));
  msg.addAttribute(HttpMessage::Attribute("FromAddr", "localhost:" + std::to_string(receiver_.port())));
  msg.addAttribute(HttpMessage::Attribute("content-length", std::to_string(msgBody.size())));
  msg.addBody(msgBody);
  return msg;
}

// -----< Get File List based on Category Request Message >-----------------------
HttpMessage RemoteClient::PrepareFileListRequest(const std::string & endpoint, const std::string & category) {
  HttpMessage msg;
  std::string msgBody = "File List Based On Category Request Message";
  msg.addAttribute(HttpMessage::Attribute("Type", "GetFileList"));
  msg.addAttribute(HttpMessage::Attribute("ToAddr", endpoint));
  msg.addAttribute(HttpMessage::Attribute("FromAddr", "localhost:" + std::to_string(receiver_.port())));
  msg.addAttribute(HttpMessage::Attribute("Category", category));
  msg.addAttribute(HttpMessage::Attribute("content-length", std::to_string(msgBody.size())));
  msg.addBody(msgBody);
  return msg;
}

// -----< Get All Files Request Message >------------------------------------------
HttpMessage RemoteClient::PrepareAllFilesRequest(const std::string & endpoint) {
  HttpMessage msg;
  std::string msgBody = "All Files List Request Message";
  msg.addAttribute(HttpMessage::Attribute("Type", "GetAllFiles"));
  msg.addAttribute(HttpMessage::Attribute("ToAddr", endpoint));
  msg.addAttribute(HttpMessage::Attribute("FromAddr", "localhost:" + std::to_string(receiver_.port())));
  msg.addAttribute(HttpMessage::Attribute("content-length", std::to_string(msgBody.size())));
  msg.addBody(msgBody);
  return msg;
}

// -----< Get File Request Message >-----------------------------------------------
HttpMessage RemoteClient::PrepareFileRequest(const std::string & endpoint, const std::string & filename) {
  HttpMessage msg;
  std::string msgBody = "File Request Message";
  msg.addAttribute(HttpMessage::Attribute("Type", "GetFile"));
  msg.addAttribute(HttpMessage::Attribute("ToAddr", endpoint));
  msg.addAttribute(HttpMessage::Attribute("FromAddr", "localhost:" + std::to_string(receiver_.port())));
  msg.addAttribute(HttpMessage::Attribute("Filename", filename));
  msg.addAttribute(HttpMessage::Attribute("content-length", std::to_string(msgBody.size())));
  msg.addBody(msgBody);
  return msg;
}

// -----< Delete File Request Message >--------------------------------------------
HttpMessage RemoteClient::PrepareDeleteFileRequest(const std::string & endpoint, const std::string & filename) {
  HttpMessage msg;
  std::string msgBody = "Delete File Request Message";
  msg.addAttribute(HttpMessage::Attribute("Type", "DeleteFile"));
  msg.addAttribute(HttpMessage::Attribute("ToAddr", endpoint));
  msg.addAttribute(HttpMessage::Attribute("FromAddr", "localhost:" + std::to_string(receiver_.port())));
  msg.addAttribute(HttpMessage::Attribute("Filename", filename));
  msg.addAttribute(HttpMessage::Attribute("content-length", std::to_string(msgBody.size())));
  msg.addBody(msgBody);
  return msg;
}

// -----< Send File Message >------------------------------------------------------
HttpMessage RemoteClient::PrepareSendFileMessage(const std::string & endpoint, const std::string & filename) {
  HttpMessage msg;
  msg.addAttribute(HttpMessage::Attribute("Type", "File"));
  msg.addAttribute(HttpMessage::Attribute("ToAddr", endpoint));
  msg.addAttribute(HttpMessage::Attribute("FromAddr", "localhost:" + std::to_string(receiver_.port())));
  msg.addAttribute(HttpMessage::Attribute("Filename", filename));
  return msg;
}


// ================================================================================
// =====<<< RETURN REMOTECLIENT INSTANCE AS IREMOTECLIENT POINTER >>===============
// ================================================================================
IRemoteClient * ObjectFactory::createClient() {
  return new RemoteClient;
}


#ifdef TEST_REMOTECLIENT
int main() {
  RemoteClient rc1;
  RemoteClient rc2;
  rc1.Listen(9091);
  rc2.Listen(9092);
  rc1.PostMessageFromCS("localhost:9090\n9091");
  rc2.PostMessageFromCS("localhost:9090\n9092");
  //std::cout << rc1.GetMessageToCS();
  //std::cout << rc2.GetMessageToCS();

}

#endif // TEST_REMOTECLIENT
