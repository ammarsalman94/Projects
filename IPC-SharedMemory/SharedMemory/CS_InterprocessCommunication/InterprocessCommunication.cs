///////////////////////////////////////////////////////////////////////////////
// InterprocessCommunication.cs:  CSharp interface for C++ IPC.dll library   //
// ver 1.0                                                                   //
//                                                                           //
// Application : Independent Study. Instructor: Professor James Fawcett      //
// Platform    : Visual Studio 2017 - Windows 10 Pro x64 - Acer Aspire R15   //
// Author      : (C) Ammar Salman, EECS Department, Syracuse University      //
//               313/788-4694, hoplite.90@hotmail.com                        // 
///////////////////////////////////////////////////////////////////////////////
/*
 * 
 *  Package Description:
 * ======================
 *  This package specifies an interface for the InterprocessCommunication.dll
 *  library developed in C++ that makes calls to WinAPI C functions. 
 *  
 *  InterprocessCommunication.dll has 'extern "C"' code fragment in which it
 *  exposes a set of C functions that use the C++ intr_process_comm class.
 *  The need of 'extern "C"' fragment is to define a set of C functions which
 *  are by definition unmangled. Name mangling in C++ can cause lots of prob-
 *  lems when we need to expose them to other libraries. C functions can be
 *  exposed by name and called by name as shown below using the DllImport and
 *  'extern static' identifiers.
 *  
 *  Please refer to intr_process.comm.h for details on how the communication
 *  system works. 
 * 
 *  
 *  Public Interface:
 * ===================
 *  // public functions
 *  InterprocessCommunication ipc = new InterprocessCommunication(); // create new IPC instance
 *  bool sres = ipc.SetMaximumMessageSize(8192);                     // set max message size to 8K
 *  bool ires = ipc.Initialize("processA");                          // initialize comm using 'processA' name
 *  ipc.PostMessage("processB", "command", "msg body");              // post message in the sending queue
 *  string msg = ipc.GetMessage();                                   // blocking-call, get message from comm queue
 *  string cmsg = ipc.GetControlMessage();                           // blokcing-call, get control msg from comm queue
 *  ipc.Close();                                                     // close communication object - can still initialize
 *  ipc.Dispose();                                                   // deletes the communication object, must call new
 *  
 *  // public properties
 *  ipc.Name;                // string:  the name on which communication was set on
 *  ipc.MaximumMessageSize;  // UInt: maximum buffer size of the communication object
 *  ipc.ConnectedProcesses;  // List<string>: the other processes in the system
 *  ipc.WaitPeriod;          // long: comm's sender timeout period (see C++ code)
 *  ipc.MessageCount;        // Int: number of received messages currently residing in the queue 
 * 
 *  Required Files:
 * =================
 *  InterprocessCommunication.cs 
 *  InterprocessCommunication.dll <--- required only at runtime, no need for compilation
 * 
 *  Maintainence History:
 * =======================
 *  ver 1.0 Dec. 3rd 2017
 *    - first release
 *    
 *  TODO:
 * =======================
 *  1. Add receiver thread that pulls messages out of the C++ code and put it in private queue.
 *  2. Make calls to the diagnosis functions in the C++ code. 
 * 
 */

using System;
using System.Linq;
using System.Threading;
using System.Runtime.InteropServices;
using System.Text;


namespace CS_InterprocessCommunication
{
  public class InterprocessCommunication : IDisposable
  {
    public class IPCMessage
    {
      public string Source { get; set; }
      public string Destination { get; set; }
      public string Command { get; set; }
      public string Body { get; set; }

      /// <summary> Print message to screen. </summary>
      public void Print()
      {
        Console.WriteLine();
        Console.Write("\r\n       Source : {0}", Source);
        Console.Write("\r\n  Destination : {0}", Destination);
        Console.Write("\r\n      Command : {0}", Command);
        Console.Write("\r\n         Body : {0}", Body);
        Console.WriteLine();
      }

      /// <summary> Reconstruct message from serialized string. </summary>
      /// <param name="MessageString">Serialized message string. </param>
      /// <returns>Interprocess Communication Message instance.</returns>
      public static IPCMessage FromString(string MessageString)
      {
        IPCMessage msg = new IPCMessage();
        int i1 = 0, i2 = sizeof(UInt64);
        int size;

        size = (int)Convert.ToInt64(MessageString.Substring(i1, i2), 16);
        msg.Source = MessageString.Substring(i1 + i2, size);

        i1 += i2 + size;
        size = (int)Convert.ToInt64(MessageString.Substring(i1, i2), 16);
        msg.Destination = MessageString.Substring(i1 + i2, size);

        i1 += i2 + size;
        size = (int)Convert.ToInt64(MessageString.Substring(i1, i2), 16);
        msg.Command = MessageString.Substring(i1 + i2, size);

        i1 += i2 + size;
        size = (int)Convert.ToInt64(MessageString.Substring(i1, i2), 16);
        msg.Body = MessageString.Substring(i1 + i2, size);

        return msg;
      }

    }


    public enum State
    {
      Opening,
      Opened,
      Closing,
      Closed,
      Corrupted
    }

    #region importing functions from the dll

    [DllImport("InterprocessCommunication.dll")]
    extern static IntPtr create_comm_object();
    [DllImport("InterprocessCommunication.dll")]
    extern static void destroy_comm_object(IntPtr obj);
    [DllImport("InterprocessCommunication.dll")]
    extern static uint max_msg_size(IntPtr obj);
    [DllImport("InterprocessCommunication.dll")]
    extern static bool set_max_msg_size(IntPtr obj, uint size);
    [DllImport("InterprocessCommunication.dll")]
    extern static bool initialize(IntPtr obj, string pname);
    [DllImport("InterprocessCommunication.dll")]
    extern static void name(IntPtr obj, StringBuilder name, uint size);
    [DllImport("InterprocessCommunication.dll")]
    extern static void post_message(IntPtr obj, string to, string cmd, string body);
    [DllImport("InterprocessCommunication.dll")]
    extern static void get_message(IntPtr obj, StringBuilder builder, uint size);
    [DllImport("InterprocessCommunication.dll")]
    extern static void get_control_message(IntPtr obj, StringBuilder builder, uint size);
    [DllImport("InterprocessCommunication.dll")]
    extern static int message_count(IntPtr obj);
    [DllImport("InterprocessCommunication.dll")]
    extern static long wait_period(IntPtr obj);
    [DllImport("InterprocessCommunication.dll")]
    extern static void set_wait_period(IntPtr obj, long period);
    [DllImport("InterprocessCommunication.dll")]
    extern static int current_state(IntPtr obj);
    [DllImport("InterprocessCommunication.dll")]
    extern static void processes_names(IntPtr obj, StringBuilder str, uint size);
    [DllImport("InterprocessCommunication.dll")]
    extern static void close(IntPtr obj);

    [DllImport("InterprocessCommunication.dll")]
    extern static uint rcvd_messages_count(IntPtr obj);
    [DllImport("InterprocessCommunication.dll")]
    extern static uint sent_messages_count(IntPtr obj);
    [DllImport("InterprocessCommunication.dll")]
    extern static void last_error(IntPtr obj, StringBuilder str, uint size);

    #endregion importing functions from the dll

    /* Pointer to the communication object in the C++ DLL */
    IntPtr CommObject;

    #region public properties

    /// <summary> Name of the communication object. </summary>
    public string Name {
      get {
        StringBuilder builder = new StringBuilder(128);
        name(CommObject, builder, 128);
        return builder.ToString();
      }
    }

    /// <summary> Message buffer size of the communication object. </summary>
    public uint MaximumMessageSize => max_msg_size(CommObject);

    /// <summary> Timeout period before the message is placed at the end of the out-queue. </summary>
    public long WaitPeriod {
      get {
        return wait_period(CommObject);
      }
      set {
        set_wait_period(CommObject, value);
      }
    }

    /// <summary> Returns current state of the communication system. </summary>
    public State ConnectionState => (State)current_state(CommObject);

    /// <summary> Processes that are available for communication. </summary>
    public string[] ConnectedProcesses
    {
      get {
        StringBuilder str = new StringBuilder(1024*128);
        processes_names(CommObject, str, 1024 * 128);
        return str.ToString().Split('\n').Where(x => !string.IsNullOrEmpty(x)).ToArray();
      }
    }

    /// <summary> Get number of messages inside the receive-queue. </summary>
    public int MessagesCount => message_count(CommObject);

    /// <summary> Get number of total received messages since initialization. </summary>
    public uint TotalReceived => rcvd_messages_count(CommObject);

    /// <summary> Get number of total sent messages since initialization. </summary>
    public uint TotalSent => sent_messages_count(CommObject);

    /// <summary> Get last error summary. </summary>
    public string LastError {
      get {
        StringBuilder str = new StringBuilder(1024);
        last_error(CommObject, str, 1024);
        return str.ToString();
      }
    }

    #endregion public properties



    /// <summary> Returns new InterprocessCommunication object. </summary>
    public InterprocessCommunication()
    {
      CommObject = create_comm_object();
    }

    /// <summary> Close all connections and destroy the object. </summary>
    public void Dispose()
    {
      destroy_comm_object(CommObject);
    }

    /// <summary> Initialize InterprocessCommunication object given name. </summary>
    /// <param name="ProcessName">Specifies the desired process name. Name cannot include '/'.</param>
    /// <returns>bool indiciating success of the initialization. </returns>
    public bool Initialize(string ProcessName)
    {
      return initialize(CommObject, ProcessName);
    }

    /// <summary> Sets the maximum message size allowed to be sent this this communication object. </summary>
    /// <param name="size">The size parameter. Must be a multiple of 4096 (4KB). </param>
    /// <returns>Boolean indicating whether or not the value was set.</returns>
    public bool SetMaximumMessageSize(uint size)
    {
      return set_max_msg_size(CommObject, size);
    }

    /// <summary> Close all handles and connections. </summary>
    public void Close()
    {
      close(CommObject);
    }

    /// <summary> Post message for another process. </summary>
    /// <param name="Destination">Name of the target process.</param>
    /// <param name="Command">Message command.</param>
    /// <param name="Body">Message body.</param>
    public void PostMessage(string Destination, string Command, string Body)
    {
      post_message(CommObject, Destination, Command, Body);
    }

    /// <summary> Get message from the received messages. </summary>
    /// <returns>XML string represetntation of the received message.</returns>
    public string GetMessage()
    {
      StringBuilder str = new StringBuilder((int)MaximumMessageSize);
      get_message(CommObject, str, MaximumMessageSize);
      return str.ToString();
    }

    /// <summary> Get control message from the communication object. </summary>
    /// <returns>XML string representation of the control message.</returns>
    public string GetControlMessage()
    {
      StringBuilder str = new StringBuilder((int)MaximumMessageSize);
      get_control_message(CommObject, str, MaximumMessageSize);
      return str.ToString();
    }


    // ================================================================================
    // ---------------------------< testing functions area >---------------------------
    // ================================================================================

#if TEST_CS_SM_IPC

    static void PrintTitle(string str)
    {
      int totalDashes = 80 - str.Length - 4;
      int preDashes, postDashes;
      if (totalDashes % 2 == 1)
      {
        preDashes = totalDashes / 2 + 1;
        postDashes = totalDashes / 2;
      }
      else
      {
        preDashes = totalDashes / 2;
        postDashes = preDashes;
      }
      Console.Write("\r\n\r\n ================================================================================");
      Console.Write("\r\n ");
      for (int i = 0; i < preDashes; i++) Console.Write("-");
      Console.Write("< ");
      Console.Write(str);
      Console.Write(" >");
      for (int i = 0; i < postDashes; i++) Console.Write("-");
      Console.Write("\r\n ================================================================================\r\n");
    }

    static void PrintTerminatingTitle()
    {
      Console.Write("\r\n\r\n ================================================================================");
      Console.Write("\r\n ================================================================================\r\n");
    }

    static void DemoSendNormalMessages()
    {
      PrintTitle("Demo: Sending 10 Normal Messages");
      InterprocessCommunication comA = new InterprocessCommunication();
      InterprocessCommunication comB = new InterprocessCommunication();

      Console.Write("\r\n  Initializing IPC on 'CommunicationA'");
      if (!comA.Initialize("CommunicationA"))
      {
        Console.Write("\r\n  IPC on 'CommunicationA' failed.\r\n  Last Error: {0}", comA.LastError);
        return;
      }
      Console.Write("\r\n  Successfully initialized IPC on 'CommunicationA'");

      Console.Write("\r\n  Initializing IPC on 'CommunicationB'");
      if (!comB.Initialize("CommunicationB"))
      {
        Console.Write("\r\n  IPC on 'CommunicationB' failed.\r\n  Last Error: {0}", comB.LastError);
        return;
      }
      Console.Write("\r\n  Successfully initialized IPC on 'CommunicationB'");

      Console.Write("\r\n\r\n  Sending 5 messages from 'CommunicationA' to 'CommunicationB' and vice versa");
      for(int i=0; i<5; i++)
      {
        comA.PostMessage("CommunicationB", "Command", "Message #" + (i + 1));
        comB.PostMessage("CommunicationA", "Command", "Message #" + (i + 1));
      }

      IPCMessage msg;
      for(int i=0; i<5; i++)
      {
        msg = IPCMessage.FromString(comA.GetMessage());
        msg.Print();
        msg = IPCMessage.FromString(comB.GetMessage());
        msg.Print();
      }

      Console.Write("\r\n\r\n  Closing 'CommunicationA' ... "); comA.Dispose(); Console.Write("Closed");
      Console.Write("\r\n  Closing 'CommunicationB' ... "); comB.Dispose(); Console.Write("Closed");
      Console.Write("\r\n\r\n  Sending 10 normal messages demonstration successfully finished.");
      PrintTerminatingTitle();
    }


    static void PrintDemoPerformanceTitle(int MessageCount, int BodySize, 
      ushort DisplayMsgDegree, ushort DisplayOverallDegree)
    {
      int msgSize = BodySize + 80, allSize = MessageCount * (80 + BodySize);
      for (ushort div = 0; div < DisplayMsgDegree; div++) msgSize = (int)Math.Round((double)msgSize/1024);
      for (ushort div = 0; div < DisplayOverallDegree; div++) allSize = (int)Math.Round((double)allSize / 1024);

      string msgSizeID = "Byte", allSizeID = "Byte";

      if (DisplayMsgDegree == 1) msgSizeID = "KB";
      else if (DisplayMsgDegree == 2) msgSizeID = "MB";
      else if (DisplayMsgDegree == 3) msgSizeID = "GB";

      if (DisplayOverallDegree == 1) allSizeID = "KB";
      else if (DisplayOverallDegree == 2) allSizeID = "MB";
      else if (DisplayOverallDegree == 3) allSizeID = "GB";

      string displaySizeMsg = msgSize.ToString() + msgSizeID;
      string displaySizeAll = allSize.ToString() + allSizeID;

      string title = "Sending " + MessageCount.ToString() + " Messages. Size: ~" +
        displaySizeMsg + " each, ~" + displaySizeAll + " overall";

      PrintTitle(title);
    }

    static void DemoPerformance(int MessageCount, int BodySize, 
      ushort DisplayMsgDegree=0, ushort DisplayOverallDegree = 0)
    {
      PrintDemoPerformanceTitle(MessageCount, BodySize, DisplayMsgDegree, DisplayOverallDegree);
      InterprocessCommunication comA = new InterprocessCommunication();
      InterprocessCommunication comB = new InterprocessCommunication();
      DateTime start, end;

      Console.Write("\r\n  IPC on 'CommunicationB' will be receiving {0} messages with bodies of size: {1}", MessageCount, BodySize);

      uint pageSize = (uint)BodySize + 80;
      pageSize = 4096 * (pageSize / 4096 + 1);
      Console.Write("\r\n  Setting the message queue size in 'CommunicationB' to {0} (equivalent to {1} Windows Pages)", pageSize, pageSize / 4096);
      comB.SetMaximumMessageSize(pageSize);

      Console.Write("\r\n  Initializing IPC on 'CommunicationA'");
      if (!comA.Initialize("CommunicationA"))
      {
        Console.Write("\r\n  IPC on 'CommunicationA' failed.\r\n  Last Error: {0}", comA.LastError);
        return;
      }
      Console.Write("\r\n  Successfully initialized IPC on 'CommunicationA'");

      Console.Write("\r\n  Initializing IPC on 'CommunicationB'");
      if (!comB.Initialize("CommunicationB"))
      {
        Console.Write("\r\n  IPC on 'CommunicationB' failed.\r\n  Last Error: {0}", comB.LastError);
        return;
      }
      Console.Write("\r\n  Successfully initialized IPC on 'CommunicationB'");

      comA.WaitPeriod = 1000;
      comB.WaitPeriod = 1000;

      StringBuilder str = new StringBuilder();
      for (int i = 0; i < BodySize; i++) str.Append("A");
      string msgBody = str.ToString();

      start = DateTime.Now;
      Console.Write("\r\n\r\n  {0} - Sending {1} msgs. Body size: '{2}'. From 'A' to 'B'",
        start.ToString("[MM/dd/yyyy HH:mm:ss.fff]"), MessageCount, BodySize);

      for (int i = 0; i < MessageCount; i++)
        comA.PostMessage("CommunicationB", "Performance", msgBody);

      SpinWait.SpinUntil(new Func<bool>(() => { return comA.TotalSent >= MessageCount; }));
      end = DateTime.Now;
      Console.Write("\r\n  {0} - Sent {1} msgs. Body size: '{2}'. From 'A' to 'B'", 
        end.ToString("[MM/dd/yyyy HH:mm:ss.fff]"), MessageCount, BodySize);

      Console.Write("\r\n\r\n  Total time to send {0} messages: {1} seconds", MessageCount, (end - start).TotalSeconds);
      Console.Write("\r\n  Average time per message: {0} microseconds", (end.Ticks - start.Ticks) / (10 * MessageCount));
      long totalSizeinKB = MessageCount * (80 + BodySize) / 1024;
      Console.Write("\r\n  Average time per 1KB: {0} microseconds", (end.Ticks - start.Ticks) / (10 * totalSizeinKB));

      Console.Write("\r\n\r\n  Closing 'CommunicationA' ... "); comA.Dispose(); Console.Write("Closed");

      Console.Write("\r\n  Closing 'CommunicationB' ... "); comB.Dispose(); Console.Write("Closed");
      Console.Write("\r\n\r\n  Demonstration successfully finished.");
      PrintTerminatingTitle();
      GC.Collect(); Thread.Sleep(1000);
    }


    static void Main(string[] args)
    {
      Console.Title = "Shared Memory -- Interprocess Communication Demo";
      try
      {
        // normal communication test
        DemoSendNormalMessages();

        // performance tests
        DemoPerformance(100000, 100, DisplayOverallDegree: 2);
        DemoPerformance(100000, 520, DisplayOverallDegree: 2);
        DemoPerformance(10000, 5030, DisplayMsgDegree: 1, DisplayOverallDegree: 2);
        DemoPerformance(5000, 51100, DisplayMsgDegree: 1, DisplayOverallDegree: 2);
        DemoPerformance(50, 5242795, DisplayMsgDegree: 2, DisplayOverallDegree: 2);
        DemoPerformance(25, 52428700, DisplayMsgDegree: 2, DisplayOverallDegree: 2);
        DemoPerformance(5, 104857500, DisplayMsgDegree: 2, DisplayOverallDegree: 2);
        DemoPerformance(1, 1073741740, DisplayMsgDegree: 3, DisplayOverallDegree: 3);
      }catch(Exception ex)
      {
        Console.Write("\r\n  Unexpected error occurred.\r\n  Details: {0}", ex.Message);
        Console.Write("\r\n  Terminating...");
        return;
      }
    }
#endif
  }
}
