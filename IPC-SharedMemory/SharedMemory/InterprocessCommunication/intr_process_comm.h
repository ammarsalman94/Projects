//////////////////////////////////////////////////////////////////////////////////
// intr_process_comm.h : defines interprocess communication system              //
// ver 1.0                                                                      //
//                                                                              //
// Application : Independent Study, Professor Jim Fawcett                       //
// Platform    : Visual Studio 2017, Windows 10 Pro x64, Acer Aspire R15        //
// Author      : (C) Ammar Salman, EECS Department, Syracuse University         //
//               313/788-4694, hoplite.90@hotmail.com                           //
//////////////////////////////////////////////////////////////////////////////////
/*
*  Package Description:
* ======================
*  This package defines interprocess communication class that uses shared-memory
*  native WINAPI functions. All processes in this system share one memory area 
*  that they put their names in. If that memory was not there, the first process
*  to initialize will create it. Other processes will get handle to the memory
*  area. Once a process joins the IPC system, it will send a connection message
*  to all registered processes, i.e., all processes that have their names in the
*  shared-memory area. On exit, the process sends connection closed message.
*
*  Each process allocates shared-memory area associated with it. This area is
*  used to receive messages. The area can be imagined as a single-item buffer
*  with multiple producers and one consumer (the process that initialized it).
*
*  This system allows users to specify the maximum allowed message size they
*  wish the system to accept. Any larger messages will not be sent.
*  This figure can only be set when the communication object is in the closed
*  state. Once closed and set, the initialization will set up the system with
*  the new value of the maximum.
*
*  The connection channel(s) determine the maximum size of the target buffer
*  they just got connected to. The used function is VirtualQuery from WinAPI.
*  The documentation of VirtualQuery indicates that the function rounds up
*  the number to match the allocated pages size of the buffer. Since the page 
*  size on Windows is 4KB, the VirtualQuery will always return multiples of 4K.
*  However, the CreateFileMapping and MapViewOfFile functions do allow any
*  number to be entered. E.g., you can set the max to 5000, but then, the
*  VirtualQuery will round that up to 8192 (2*4096) even though the real
*  allowed size is 5000. This is caused by the way pages are allocated as
*  Windows cannot allocate a portion of a page, a full page is always used.
*
*  Therefore, this package does not accept any max-msg-size that is not a
*  multiple of 4K to make sure there are no problems while sending.
*
*  Any message that is larger than the target buffer will be blocked until
*  the target process calls close() and initialize() or it completely res-
*  tarts itself. If the max size is still less than the message size, the
*  message will simply be blocked again.
*
*  Each IPC system has thread to send messages and another to receive messages.
*  On initialization, each process creates two semaphores:
*    receiving semaphore : initial count = 0; max count = 1
*    waiters semaphore   : initial count = 1; max count = 0
*  Upon initialization, the buffer is empty which means there is nothing to 
*  receive yet. That's why the receiving semaphore is not free while the waiters
*  semaphore is. 
*  
*  Only one message can be put at a time any process attempting to put message
*  into a full-buffer will block until that message is consumed. When a message
*  is put inside the buffer, the receiving semaphore will be signaled. At this
*  time, the waiters semaphore will block since it was acquired by the process
*  that placed the message, and it's not released yet. The receiver process will
*  consume the message and then signal the waiters semaphore which will free
*  one of the waiters to put another message in the buffer. And so on...
*
*  Normally, only the receiver can release the waiters semaphore while only the
*  waiters can release the receiver semaphore. This way each of them can tell 
*  each other when to start performing. 
*
*  All messages must be serialized before sending. std::string works well for
*  serialization since it can be converted to C type string using '.c_str()'
*  function. This leaves us with another problem; how to construct the message
*  and deserialize it. See 'ver 2.0' remarks for changes in serialization and
*  deserialization.
*
*  The intr_process_locking class mainly holds two semaphores (not created in
*  process that uses this class), and handles to a buffer associated with some
*  other process. In other words, this class is a channel/proxy to other hosts.
*  Initially I had only one instance of this class per intr_process_comm
*  instance. Each time a new message is to be sent, the instance connects to
*  the target process and sends the message.
*
*  For performance reasons, I have added an std::unordered_map with std::string
*  key (representing target process name) and intr_process_locking value (i.e. 
*  the channel/proxy). Each time a new message is to be sent, the system checks
*  if a connection was already established, if not, it creates the channel, then
*  it continues to attempt to send the message. 
*
*  Also to improve performance, the channel does not wait indefinitely on some
*  host to consume a message. That is, if the target process receiver is having
*  some trouble and is running slow, other processes will wait a timeout period
*  which if exceeded, the message will be put back in the end of the out going
*  messages queue. Of course, the option to wait indefinitely is still present.
*
*
*  Notes:
* ===================
*  This IPC system can be used multiple times in the same process without any
*  problems since there are no static members. One process can instanitate 
*  multiple instances of this IPC.
*
*
*  Public Interface:
* ===================
*    Msg Struct:
*   -------------
*    Msg msg;
*    msg.source = "processA";             // sets the source of the message
*    msg.destination = "processB";        // sets the destination of the message
*    msg.command = "connection";          // sets the command of the message - can have multiple definitions
*    msg.body = "closed";                 // sets the body content - in this case it is a 'connection closed message'
*   
*    std::string str = msg.str();         // returns serialized string which can be understood by 'from_str'  
*    Msg msg = Msg::from_str(msg_str);    // reconstructs message from serialized string
*
*    intr_process_locking class:
*   -----------------------------
*    intr_process_locking channel;
*    channel.connect("processA");   // connect to processA buffer
*    channel.wait(INFINITE);        // waits indefinitely on the semaphore
*    channel.put_message(msg);      // accepts Msg struct, serializes it and puts it on the buffer
*    channel.put_message(xml_str);  // accepts serialized Msg and puts it on the buffer
*    channel.signal();              // signals the target procecss of a new message arrival
*    channel.close();               // close connections
*    channel.connection_state();    // returns the status of the channel (see enum state)
*    channel.target_process();      // returns name of the process this channel is connected to
*
*    intr_process_communication class:
*   -----------------------------------
*    intr_process_communication comm;
*    comm.maximum_msg_size(...);         // gets/sets maximum message size - must be a multiple of 4K
*    if(!comm.initialize("processA"));   // tries to setup communication given process name
*    comm.name();                        // gets the name of the comm object set upon initialization
*    comm.current_state();               // gets the current state of the system (see communication::state enum)
*    comm.post_message(msg);             // accepts Msg struct and puts it in the out_queue
*    comm.post_message(to, cmd, body);   // accepts msg attributes, constructs Msg and queues it for sending
*    Msg msg = comm.get_message();       // deQs and returns one of the received messages in in_queue
*    comm.get_message_str();             // deQs and returns the xml string of a received message
*    comm.wait_period(...);              // gets/sets the timeout period (in milliseconds) for the sender - default is 5ms
*    comm.message_count();               // returns number of communication messages currently in the queue
*    comm.connected_processes();         // returns std::vector<std::string> holding all subscribed processes names
*    comm.send_all_before_closing(...);  // gets/sets whether or not to send all queued messages to their destinations
*    comm.close();                       // closes all handles and connections 
*    
*    // diagnosis functions
*    comm.get_control_message();         // deQs and returns a control message
*    comm.total_received();              // returns total number of received messages including control messages
*    comm.total_sent();                  // returns total number of sent messages including control messages
*    string err = comm.last_error();// returns last logged error 
*    
*
*  Required Files:
* =================
*  intr_process_comm.h intr_process_comm.cpp Cpp11-BlockingQueue.h XmlDocument.h
*
*  Build Process:
* =================
*  devenv ProcessTemplate.vcxproj
*
*  Maintainance History:
* =======================
*  ver 1.0 December 3rd 2017
*    - first release
*
*  ver 2.0 December 14th 2017
*    - Msg class was optimized to use fast serialization/deserialization mechanism.
*      Msg class no longer uses XML serialization/deserialization - references removed.
*      XML deserialization used to significantly increase with the increase of message
*      size. The new deserialization mechanism time barely increases with the incraese
*      in message size. 
*
*    - intr_process_comm::close() no longer needs to sleep for 100ms. Instead, it keeps
*      checking if all references to this handle are closed. As long as the references
*      are not released, it will stay in 'closing' state. Once all of them are released,
*      it will change to 'closed' and the system can be reopened again.
*
*    - Removed some diagnosis functions and variables as they used to inacurately 
*      measure average sending/receiving times. Such functions are now left for users
*      to implement.
* 
*    - Removed 'inner_release' variable. Receiver thread no longer loses any messages
*      when the system is closing. All messages are gauranteed to arrive.
*  
*    - Added checking if the shared memory area is full or not. When a process enters
*      the communication system, it has to register its name in the shared memory area.
*      If the shared memory area is full, which is very rare to happen since its already
*      big enough, the communication will fail to initialize.
*
*    - Added last_error std::string variable so users can know why the system was not
*      able to do some specific task.
*
*    - Added intr_process_comm state public function and its corresponding extern C
*      function so C# users can know the state of the communication system.
*
*  Future Work:
* =======================
*  Create private kernel namespace for each process. This is a heavy work project
*  that is needed for to add security measures to the system.
*
*/

#pragma once

// for interprocess communication
#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include <tchar.h>

// CPP libraries
#include <vector>
#include <string>
#include <thread>
#include <unordered_map>

// for in and out queues
#include "../Cpp11-BlockingQueue/Cpp11-BlockingQueue.h"

// for diagnosis
#include <chrono>


#define PBUFF_SIZE 1024 * 128   // processes names area max size - set to 128KB - can hold upto 1024 processes

namespace communication {

  // -----< Msg struct - shared accross processes >----------------------
  struct Msg {
    std::string source;
    std::string destination;
    std::string command;
    std::string body;

    // fast serialization mechanism
    std::string str() const;
    static Msg from_str(const std::string& msg_str);
  };

  // -----< state enum - to check the status of IPC objects >-------------
  enum state {
    opening,
    opened,
    closing,
    closed,
    corrupted
  };

  // -----< intr_process_locking - channel/proxy to other processes >-----
  class intr_process_locking {
  public:
    ~intr_process_locking();
    state connect(const std::string& target_name);
    const size_t & max_msg_size() const;
    state close();

    DWORD wait(unsigned long period = INFINITE);
    bool put_message(const std::string& msg_str);
    bool put_message(const Msg& msg);
    void signal();

    const state& connection_state() const;
    const std::string& target_process() const;

  private:
    HANDLE trgt_file;
    LPSTR trgt_map;

    HANDLE sndr_sem;
    HANDLE trgt_sem;

    size_t trgt_max_size;

    state conn_state;
    std::string target_process_name;
  };


  // -------------------------------------------------------------------------------
  // -----< interprocess communication class declarations >-------------------------
  // -------------------------------------------------------------------------------

  class intr_process_comm
  {
  public:
    intr_process_comm();
    ~intr_process_comm();

    const std::string& name() const;
    const size_t& maximum_msg_size() const;
    bool maximum_msg_size(size_t size);

    bool initialize(const std::string& pname);
    void post_message(const Msg& msg);
    void post_message(const std::string& to, const std::string& cmd, const std::string& body);

    Msg get_message();
    std::string get_message_str();
    std::string get_control_message();

    size_t msg_count();

    void close();

    const unsigned long& wait_period() const;
    void wait_period(unsigned long period);

    const std::vector<std::string>& connected_processes() const;
    std::string connected_processes_str();

    const bool & send_all_before_closing() const;
    void send_all_before_closing(bool s);

    const state& current_state() const;

  private:
    bool create_shared_message_file();
    bool create_rcvr_semaphores();

    void close_threads();
    void close_handles_and_connections();

    bool connect_to_process_names_pagefile();
    void get_processes_names();
    bool put_this_name(const std::string& name = "");
    void remove_this_name();
    void send_connection_messages(bool opened);
    void process_connection_message(Msg msg);

    void rcvr_thread_proc();
    void sndr_thread_proc();

  private:
    std::string this_name;
    size_t max_msg_size;
    state conn_state;

    // handles owned by this process
    HANDLE hmsg_file;
    LPSTR hmf_mapview;

    HANDLE rcvr_sem;
    HANDLE wait_sem;

    // processes names list memory area handles
    HANDLE hprocesses_namepage;
    LPSTR hpnp_mapview;
    HANDLE hpnp_mutex;

    // list of all identified processes 
    std::vector<std::string> processes_names;
    // holds connections locking objects for faster communication
    std::unordered_map<std::string, intr_process_locking> connections;

    // no need to identify as static
    BlockingQueue<Msg> in_messages;
    BlockingQueue<Msg> out_messages;

    unsigned long wait_timeout;
    std::unordered_map<std::string, std::vector<Msg>> blocked_messages;

    // threads for receiving and sending messages
    std::thread receiver_thread;
    std::thread sender_thread;
    bool send_before_closing;


    // diagnosis
  public:
    const size_t & total_received() const;
    const size_t & total_sent() const;
    const std::string& last_error() const;

  private:
    void log(const std::string& command, const std::string& description);
    BlockingQueue<std::string> control_messages;

    size_t rcv_message_count;
    size_t snd_message_count;

    std::string last_err;
  };

}


// -------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------
// ----------< expose Cpp communication::intr_process_comm using C library >------------------
// -------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------

extern "C" {
  typedef void* comm_obj;

  /* Returns interprocess communication object instance. */
  __declspec(dllexport) comm_obj create_comm_object() {
    return new communication::intr_process_comm();
  }

  /* Destroys intr_process_communication object. */
  __declspec(dllexport) void destroy_comm_object(comm_obj obj) {
    delete static_cast<communication::intr_process_comm*>(obj);
  }

  /* Get the current maximum message size allowed for this communicaiton object. */
  __declspec(dllexport) size_t max_msg_size(comm_obj obj) {
    return static_cast<communication::intr_process_comm*>(obj)->maximum_msg_size();
  }

  /* Set the current maximum message size allowed for this communication object. */
  __declspec(dllexport) bool set_max_msg_size(comm_obj obj, size_t size) {
    return static_cast<communication::intr_process_comm*>(obj)->maximum_msg_size(size);
  }

  /* Initializes interprocess communication given process name. */
  __declspec(dllexport) bool initialize(comm_obj obj, const char* pname) {
    return static_cast<communication::intr_process_comm*>(obj)->initialize(pname);
  }

  /* Get the name of the communication object. */
  __declspec(dllexport) void name(comm_obj obj, char * out, size_t size) {
    std::string name = static_cast<communication::intr_process_comm*>(obj)->name();
    if (name.size() == 0) return;
    strcpy_s(out, size, name.c_str());
  }

  /* Puts message in interprocess communication out queue. */
  __declspec(dllexport) void post_message(comm_obj obj, const char* to, const char* cmd, const char* body) {
    static_cast<communication::intr_process_comm*>(obj)->post_message(to, cmd, body);
  }

  /* Returns XML string message from interprocess communication object. */
  __declspec(dllexport) void get_message(comm_obj obj, char* out, size_t size) {
    std::string str = static_cast<communication::intr_process_comm*>(obj)->get_message_str();
    strcpy_s(out, size, str.c_str());
  }

  /* Returns XML string message from the control messages queue. */
  __declspec(dllexport) void get_control_message(comm_obj obj, char * out, size_t size) {
    std::string str = static_cast<communication::intr_process_comm*>(obj)->get_control_message();
    strcpy_s(out, size, str.c_str());
  }

  /* Returns interprocess communication object's in-queue size. */
  __declspec(dllexport) int message_count(comm_obj obj) {
    return static_cast<int>(static_cast<communication::intr_process_comm*>(obj)->msg_count());
  }

  /* Returns the current wait period in interprocess communication object. */
  __declspec(dllexport) long wait_period(comm_obj obj) {
    return static_cast<long>(static_cast<communication::intr_process_comm*>(obj)->wait_period());
  }

  /* Sets the wait period in interprocess communication object. */
  __declspec(dllexport) void set_wait_period(comm_obj obj, long period) {
    static_cast<communication::intr_process_comm*>(obj)->wait_period(period);
  }

  /* Returns string-list of all connected processes of an interprocess communication object. */
  __declspec(dllexport) void processes_names(comm_obj obj, char* out, size_t size) {
    std::string str = static_cast<communication::intr_process_comm*>(obj)->connected_processes_str();
    if (str.size() == 0) return;
    strcpy_s(out, size, str.c_str());
  }

  /* Returns current state of the communication system. */
  __declspec(dllexport) int current_state(comm_obj obj) {
    return static_cast<communication::intr_process_comm*>(obj)->current_state();
  }

  /* Closes the connection of an interprocess communication object. */
  __declspec(dllexport) void close(comm_obj obj) {
    static_cast<communication::intr_process_comm*>(obj)->close();
  }

  /* Returns total number of received messages, including control messages. */
  __declspec(dllexport) size_t rcvd_messages_count(comm_obj obj) {
    return static_cast<communication::intr_process_comm*>(obj)->total_received();
  }

  /* Returns total number of sent messages, including control messages. */
  __declspec(dllexport) size_t sent_messages_count(comm_obj obj) {
    return static_cast<communication::intr_process_comm*>(obj)->total_sent();
  }

  /* Returns last error message. */
  __declspec(dllexport) void last_error(comm_obj obj, char* out, size_t size) {
    std::string str = static_cast<communication::intr_process_comm*>(obj)->last_error();
    if (str.size() == 0) return;
    strcpy_s(out, size, str.c_str());
  }
}
