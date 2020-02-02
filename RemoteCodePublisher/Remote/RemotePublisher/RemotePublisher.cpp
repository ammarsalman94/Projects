///////////////////////////////////////////////////////////////////////////////////
// RemotePublisher.cpp  : Provides definitions for RemotePublisher.h             //
// ver 1.0                                                                       //
//                                                                               //
// Platform    : Dell Inspiron 5520. Windows 10 Professional. Visual Studio 15   //
// Application : CSE-687 OOD Project 4                                           //
// Author      : Ammar Salman, EECS Department, Syracuse University              //
//               (313)-788-4694, hoplite.90@hotmail.com                          //
///////////////////////////////////////////////////////////////////////////////////

#include "RemotePublisher.h"

// -----< RemotePublisher Constructor >--------------------------------------------
RemotePublisher::RemotePublisher() {
  // i made the categories based on the file type
  categories_["Webpage"] = std::vector<std::string>();
  categories_["CppHeader"] = std::vector<std::string>();
  categories_["CppSource"] = std::vector<std::string>();
  categories_["CSharp"] = std::vector<std::string>();
  patterns_.insert("*.h");
  patterns_.insert("*.cpp");
  patterns_.insert("*.cs");
  patterns_.insert("*.htm");
  patterns_.insert("*.html");
  // intitialize receiving thread
  receiver_thread = std::thread(&RemotePublisher::receiveThreadProc, this);
}

// -----< RemotePublisher Destructor >---------------------------------------------
RemotePublisher::~RemotePublisher() {
  if (receiver_thread.joinable())
    receiver_thread.detach();
}

// -----< Check validity of command line args and initialize based on them >-------
bool RemotePublisher::ProcessCommandLineArgs(int argc, char ** argv) {
  if (argc < 9) {
    ShowUsage("Too few arguments.");
    return false;
  }
  size_t port = 0;
  size_t port_iis = 0;
  std::string iis_dir = "";
  std::string wdir = "";
  try {
    for (int i = 1; i < argc; ++i) {
      if (std::string(argv[i]) == "--wdir") {
        wdir = std::string(argv[i + 1]);
        ++i;
      }
      if (std::string(argv[i]) == "--iis") {
        iis_dir = std::string(argv[i + 1]);
        ++i;
      }
      if (std::string(argv[i]) == "--port") {
        port = static_cast<size_t>(std::stoi(argv[i + 1]));
        ++i;
      }
      if (std::string(argv[i]) == "--port_iis") {
        port_iis = static_cast<size_t>(std::stoi(argv[i + 1]));
        ++i;
      }
    }
    if (port == 0 || port_iis == 0|| iis_dir == "" || wdir == "") {
      ShowUsage();
      return false;
    }
    std::cout << "\n  Starting server on port : " << port;
    if (!Listen(port)) {
      ShowUsage("Cannot instantiate the server on port " + std::to_string((int)port));
      return false;
    }
    std::cout << "\n  Setting working directory to \"" << wdir << "\"";
    if (!working_directory(wdir)) {
      ShowUsage("Working directory \"" + wdir + "\" does not exist.");
      return false;
    }
    std::cout << "\n  Setting IIS directory to \"" << iis_dir << "\"";
    if (!this->iis_dir(iis_dir)) {
      ShowUsage("IIS directory \"" + iis_dir + "\" does not exist.");
      return false;
    }
    iis_port_ = port_iis;
    return true;
  }
  catch (std::exception &ex) {
    ShowUsage(ex.what());
    return false;
  }
}

// -----< Show Command Line Args Usage >-------------------------------------------
void RemotePublisher::ShowUsage(const std::string& message) {
  std::cout << "\n\n  Usage:\n\n  {path to remote publisher} --port PORT_NO --wdir WORKING_DIRECTORY --iis IIS_DIRECTORY";
  std::cout << "\n  --port : specifies the port which Remote Publisher will listen to.";
  std::cout << "\n  --wdir : specifies the working directory of Remote Publisher";
  std::cout << "\n  --iis  : specifies Internet Information Services (IIS) Website physical location";
  std::cout << "\n\n  Notes:";
  std::cout << "\n    1. The parameters above must be specified. Otherwise, this message will show.";
  std::cout << "\n    2. If the specified port number is in use the server will not start.";
  std::cout << "\n    3. If the specified working directory does not exist, the server will not start.";
  std::cout << "\n    4. If the specified IIS directory does not exist, the server will not start.";
  if (message != "")
    std::cout << "\n\n  Error message: " << message;
}

// -----< Listen on Port >---------------------------------------------------------
bool RemotePublisher::Listen(size_t port) {
  return receiver_.Initialize("localhost", port);
}

// -----< Get Working Directory >--------------------------------------------------
const std::string & RemotePublisher::working_directory() const {
  return working_directory_;
}

// -----< Set Working Directory >--------------------------------------------------
bool RemotePublisher::working_directory(const std::string & wdir) {
  if (FileSystem::Directory::exists(wdir)) {
    working_directory_ = FileSystem::Path::getFullFileSpec(wdir);
    sender_.working_directory(working_directory_);
    receiver_.save_directory(working_directory_);
    // rescan the repository
    for (auto it = categories_.begin(); it != categories_.end(); ++it)
      categories_[it->first].clear();
    directories_.clear();
    ScanRoot(working_directory_);
    for (size_t i = 0; i < directories_.size(); ++i)
      directories_[i] = directories_[i].substr(working_directory_.size());
    for (auto it = categories_.begin(); it != categories_.end(); ++it)
      for (size_t i = 0; i < categories_[it->first].size(); ++i)
        categories_[it->first][i] = categories_[it->first][i].substr(working_directory_.size());
    return true;
  }
  return false;
}

// -----< get iis directory >------------------------------------------------------
const std::string & RemotePublisher::iis_dir() const {
  return iis_dir_;
}

// -----< set iis directory >------------------------------------------------------
bool RemotePublisher::iis_dir(const std::string & iis) {
  if (FileSystem::Directory::exists(iis)) {
    iis_dir_ = FileSystem::Path::getFullFileSpec(iis);
    ScanWebpages();
    return true;
  }
  return false;
}

// -----< Wait - Blocking Call >---------------------------------------------------
void RemotePublisher::Wait() {
  if (receiver_thread.joinable())
    receiver_thread.join();
}

// -----< Sacn Working Directory for files >---------------------------------------
void RemotePublisher::ScanRoot(const std::string& dir) {
  for (std::string pattern : patterns_) {
    std::vector<std::string> files = FileSystem::Directory::getFiles(dir, pattern);
    std::string key = "";
    if (pattern == "*.h")
      key = "CppHeader";
    else if (pattern == "*.cpp")
      key = "CppSource";
    else if (pattern == "*.htm" || pattern == "*.html")
      key = "Webpage";
    else if (pattern == "*.cs")
      key = "CSharp";
    for (size_t i = 0; i < files.size(); ++i)
      files[i] = dir + '\\' + files[i];
    categories_[key].insert(categories_[key].end(), files.begin(), files.end());
  }
  std::vector<std::string> directories = FileSystem::Directory::getDirectories(dir);
  directories.erase(directories.begin());
  directories.erase(directories.begin());
  for (size_t i = 0; i < directories.size(); ++i)
    directories[i] = dir + '\\' + directories[i];
  directories_.insert(directories_.begin(), directories.begin(), directories.end());
  for (size_t i = 0; i < directories.size(); ++i)
    ScanRoot(directories[i]);
}

// -----< Scan IIS directory for published webpages >------------------------------
void RemotePublisher::ScanWebpages() {
  iis_webpages_ = FileSystem::Directory::getFiles(iis_dir_, "*.html");
}

// -----< Message Handler Thread >-------------------------------------------------
void RemotePublisher::receiveThreadProc() {
  while (true) {
    HttpMessage msg = receiver_.Receive();
    HandleMessage(msg);
  }
}

// -----< Handle Messages based on their Types >-----------------------------------
void RemotePublisher::HandleMessage(HttpMessage & msg) {
  std::string msgType = msg.findValue("Type");
  std::cout << "\n" << msg.toString() << "\n";
  HttpMessage outMsg;
  outMsg.addAttribute(HttpMessage::Attribute("ToAddr", msg.findValue("FromAddr")));
  outMsg.addAttribute(HttpMessage::Attribute("FromAddr", msg.findValue("ToAddr")));

  if (msgType == "Publish") { Publish(msg, outMsg); return; }
  if (msgType == "GetCategories") ManageGetCategories(msg, outMsg);
  if (msgType == "GetDirectories") ManageGetDirectories(msg, outMsg);
  if (msgType == "GetFileList") ManageGetFileList(msg, outMsg);
  if (msgType == "GetAllFiles") ManageGetAllFiles(msg, outMsg);
  if (msgType == "GetFile") ManageGetFile(msg, outMsg);
  if (msgType == "GetPublishedWebpages") ManageGetPublished(msg, outMsg);
    
  if (msgType == "File") { ManageFileMessage(msg); return; }
  if (msgType == "DeleteFile") ManageDeleteFileMessage(msg, outMsg);
  
  sender_.PutMessage(outMsg);
}

// -----< Handle Publish Message >-------------------------------------------------
/* Publish webpages given their directory on the server.
 * This starts a thread so it does not block the main thread while performing
 * the dependency analysis as it takes a lot of time.
 * It checks whether or not the option to publish files to IIS directory is
 * enabled and if it is, the thread will copy the published files into the 
 * IIS server's directory.
 * It also checks if Automatic Download attribute was set, if yes, it will
 * send the published files to the client who made the request.*/
void RemotePublisher::Publish(HttpMessage & msg, HttpMessage& outMsg) {
  std::thread t([&] {
    Publisher publisher_;
    int argc = 6; char** argv = new char*;
    argv[0] = "publisher.exe"; std::string dir = working_directory_ + msg.findValue("Directory");
    argv[1] = &dir[0u]; argv[2] = "-output";
    std::string out = working_directory_ + "\\Published";
    argv[3] = &out[0u]; argv[4] = "*.h"; argv[5] = "*.cpp";
    for (int i = 0; i < argc; ++i) cout << argv[i] << "\n";
    publisher_.ProcessCommandLineArgs(argc, argv);
    publisher_.PerformDependencyAnalysis();
    publisher_.ProcessFiles();
    publisher_.CreateMainPage();
    publisher_.CopyStylesAndScripts(out + "\\resources", iis_dir_ + "\\resources");
    std::vector<std::string> published_files = publisher_.PublishedFiles();
    if (msg.findValue("AutoDown") == "True") {
      for (size_t i = 0; i < published_files.size(); ++i) {
        published_files[i] = published_files[i].substr(working_directory_.size());
        HttpMessage fileMsg = outMsg;
        fileMsg.addAttribute(HttpMessage::Attribute("Type", "File"));
        fileMsg.addAttribute(HttpMessage::Attribute("Filename", published_files[i]));
        sender_.PutMessage(fileMsg);
      }
    }
    if (msg.findValue("IIS") == "True") {
      std::string msgBody = "";
      std::ifstream in;
      std::ofstream out;
      for (size_t i = 0; i < published_files.size(); ++i) {
        in.open(published_files[i], std::ios::binary);
        if (!in.good()) { in.close(); continue; }
        std::string iis_filename = iis_dir_ + "\\" + FileSystem::Path::getName(published_files[i]);
        msgBody += iis_filename + "\n";
        out.open(iis_filename);
        out << in.rdbuf();
        in.close();
        out.close();
      }
      outMsg.addAttribute(HttpMessage::Attribute("Type", "PublishedWebpages"));
      outMsg.addAttribute(HttpMessage::Attribute("Port", std::to_string(iis_port_)));
      outMsg.addAttribute(HttpMessage::Attribute("content-length", std::to_string(msgBody.size())));
      outMsg.addBody(msgBody);
      sender_.PutMessage(outMsg);
    }
    //delete[] argv;
  });
  t.detach();
}

// -----< Handle Get Categories List Request >-------------------------------------
void RemotePublisher::ManageGetCategories(HttpMessage & msg, HttpMessage& outMsg) {
  std::string msgBody = "";
  for (auto it = categories_.begin(); it != categories_.end(); ++it)
    msgBody += it->first + "\n";

  outMsg.addAttribute(HttpMessage::Attribute("Type", "Categories"));
  outMsg.addAttribute(HttpMessage::Attribute("content-length", std::to_string(msgBody.size())));
  outMsg.addBody(msgBody);
}

// -----< Handle get directories list request >------------------------------------
void RemotePublisher::ManageGetDirectories(HttpMessage & msg, HttpMessage & outMsg) {
  outMsg.addAttribute(HttpMessage::Attribute("Type", "Directories"));
  std::string msgBody = "\\\n";
  for (std::string dir : directories_)
    msgBody += dir + "\n";
  outMsg.addAttribute(HttpMessage::Attribute("content-length", std::to_string(msgBody.size())));
  outMsg.addBody(msgBody);
}

// -----< Handle Get File List based on Category Request >-------------------------
void RemotePublisher::ManageGetFileList(HttpMessage & msg, HttpMessage& outMsg) {
  std::string cat = msg.findValue("Category");
  if (cat == "") return;
  std::vector<std::string>& catVector = categories_[cat];
  std::string msgBody = "";
  for (std::string file : categories_[cat])
    msgBody += file + "\n";

  outMsg.addAttribute(HttpMessage::Attribute("Type", "FileList"));
  outMsg.addAttribute(HttpMessage::Attribute("Category", cat));
  outMsg.addAttribute(HttpMessage::Attribute("content-length", std::to_string(msgBody.size())));
  outMsg.addBody(msgBody);
}

// -----< Handle Get All Files List Request >--------------------------------------
void RemotePublisher::ManageGetAllFiles(HttpMessage & msg, HttpMessage& outMsg) {
  std::string msgBody = "";
  for (auto it = categories_.begin(); it != categories_.end(); ++it) {
    for (std::string file : it->second)
      msgBody += file + "\n";
  }
  outMsg.addAttribute(HttpMessage::Attribute("Type", "AllFiles"));
  outMsg.addAttribute(HttpMessage::Attribute("content-length", std::to_string(msgBody.size())));
  outMsg.addBody(msgBody);
}

// -----< Handle download file request >-------------------------------------------
void RemotePublisher::ManageGetFile(HttpMessage & msg, HttpMessage& outMsg) {
  std::string filename = msg.findValue("Filename");
  if (filename == "") return;
  outMsg.addAttribute(HttpMessage::Attribute("Type", "File"));
  outMsg.addAttribute(HttpMessage::Attribute("Filename", filename));
}

// -----< Handle file received message >-------------------------------------------
void RemotePublisher::ManageFileMessage(HttpMessage & msg) {
  std::string filename = msg.findValue("Filename");
  if (filename == "") return;
  // rescan the repository
  for (auto it = categories_.begin(); it != categories_.end(); ++it)
    categories_[it->first].clear();
  directories_.clear();
  ScanRoot(working_directory_);
}

// -----< Handle delete file request message >-------------------------------------
void RemotePublisher::ManageDeleteFileMessage(HttpMessage & msg, HttpMessage & outMsg) {
  std::string filename = msg.findValue("Filename");
  bool removed = FileSystem::File::remove(working_directory_ + filename);
  outMsg.addAttribute(HttpMessage::Attribute("Type", "DeleteResult"));
  outMsg.addAttribute(HttpMessage::Attribute("Result", std::to_string(removed)));
  outMsg.addAttribute(HttpMessage::Attribute("Filename", filename));
  std::string msgBody = "Delete file result message body";
  outMsg.addAttribute(HttpMessage::Attribute("content-length", std::to_string(msgBody.size())));
  outMsg.addBody(msgBody);
  if (removed) {
    // rescan the repository
    for (auto it = categories_.begin(); it != categories_.end(); ++it)
      categories_[it->first].clear();
    directories_.clear();
    ScanRoot(working_directory_);
  }
}

// -----< Handle get published webpages message >-----------------------------------
void RemotePublisher::ManageGetPublished(HttpMessage & msg, HttpMessage & outMsg) {
  outMsg.addAttribute(HttpMessage::Attribute("Type", "PublishedWebpages"));
  outMsg.addAttribute(HttpMessage::Attribute("Port", std::to_string(iis_port_)));
  std::string msgBody = "";
  for (std::string webpage : iis_webpages_)
    msgBody += webpage + "\n";
  outMsg.addAttribute(HttpMessage::Attribute("content-length", std::to_string(msgBody.size())));
  outMsg.addBody(msgBody);
}


// ---------------------------------------------------------------------------------
// TEST STUB
// ---------------------------------------------------------------------------------
#ifdef TEST_REMOTEPUBLISHER

int main(int argc, char** argv) {
  try {
    std::cout << "\n            Starting Remote Publisher ";
    std::cout << "\n ===============================================";
    RemotePublisher rPublisher;
    if (!rPublisher.ProcessCommandLineArgs(argc, argv))
      return 1;
    std::cout << "\n\n  Remote client receiving messages:\n";
    rPublisher.Wait();
  }
  catch (...) {
    std::cout << "\n\n\n  Something wrong happened. Terminating ... \n\n";
    return 1;
  }
  return 0;
}

#endif