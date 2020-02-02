/////////////////////////////////////////////////////////////////////////////
//  Receiver.cs - demonstrate receiver using WCF environment			         //
//  ver 2.0                                                                //
//  Language:     C#, VS 2015                                              //
//  Platform:     Dell Inspiron 5520, Windows 10 Professional              //
//  Application:  Project for CSE681 - Software Modeling & Analysis				 //
//  Author:       Ammar Salman, ECEE, Syracuse University			             //
//                (313) 788-4694, assalman@syr.edu	  	                   //
/////////////////////////////////////////////////////////////////////////////
/*
 *   Module Operations
 *   -----------------
 *   This module demonstrates receiving operations. This is a general class
 *   that can be used by the Repository, TestHarness and Client where each
 *   one of them specifies the required Uri and hosts its own service. The
 *   binding type we used is WSHttpBinding as it is fast enough, secure and 
 *   insures good message transmission.
 * 
 *   Public Interface
 *   ----------------
 *   Receiver receiver = new Receiver();
 *   receiver.CreateCommHost(new Uri("http://localhost:9090/IComm/Receiver"));
 *   receiver.CreateStreamingHost(new Uri("http://localhost:9090/IStreamService/Receiver"));
 *   receiver.SetFilesPath(".\\Libraries");
 *   receiver.SetResultsPath(".\\Results");
 *   receiver.GetFilesPath();
 *   receiver.GetResultsPath();
 *   Message msg = receiver.GetMessage();
 *   receiver.PostMessage(msg);
 *   receiver.UploadFile(file);
 *   receiver.DownloadFile(file);
 *   
 *   BlockingQueue<string> bQ = new BlockingQueue<string>();
 *   bQ.enQ(msg);
 *   string msg = bQ.deQ();
 * 
 */
/*
 *   Build Process
 *   -------------
 *   - Required files:   Receiver.cs
 *   - Compiler command: csc Receiver.cs
 * 
 *   Maintenance History
 *   -------------------
 *   ver 2.0 : 12 November 2016
 *		 - Added support for file streaming
 *   ver 1.0 : 11 November 2016
 *     - first release
 *     - Support for normal message communication
 * 
 */

using System;
using System.ServiceModel;
using BlockingQueue;
using System.Threading;
using System.IO;
using System.Diagnostics;

namespace Communication {

	[ServiceBehavior(InstanceContextMode = InstanceContextMode.PerSession)]
	public class Receiver : IComm, IStreamService {
		// Fields and properties declarations
		#region
		/// <summary>Static Blocking Queue shared by all clients</summary>
		static BlockingQ<Message> Queue;
		/// <summary>Agreggated ServiceHost</summary> 
		ServiceHost commHost, streamingHost;
		/// <summary>Gets the communication address</summary>
		public string CommAddress { get; private set; }
		/// <summary>Gets the streaming address</summary>
		public string StreamAddress { get; private set; }

		/* 
		 * The files/results directories should be static if we want to
		 * allow the aggregator classes to change them. Same as the queue 
		 */
		/// <summary>Gets or sets libraries storage path</summary>
	  static string FilesPath { get; set; } = "..\\Libraries";
		/// <summary>Gets or sets results storage path</summary>
		static string ResultsPath { get; set; } = "..\\Results";
		#endregion


		// -----< Receiver Constructors >-----
		/// <summary>Returns an instance of Receiver</summary> 
		public Receiver() {
			/* 
			 * This check is important because whenever a sender creates
			 * channel/proxy it will automatically try creating an instance
			 * of the receiver. This way we can insure all senders send to
			 * the same Queue
			 */
			if (Queue == null)
				Queue = new BlockingQ<Message>();
			commHost = null;
			streamingHost = null;
		}
		
		public string GetFilesPath() { return FilesPath; }
		public string GetResultsPath() { return ResultsPath; }
		public void SetFilesPath(string s) { FilesPath = s; }
		public void SetResultsPath(string s) { ResultsPath = s; }

		// Create hosts region
		#region
		// -----< Create Communication Host >-----
		/// <summary>Prepares the ServiceHost to accept connections on the specified Uri</summary>
		/// <param name="uri">Address of the host</param>
		public void CreateCommHost(Uri uri) {
			CommAddress = uri.AbsoluteUri;
			WSHttpBinding binding = new WSHttpBinding();

			commHost = new ServiceHost(typeof(Receiver), uri);
			commHost.AddServiceEndpoint(typeof(IComm), binding, uri);
			commHost.Open();
		}

		// -----< Create Streaming Host >-----
		/// <summary>Prepares the ServiceHost to accept connections on the specified Uri</summary>
		/// <param name="uri">Address of the host</param>
		public void CreateStreamHost(Uri uri) {
			StreamAddress = uri.AbsoluteUri;
			BasicHttpBinding binding = new BasicHttpBinding();
			binding.TransferMode = TransferMode.Streamed;
			binding.MaxReceivedMessageSize = 5368709120;

			streamingHost = new ServiceHost(typeof(Receiver), uri);
			streamingHost.AddServiceEndpoint(typeof(IStreamService), binding, uri);
			streamingHost.Open();
		}
		#endregion

		// Communication methods region
		#region
		// -----< Get Message >-----
		///<summary>Returns retreived message from senders</summary>
		[OperationBehavior]
		public Message GetMessage() {
			return Queue.deQ();
		}

		// -----< Post Message >-----
		/// <summary>Called by senders to post messages to the queue</summary>
		/// <param name="msg">Message type that contains command and message body</param>
		[OperationBehavior(TransactionAutoComplete = true)]
		public void PostMessage(Message msg) {
			Queue.enQ(msg);
		}
		#endregion

		// Streaming methods region
		#region
		[OperationBehavior]
		public void UploadFile(FileTransferMessage msg) {
			Stopwatch timer = new Stopwatch();
			timer.Start();
			string filename;
			switch (msg.Type) {
				case FileTransferMessage.FileType.Library:
					filename = Path.Combine(FilesPath, msg.Filename);
					break;
				case FileTransferMessage.FileType.Result:
					filename = Path.Combine(ResultsPath, msg.Filename);
					break;
				case FileTransferMessage.FileType.ConnectionTest:
					filename = Path.Combine(Directory.GetCurrentDirectory(), msg.Filename);
					break;
				default:
					return;
			}
			
			int BlockSize = 1024; byte[] block = new byte[BlockSize];
			if (!Directory.Exists(FilesPath))
				Directory.CreateDirectory(FilesPath);
			if (!Directory.Exists(ResultsPath))
				Directory.CreateDirectory(ResultsPath);
			using (FileStream outputStream = new FileStream(filename, FileMode.Create)) {
				int bytesRead;
				while (true) {
					bytesRead = msg.TransferStream.Read(block, 0, BlockSize);
					if (bytesRead > 0)
						outputStream.Write(block, 0, bytesRead);
					else
						break;
				}
			}
			timer.Stop();
		}

		[OperationBehavior]
		public Stream DownloadFile(string Filename, FileTransferMessage.FileType Type) {
			string sfilename;
			switch (Type) {
				case FileTransferMessage.FileType.Library:
					sfilename = Path.Combine(FilesPath, Filename);
					break;
				case FileTransferMessage.FileType.Result:
					sfilename = Path.Combine(ResultsPath, Filename);
					break;
				case FileTransferMessage.FileType.ConnectionTest:
					sfilename = Path.Combine(Directory.GetCurrentDirectory(), Filename);
					break;
				default:
					return null;
			}
			FileStream outStream = null;
			if (File.Exists(sfilename)) {
				Console.Write("\n  Sending File \"{0}\" ... ", Filename);
				try {
					outStream = new FileStream(sfilename, FileMode.Open);
				} catch (Exception ex) {
					Console.Write("Failed\n\tDetails: {0}\n", ex.Message);
				}
			} else {
				Console.Write("\n  Cannot find file \"{0}\" in \"{1}\"", Filename, Directory.GetParent(sfilename));
				throw new FileNotFoundException("Failed to find \"" + Filename + "\"");
			}
			return outStream;
		}
		#endregion

		// Test methods region
		#region
		// -----< Thread Processing >-----
		/// <summary>
		/// Conteiniously retreives messages from the queue. Override
		/// this method to serve specific needs using the Receiver.
		/// </summary>
		protected virtual void ThreadProc() {
			while (true) {
				Message msg = GetMessage();
				string cmdStr = "";
				// Prepare output string to print
				switch (msg.command) {
					case Message.Command.ResultRequest:
						cmdStr = "Result Request - ";
						break;
					case Message.Command.TestRequest:
						cmdStr = "Test Request - ";
						break;
					default:
						cmdStr = "Unknown command - ";
						break;
				}
				Console.Write("\n  Received: {0}\t{1}", cmdStr, msg.Body);
				if (msg.Body == "quit")
					break;
			}
		}

		/// <summary>Test stub for Receiver class</summary>
#if (TEST_RECEIVER)
		static void Main(string[] args) {
			Console.Write("\n  Creating Receiver Instance ");
			Console.Write("\n ----------------------------\n");

			Receiver receiver = new Receiver();
			receiver.CreateCommHost(new Uri("http://localhost:9090/IComm/Receiver"));
			receiver.CreateStreamHost(new Uri("http://localhost:9091/IStream/Receiver"));
			// Initialize receiver thread and start it
			Thread thread = new Thread(receiver.ThreadProc);
			thread.Start();

			// Prepare and send test message
			Message msg = new Message();
			msg.Body = "Test message from Receiver to itself";
			receiver.PostMessage(msg);
			
			thread.Join();
		}
#endif

	#endregion
	}
}
