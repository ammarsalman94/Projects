//////////////////////////////////////////////////////////////////////////////////
// intr_process_comm.cpp : contains definitions for intr_process_comm.h         //
// ver 1.0                                                                      //
//                                                                              //
// Application : Independent Study, Professor Jim Fawcett                       //
// Platform    : Visual Studio 2017, Windows 10 Pro x64, Acer Aspire R15        //
// Author      : Ammar Salman, EECS Department, Syracuse University             //
//               313/788-4694, hoplite.90@hotmail.com                           //
//////////////////////////////////////////////////////////////////////////////////

#include "intr_process_comm.h"

#include <sstream>
#include <iomanip>

using namespace communication;

// ------------------------------------------------------------------------------
// -----< Global functions for time printing >-----------------------------------
// ------------------------------------------------------------------------------

// -----< Print Time in ms precision >---------------------------------
template <typename Duration>
std::string get_time(tm t, Duration fraction) {
  using namespace std;
  using namespace std::chrono;
  ostringstream stream;
  stream << setfill('0');
  stream << setprecision(4) << "[" << setw(4) << t.tm_year + 1900 << "-";
  stream << setprecision(2);
  stream << setw(2) << t.tm_mon + 1 << "-" << setw(2) << t.tm_mday << " ";
  stream << setw(2) << t.tm_hour << ":" << setw(2) << t.tm_min << ":" << setw(2) << t.tm_sec << ":";
  stream << setprecision(3) << setw(3) << static_cast<unsigned>(fraction / milliseconds(1)) << "] ";

  return stream.str();
}

// -----< Print current time in ms precision >-------------------------
std::string get_current_time() {
  using namespace std;
  using namespace std::chrono;

  system_clock::time_point now = system_clock::now();
  system_clock::duration tp = now.time_since_epoch();

  tp -= duration_cast<seconds>(tp);

  time_t tt = system_clock::to_time_t(now);

  struct tm timeinfo;
  localtime_s(&timeinfo, &tt);
  return get_time(timeinfo, tp);
}

// ---------------------------------------------------------------------------------
// -----< Message struct definiaitons >---------------------------------------------
// ---------------------------------------------------------------------------------

// -----< Serialize message >------------------------------------------
/* Assuming the following message structure:
*  source:      'processA'       -- 08 characters
*  destination: 'processB'       -- 08 characters
*  command:     'command'        -- 07 characters
*  body:        'message body'   -- 12 characters
*
*  The size of 'size_t' is 4 bytes on Windows. Therefore, the 
*  serialized message would look like this:
*  [0008processA0008processB0007command000Amessage body]
*  Each attribute is preceeded by its size (4 bytes) represented
*  in the hexidecimal format. The size tells deserialization
*  how to reconstruct the attributes.
*/
std::string communication::Msg::str() const
{
  std::ostringstream serialized;
  serialized << std::setfill('0');
  serialized << std::setw(sizeof(size_t)) << std::hex << source.size() << source;
  serialized << std::setw(sizeof(size_t)) << std::hex << destination.size() << destination;
  serialized << std::setw(sizeof(size_t)) << std::hex << command.size() << command;
  serialized << std::setw(sizeof(size_t)) << std::hex << body.size() << body;
  serialized << '\0'; // <-- terminator 
  return serialized.str();
}

// -----< reconstruct message from string >--------------------------
/* See Msg::str() function to check the representation of the 
*  serialized message. Assuming the same message described there:
*   [0008processA0008processB0007command000Amessage body]
*  To recontruct it:
*  1. bytes 0-3 are converted to number: 8.
*  2. bytes 4-11 are taken as message source.
*  3. bytes 12-15 are converted to number: 8.
*  4. bytes 16-23 are taken as message destination.
*  5. bytes 24-27 are converted to number: 7.
*  6. bytes 28-34 are taken as message command.
*  7. bytes 35-38 are converted to number: 12.
*  8. bytes 39-40 are taken as message body.
*/
Msg communication::Msg::from_str(const std::string & msg_str)
{
  Msg msg;
  size_t i1 = 0, i2 = sizeof(size_t);
  size_t size;
  
  size = std::strtoull(msg_str.substr(i1, i2).c_str(), 0, 16);
  msg.source = msg_str.substr(i1 + i2, size);

  i1 += i2 + size;
  size = std::strtoull(msg_str.substr(i1, i2).c_str(), 0, 16);
  msg.destination = msg_str.substr(i1 + i2, size);

  i1 += i2 + size;
  size = std::strtoull(msg_str.substr(i1, i2).c_str(), 0, 16);
  msg.command = msg_str.substr(i1 + i2, size);

  i1 += i2 + size;
  size = std::strtoull(msg_str.substr(i1, i2).c_str(), 0, 16);
  msg.body = msg_str.substr(i1 + i2, size);

  return msg;
}

// ---------------------------------------------------------------------------------
// -----< interprocess locking class definiaitons >---------------------------------
// ---------------------------------------------------------------------------------

// -----< destructor for the channel >----------------------------------
communication::intr_process_locking::~intr_process_locking()
{
  close();
}

// -----< connect to other process >------------------------------------
/* connects to another process's shared memory area.
*  gets handle to the memory area itself.
*  gets handle to the map view of the memory file
*  connects to the other process's semaphores
* 
*  returns connection result. any failure would return false.
*/
state intr_process_locking::connect(const std::string & target_name)
{
  // if connection is already established to the target 
  // process return immediately with opened state
  if (target_process_name == target_name && conn_state == state::opened)
    return state::opened;

  target_process_name = target_name;
  // set status to state::opening 
  conn_state = state::opening;

  std::string fname("Global\\" + target_name);
  auto str = fname.c_str();
  //std::cout << "\n  Connecting to '" << str << "'";
  trgt_file = OpenFileMapping(FILE_MAP_WRITE, FALSE, str);
  
  // if file mapping opening failed, return closed connection
  if (!trgt_file) {
    conn_state = state::closed;
    return conn_state;
  }

  trgt_map = (LPSTR)MapViewOfFile(trgt_file, FILE_MAP_WRITE, 0, 0, 0);
  
  // get size of the target process's memory 
  MEMORY_BASIC_INFORMATION info;
  VirtualQuery(trgt_map, &info, sizeof(info));
  trgt_max_size = info.RegionSize;
  
  // if the map view failed return closed connection
  if (!trgt_map) {
    conn_state = state::closed;
    return conn_state;
  }


  // this sender connects to the wait semaphore in the target process
  sndr_sem = OpenSemaphore(SYNCHRONIZE | SEMAPHORE_MODIFY_STATE, FALSE, ("Global\\" + target_name + "_wait_sem").c_str());
  // this target connects to the receiver semaphore in the target process
  trgt_sem = OpenSemaphore(SYNCHRONIZE | SEMAPHORE_MODIFY_STATE, FALSE, ("Global\\" + target_name + "_rcvr_sem").c_str());

  // if semaphores connection failed, return closed
  bool sems_result = sndr_sem && trgt_sem;
  if(!sems_result) {
    conn_state = state::closed;
    return conn_state;
  }

  // if this was reached then everything went okay, return opened state
  conn_state = state::opened;
  return conn_state;
}

// -----< returns target process max msg size >-------------------
const size_t & communication::intr_process_locking::max_msg_size() const
{
  return trgt_max_size;
}

// -----< close connection >--------------------------------------
/* closes all handles to another process shared memory area*/
state intr_process_locking::close()
{
  if (conn_state == state::closed) return conn_state;
  conn_state = state::closing;
  CloseHandle(trgt_file);
  UnmapViewOfFile(trgt_map);
  CloseHandle(sndr_sem);
  CloseHandle(trgt_sem);

  conn_state = state::closed;
  return conn_state;
}

// -----< wait to get access to other process memory >------------
DWORD intr_process_locking::wait(unsigned long period)
{
  return WaitForSingleObject(sndr_sem, period);
}

// -----< put memory content in target process area >-------------
/* if the message is larger than the target process area
*  this function will return false. Otherwise it will return true*/
bool intr_process_locking::put_message(const std::string & msg_str)
{
  // message is larger than the target process maximum message size
  if (msg_str.size() > trgt_max_size) return false;
  CopyMemory(trgt_map, msg_str.c_str(), msg_str.size());
  return true;
}

// -----< put memory content in target process area >-------------
/* if the message is larger than the target process area
*  this function will return false. Otherwise it will return true*/
bool intr_process_locking::put_message(const Msg & msg)
{
  //std::string xml_msg = msg.to_xml();
  return put_message(msg.str());
}

// -----< signal the target process that there is a message >------
void intr_process_locking::signal()
{
  ReleaseSemaphore(trgt_sem, 1, 0);
}

// -----< get connection status >----------------------------------
const state & intr_process_locking::connection_state() const
{
  return conn_state;
}

// -----< get target process name >--------------------------------
const std::string & intr_process_locking::target_process() const
{
  return target_process_name;
}


// ---------------------------------------------------------------------------------
// -----< interprocess comm class definiaitons >------------------------------------
// ---------------------------------------------------------------------------------

// -----< constructor >-------------------------------------------
intr_process_comm::intr_process_comm() : wait_timeout(5), max_msg_size(4096), conn_state(state::closed){}

// -----< destructor - closes everything >------------------------
intr_process_comm::~intr_process_comm()
{
  close();
}

// -----< get this process name in the comm system >-------------------
const std::string & communication::intr_process_comm::name() const
{
  return this_name;
}

// -----< return maximum msg size this process can receive >-----------
const size_t & communication::intr_process_comm::maximum_msg_size() const
{
  return max_msg_size;
}

// -----< set the maximum message size >-------------------------------
bool communication::intr_process_comm::maximum_msg_size(size_t size)
{
  if (conn_state != state::closed)
    return false;  // communication has to be closed before this can be set
  if (size % 4096 != 0) 
    return false;  // size must be a multiple of 4KB
  max_msg_size = size;
  return true;
}

// -----< intialize communication object >------------------------
/* accepts process name and initializes the comm object
*  Size of the name must be less than or equal to 120 characters.
*  This limit is to ensure processes do not have absurd long names.
*  
*  the connection status must be closed for initialization to work.
*  if it was opened, this will return false indicating it will not
*  attempt to reopen it on a different name. close() must be called
*  before any re-initialization is attempted.
*
*  this will send connection messages to all identified processes
*  informing them that there is a new process. this is especially
*  helpful when other processes have messages meant for this one
*  and this process was offline. the connection message will be
*  received by other processes and they will immediately attempt
*  to send any blocked messages back to this process.
*
*  if this communication was closed before, inner_release will be
*  set to true (see close()). it is very important to set it back
*  to false in order for the receiving thread not to quit after 
*  its initialization. 
*
*  this method will return false if close() did not finish executing
*  as close() will set the state to closing until it finishes and 
*  then it will set it to closed so this will be allowed to run.
*/
bool intr_process_comm::initialize(const std::string & pname)
{
  //log("initializing", "attempting to initialize communication");

  // cannot initialize unless state is closed
  if (conn_state != state::closed)
    return false;

  conn_state = state::opening;
  this_name = pname;

  connect_to_process_names_pagefile();
  get_processes_names();
  if (!put_this_name()) {
    conn_state = state::closed;
    last_err = "Processes names shared area is full. Cannot put this process name there.";
    //log("initialization", "initialization error, details: " + last_err);
    return false;
  }

  if (!create_shared_message_file()) {
    conn_state = state::closed;
    last_err = "Could not create message queue.";
    //log("initialization", "initialization error, details: " + last_err);
    return false;
  }

  send_connection_messages(true);
  
  conn_state = state::opened;

  // start up the threads 
  receiver_thread = std::thread(&intr_process_comm::rcvr_thread_proc, this);
  sender_thread = std::thread(&intr_process_comm::sndr_thread_proc, this);

  // diagnosis properties
  rcv_message_count = 0;
  snd_message_count = 0;

  //log("initialization", "communication was successfully initialized");
  return true;
}

// -----< close communication object >--------------------------------------
/* closes all handles and connections established
*  there are three threads executing at this stage which need to be terminated.
*  this method allows all threads to normally exit execution, nothing is forced
*  
*  first this will set the state to closing so that no one can initialize
*  during the closing. This is important because initialization performs on
*  the same variables as the closing.
*
*  in case there are external threads waiting on in_messages and control_messages
*  queues, they have to be notified that this is exiting so a message is prepared
*  and enqueued in both these queues saying "self-closing".
*
*  if the user desires to send all messages before closing and wait until
*  they are all sent, they can do that by setting "send_before_exiting" to true.
*  if not, the out_messages queue will be emptied and all messages there will 
*  be discarded. However, a new set of messages must be sent, that is, the 
*  connection closed messages. these messages inform other processes that this
*  process closed its connection and so other processes will close their
*  channels to this and block any messages they have to this process. 
*  
*  later, this will release its own receiving semaphore to terminate receiving
*  thread (see rcvr_thread_proc()). 
*
*  then this will close all handles and iterate through the connections and 
*  close all of them. 
*
*  Finally, it will make sure that other processes have closed their handles
*  to this process' memory area and semaphores. This ensures that the next
*  time initialization is attempted, it will not face an error saying
*  resources are already allocated. 
*/
void intr_process_comm::close()
{
  if (conn_state == state::closed) {
    //log("closing error", "communication services are already closed");
    last_err = "Could not close communication system. System already closed.";
    return;
  }
  conn_state = state::closing;
  //log("closing", "communication system closing all services");

  // remove this process's name from the names list
  remove_this_name();

  close_threads();
  close_handles_and_connections();

  // the following code makes sure other systems have closed their handles to this one
  while (true) {
    hmsg_file = CreateFileMapping(INVALID_HANDLE_VALUE, 0, PAGE_READWRITE, 1024, 0, ("Global\\" + this_name).c_str());
    //std::cout << "  GetLastError(): " << GetLastError();
    if (hmsg_file != NULL) {
      CloseHandle(hmsg_file);
      break;
    }
    if (GetLastError() != ERROR_ALREADY_EXISTS) break;
  }

  // safe to return now
  conn_state = state::closed;
  //log("closed", "communication closed successfully");
}

// ----< private function to close running threads >----------------------------
void intr_process_comm::close_threads() {
  Msg msg; msg.source = this_name; msg.destination = this_name;
  msg.command = "self_closing"; msg.body = "closing";

  if (!send_before_closing) out_messages.clear();

  if (processes_names.size() == 0)
    out_messages.enQ(msg); // if there were no other processes, allow the sender thread to close normally
  else
    send_connection_messages(false);  // if there are other processes, notify them about your closing
                                      // wait to make sure all messages are sent
  sender_thread.join();

  // close the receiver thread
  ReleaseSemaphore(rcvr_sem, 1, 0);
  receiver_thread.join();
}

// -----< private function to close handles and connections >-------------------
void intr_process_comm::close_handles_and_connections() {
  // close shared memory that holds processes information
  CloseHandle(hprocesses_namepage);
  UnmapViewOfFile(hpnp_mapview);
  CloseHandle(hpnp_mutex);

  // close memory area associated with this connection
  CloseHandle(hmsg_file);
  UnmapViewOfFile(hmf_mapview);
  // close receiver semaphores
  CloseHandle(rcvr_sem);
  CloseHandle(wait_sem);


  // close all conenctions 
  for (auto iter = connections.begin(); iter != connections.end(); ++iter)
    iter->second.close();
}

// -----< establish connection to the names list memory area >--------------------
bool intr_process_comm::connect_to_process_names_pagefile()
{
  TCHAR pnp_location[] = TEXT("Global\\ProcessesDB");

  hprocesses_namepage = OpenFileMapping(
    FILE_MAP_ALL_ACCESS,   // read/write access
    FALSE,                 // do not inherit the name
    pnp_location);         // name of mapping object

  if (hprocesses_namepage == NULL) {
    hprocesses_namepage = CreateFileMapping(
      INVALID_HANDLE_VALUE,    // use paging file
      NULL,                    // default security
      PAGE_READWRITE,          // read/write access
      0,                       // maximum object size (high-order DWORD)
      PBUFF_SIZE,              // maximum object size (low-order DWORD)
      pnp_location);           // name of mapping object
  }

  if (hprocesses_namepage == NULL) return false;

  TCHAR pnp_mutex_name[] = TEXT("Global\\Mutex");

  hpnp_mutex = OpenMutex(
    NULL,            // access
    FALSE,           // initially not owned, no need to release here
    pnp_mutex_name   // name of the shared mutex
  );

  if (hpnp_mutex == NULL) {
    hpnp_mutex = CreateMutex(NULL, FALSE, pnp_mutex_name);
  }
  
  if (hpnp_mutex == NULL) return false;

  hpnp_mapview = (LPSTR)MapViewOfFile(hprocesses_namepage,   // handle to map object
    FILE_MAP_ALL_ACCESS, // read/write permission
    0,
    0,
    PBUFF_SIZE);

  if (hpnp_mapview == NULL) return false;

  // if this was reached then everything was created correctly
  return true;
}

// ----------------------------------------------------------------------------------
// -----< create memory for this process definitions >-------------------------------


// -----< create message shared memory file >---------------------
/* creates shared memory area associated with this process */
bool intr_process_comm::create_shared_message_file()
{
  if (this_name == "") return false;

  hmsg_file = CreateFileMapping(
    INVALID_HANDLE_VALUE,    // use paging file
    NULL,                    // default security
    PAGE_READWRITE,          // read/write access
    0,                       // maximum object size (high-order DWORD)
    (DWORD)max_msg_size,              // maximum object size (low-order DWORD)
    ("Global\\" + this_name).c_str());                // name of mapping object

  if (hmsg_file == NULL) return false;

  hmf_mapview = (LPSTR)MapViewOfFile(hmsg_file,   // handle to map object
    FILE_MAP_ALL_ACCESS, // read/write permission
    0,
    0,
    max_msg_size);

  if (hmf_mapview == NULL) return false;

  bool rcv_result = create_rcvr_semaphores();

  return rcv_result;
}

// -----< create receiver semaphores >-------------------------
/* creates a pair of semaphores used for concurrency safety
*  to fill-empty the contents of this process's memory */
bool intr_process_comm::create_rcvr_semaphores()
{
  // create the semaphore that this process will wait upon
  // initial count is 0 so that this process does not acquire the
  // semaphore and will wait until another process signals it
  rcvr_sem = CreateSemaphore(NULL, 0, 1, ("Global\\" + this_name + "_rcvr_sem").c_str());

  // create the semaphore that other processes will use to fill
  // this process's message queue. Initially available for any
  // process that wants to connect so they can fill immediately.
  wait_sem = CreateSemaphore(NULL, 1, 1, ("Global\\" + this_name + "_wait_sem").c_str());

  return (rcvr_sem && wait_sem);
}

// ---------------------------------------------------------------------------------
// -----< functions associated with the processes name list >-----------------------

// -----< get list of all processes in the communication system >-----
void intr_process_comm::get_processes_names()
{
  processes_names.clear();
  connect_to_process_names_pagefile();
  if (hprocesses_namepage == NULL || hpnp_mutex == NULL || hpnp_mapview == NULL) return;

  DWORD wait_result = WaitForSingleObject(hpnp_mutex, INFINITE);

  if (wait_result == WAIT_OBJECT_0) {
    // cast map file contents to std::string

    std::string str_content(hpnp_mapview);

    std::istringstream in_stream(str_content);

    std::string tmp;

    while (in_stream.good()) {
      in_stream >> tmp;
      if (tmp != this_name && tmp != "")
        processes_names.push_back(tmp);
    }

    ReleaseMutex(hpnp_mutex);
  }
}

// -----< put this process's name in the names list >----------------
bool intr_process_comm::put_this_name(const std::string & name)
{
  if (name != "")
    this_name = name;

  // lock the names list 
  WaitForSingleObject(hpnp_mutex, INFINITE);

  // get the current name list and add this process's name to it
  std::string str_content((LPCSTR)hpnp_mapview);

  if ((str_content.size() + this_name.size()) >= PBUFF_SIZE) {
    ReleaseMutex(hpnp_mutex);
    return false;
  }

  std::istringstream in_stream(str_content);
  std::string tmp;
  bool already_up = false;
  while (in_stream.good()) {
    in_stream >> tmp;
    if (tmp == this_name) {
      already_up = true;
      break;
    }
  }

  // if the process name was not already in the shared memory (in case an error happened and it wasn't removed)
  if (!already_up) {
    str_content += "\n" + this_name;
    // put back the names list
    CopyMemory(hpnp_mapview, str_content.c_str(), str_content.size() * sizeof(char));
  }

  // release the names list lock
  ReleaseMutex(hpnp_mutex);
  return true;
}

// -----< remove this process's name from the names list >------------
void intr_process_comm::remove_this_name()
{
  // obtain the lock for the names list 
  WaitForSingleObject(hpnp_mutex, INFINITE);

  // get the content as string
  std::string buff_content((LPCSTR)hpnp_mapview);

  // find and remove this process's name from the list
  size_t name_pos = buff_content.find(this_name);

  if (name_pos != std::string::npos) {
    buff_content.erase(name_pos, this_name.size() + 1);
    buff_content += '\0';
    // put the modified content back 
    CopyMemory(hpnp_mapview, buff_content.c_str(), buff_content.size());
  }

  // release the lock
  ReleaseMutex(hpnp_mutex);
}


// -----< send connection messages >---------------------------------
/* sends a connection message to all processes in the system */
void intr_process_comm::send_connection_messages(bool opened)
{
  std::string body;
  if (opened)
    body = "opened";
  else
    body = "closed";
  get_processes_names();
  for (size_t i = 0; i < processes_names.size(); i++)
    post_message(processes_names[i], "connection", body);
}

// -----< process connection message >-------------------------------
/* */
void intr_process_comm::process_connection_message(Msg msg)
{
  //log("connection", "process '" + msg.source + "' " + msg.body + " its communication");
  std::string source = msg.source;
  // if a new process just opened up
  if (msg.body == "opened") {
    if (blocked_messages.find(source) != blocked_messages.end()) {
      for (size_t i = 0; i < blocked_messages[source].size(); i++)
        out_messages.enQ(blocked_messages[source][i]);
      blocked_messages.erase(source);
    }
  }
  // if a process is closing up connection
  else if (msg.body == "closed") {
    if (connections.find(source) != connections.end()) {
      connections[source].close();
      //log("channel closed", "closed channel to process " + source);
      connections.erase(source);
    }
  }
  // update names list
  get_processes_names();
}

// ---------------------------------------------------------------------------------
// -----< functions associated with communication >---------------------------------

// -----< post Msg type >--------------------------------------------
void intr_process_comm::post_message(const Msg& msg)
{
  out_messages.enQ(msg);
}

// -----< post message given attributed >----------------------------
void intr_process_comm::post_message(const std::string & to, const std::string & cmd, const std::string & body)
{
  Msg msg;
  msg.source = this_name;
  msg.destination = to;
  msg.command = cmd;
  msg.body = body;
  out_messages.enQ(msg);
}

// -----< get message from the received messages > ------------------
Msg intr_process_comm::get_message()
{
  Msg msg = in_messages.deQ();
  return msg;
}

// -----< return message string - used for extern C function >-------
std::string communication::intr_process_comm::get_message_str()
{
  Msg msg = in_messages.deQ();
  std::string ret = msg.str();
  return ret;
}

// -----< return a control message >---------------------------------
std::string communication::intr_process_comm::get_control_message()
{
  return control_messages.deQ();
}

// -----< return number of unhandled messages in the queue >---------
size_t intr_process_comm::msg_count()
{
  return in_messages.size();
}

// -----< return max wait perdiod for sending messages >-------------
const unsigned long & intr_process_comm::wait_period() const
{
  return wait_timeout;
}

// -----< set wait period max >--------------------------------------
void intr_process_comm::wait_period(unsigned long period)
{
  wait_timeout = period;
}


// -----< return processes names list >--------------------------------
const std::vector<std::string>& intr_process_comm::connected_processes() const
{
  return processes_names;
}

// -----< return processes names list as string >----------------------
std::string communication::intr_process_comm::connected_processes_str()
{
  if (processes_names.size() == 0) return "";
  std::ostringstream out;
  for (size_t i = 0; i < processes_names.size() - 1; i++) {
    out << processes_names[i] << "\n";
  }
  out << processes_names[processes_names.size() - 1];
  return out.str();
}

// -----< whether or not to send all messages before closing >---------
const bool & communication::intr_process_comm::send_all_before_closing() const
{
  return send_before_closing;
}

// -----< set comm to send or ignore out-messages >--------------------
void communication::intr_process_comm::send_all_before_closing(bool s)
{
  send_before_closing = s;
}

// -----< get the current state of the system >------------------------
const state & communication::intr_process_comm::current_state() const
{
  return conn_state;
}


// -----< receiver thread function >-----------------------------------
/*
*  This process waits on the rcvr_sem which is only released by other
*  processes. Once rcvr_sem is signaled, it means there is a message
*  in this process's queue. Now, this process acquires the message
*  and signals one of the waiters that wait on the wait_sem.
*  Only this process is able to release wait_sem to ensure no
*  process can write to this queue unless it's emptied.
*
*  On closing, the close function will release the receiving semaphore
*  to allow this thread to wake up. Once it does, the next loop iteration
*  will check the state and find it not 'opened' so it will terminate. 
*
*/
void intr_process_comm::rcvr_thread_proc()
{
  while (conn_state == state::opened) {
    try {
      // wait on rcvr_semaphore for an infinite time
      WaitForSingleObject(rcvr_sem, INFINITE);
      
      // once signaled by another process, get the message
      Msg msg = Msg::from_str(hmf_mapview);
      CopyMemory(hmf_mapview, "\0", 1);
      // signal one of the waiters that want to put messages 
      // in this process's memory area
      ReleaseSemaphore(wait_sem, 1, 0);

      rcv_message_count++;
      // put this message in this process's queue
      // Msg msg = Msg::from_xml(msg_str);  <--- old de-serialization - slow , do not use
      //Msg msg = Msg::from_str(msg_str);
      if (msg.command == "connection") {
        process_connection_message(msg);
        continue;
      }

      // log message arrival
      //log("received message", "message size: " + std::to_string(msg_str.size()));
      in_messages.enQ(msg);

    }
    catch (std::exception e) {
      //log("error occured", e.what());
    }
  }
}

// -----< sender thread function >-------------------------------------
/* defines the processing done by the sender thread 
*
*  This thread keeps working as long as the connection status is opened
*  or if it was closing, it will work until the out-queue is empty.
*  close() might empty the out-queue if the user wants the system to 
*  discard all outgoing messages on closing. Otherwise, all messages
*  will be sent to their destinations.
*
*  This tries to implement reliable sending service as much as possible.
*  First it will check if a connection to the destination is already
*  established. If not, it will create the channel.
*
*  The thread will try to connect to the channel, if the connection
*  fails, the message will be blocked. Messages stay blocked until
*  the destination sends a connection message when it wakes up.
*
*  All processes will automatically send connection messages on 
*  initialization. This will let other processes update their
*  processes_names list, and will allow other processes to release
*  any blocked messages to the process that initialized.
*
*  The destination process might have a small buffer size. The 
*  size is set to 4KB by default but users are allowed to change
*  it. If the message is larger than the destination buffer size
*  this will detect it and put a message in the control_messages
*  queue telling the problem. 
*
*/
void intr_process_comm::sndr_thread_proc()
{
  while (conn_state == state::opened ||
    // if closing, keep working until the queue is empty. close() might empty the queue
    (conn_state == state::closing && out_messages.size() > 0)) {
    // pull a message out of the queue
    Msg msg = out_messages.deQ();

    // check if there is an established connection to the destination process
    if (connections.find(msg.destination) == connections.end())
      connections[msg.destination].connect(msg.destination);

    // if the connection state is not opened, block this message until target process comes up
    if (connections[msg.destination].connect(msg.destination) != state::opened) {
      blocked_messages[msg.destination].push_back(msg);
      continue;
    }
    std::string msg_str = msg.str();

    //log("sending", "message (cmd:" + msg.command + ", size:" + std::to_string(msg_str.size()) + ") to '" + msg.destination + "'");

    if (connections[msg.destination].max_msg_size() < msg.str().size()) { // if message is larger than target buffer, log and block
      std::string logged = "size of message: " + std::to_string((int)msg_str.size())
        + " - max allowed size for '" + connections[msg.destination].target_process()
        + "':" + std::to_string((int)connections[msg.destination].max_msg_size())
        + " - message blocked";
      //log("sending failed", logged);
      // block this message until target process re-initializes its communication system
      blocked_messages[msg.destination].push_back(msg);
      continue; 
    }

    // if the wait timedout, put the message at the end of the queue and try to send another message
    DWORD wait_result = connections[msg.destination].wait(wait_timeout);
    if (wait_result == WAIT_TIMEOUT) { // waited for longer than the timeout period
      out_messages.enQ(msg);
      continue;
    }
    else if (wait_result != WAIT_OBJECT_0) { // unexpected error occured - probably issue in target process
      connections[msg.destination].close();   // if the target is having issue, close the connection to it
      blocked_messages[msg.destination].push_back(msg);  // block message until target reinitializes
      continue;
    }
    connections[msg.destination].put_message(msg_str); // put the message in the queue memory of the other process
    snd_message_count++;   // increment total number of sent messages
    connections[msg.destination].signal();         // signal receiver process about the new message
    //log("message sent", "message successfully sent to '" + msg.destination + "'");
  }
}

// ------------------------------------------------------------------------
// ----------< diagnosis functions for time measurement >------------------
// ------------------------------------------------------------------------

// -----< log command and message in the control messages >-------------
void communication::intr_process_comm::log(const std::string & command, 
                                           const std::string & description)
{
#ifdef LOG
  std::ostringstream logger;
  logger << std::endl << get_current_time(); // log the time with ms accuracy
  logger << "- [Command: " << std::setw(20) << std::left << command + "]";  // log the command
  logger << " - Details: " << description; // log the details
  control_messages.enQ(logger.str());  // put the log in the control messages queue
#endif
}

// -----< get the amount of received messages since initialization >----
const size_t & communication::intr_process_comm::total_received() const
{
  return rcv_message_count;
}

// -----< get the amount of sent messages since initialization >---------
const size_t & communication::intr_process_comm::total_sent() const
{
  return snd_message_count;
}

// -----< returns last logged error that occurred in the system> --------
const std::string & communication::intr_process_comm::last_error() const
{
  return last_err;
}



// --------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------
// -----< testing and normal run area definitions - contains teststub and normal run >---------------
// --------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------

#ifdef DEMO_IPC

#include <random>

std::string gen_string() {
  static const char alphanum[] =
    "0123456789"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz";

  int rand = (std::rand() % 90) + 10;
  std::string s;
  s.resize(rand);
  for (int i = 0; i < rand; ++i) {
    s[i] = alphanum[std::rand() % (sizeof(alphanum) - 1)];
  }

  return s;
}

bool sending = true;
void snd_proc(intr_process_comm* comm) {
  while (sending) {
    Sleep(2000);
    auto list = comm->connected_processes();
    for (int i = 0; i < 1000; i++) {
      // send message to some random process
      size_t index = (size_t)std::rand() % list.size();
      comm->post_message(list[index], "command", gen_string());
    }
  }
}

bool print = true;
void print_avgs(intr_process_comm* comm) {
  while (print) {
    Sleep(10000);
    std::cout << "\n  " << std::setw(20) <<"Total received: " << std::setw(8) << std::right << comm->total_received() 
      << " - " << std::setw(25) << "Average receiving time: " << std::setw(3) << std::right << comm->avg_rcvr_ms() << " ms";

    std::cout << "\n  " << std::setw(20) << "Total sent: " << std::setw(8) << std::right << comm->total_sent()
      << " - " << std::setw(25) << "Average sending time: " << std::setw(3) << std::right << comm->avg_sndr_ms() << " ms";
  }
}

int main(int argc, char ** argv) {
  std::string name(argv[1]);
  intr_process_comm communication;
  communication.initialize(name);
  //communication.wait_period(INFINITE);

  std::thread t(snd_proc, &communication);
  t.detach();

  std::thread s(print_avgs, &communication);
  s.detach();

  getchar();
  sending = false;
  print = false;
  return 0;
}

#elif TEST_PERFORMANCE

int main(int argc, char ** argv) {
  using namespace std::chrono_literals;
  if (argc != 2) {
    std::cout << "\n  Error: must specify only one argument which is the process name.\n  Terminating..\n\n";
    return 1;
  }
  
  std::string pname(argv[1]);
  intr_process_comm comm;
  comm.maximum_msg_size(8192);

  if (!comm.initialize(pname)) {
    std::cout << "\n  Failed to initialize IPC object using '" << argv[1] << "'\n  Terminating..\n\n";
    return 1;
  }

  if (pname == "processA") {
    std::cout << "\n  Press any key to start sending messages to processB";
    getchar();
    Msg msg;
    msg.source = "processA";
    msg.command = "cmd";
    msg.destination = "processB";
    
    std::cout << "\n  " << get_current_time() << " sending 2000 messages to processB";

    for (int i = 0; i < 1000; i++) {
      msg.body = "message body " + std::to_string(i);
      comm.post_message(msg);
    }

    Msg big_msg;
    big_msg.source = "processA";
    big_msg.command = "cmd";
    big_msg.destination = "processB";
    for (int i = 0; i < 5000; i++)
      big_msg.body += "a";

    for (int i = 0; i < 1000; i++)
      comm.post_message(big_msg);

    while (true)
      if (comm.total_sent() > 1999) break;
    std::cout << "\n  " << get_current_time() << " sent 2000 messages to processB";
  }

  if (pname == "processB") {
    std::cout << "\n  " << get_current_time() << " receiving 2000 messages from processA";
    while (true)
      if (comm.total_received() > 1999) break;
    std::cout << "\n  " << get_current_time() << " received 2000 messages from processA";
  }
  std::cout << "\n  Average receiving time: " << comm.avg_rcvr_ms();
  std::cout << "\n  Average sending time:   " << comm.avg_sndr_ms();
  std::cout << "\n\nPress any key to close communication...";
  getchar();
  return 0;
}

#elif RUN_NORMAL

int main(int argc, char ** argv) {
  try {
    intr_process_comm comm;
    if (argc != 2) return 1;

    if (!comm.initialize(argv[1]))
      return 1;

    getchar();
    return 0;
  }
  catch (...) {
    return 1;
  }
}

#endif