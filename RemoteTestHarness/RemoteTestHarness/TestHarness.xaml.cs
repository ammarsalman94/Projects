/////////////////////////////////////////////////////////////////////////////
//  TestHarness.xaml.cs - TestHarness GUI and processing									 //
//  ver 1.0                                                                //
//  Language:     C#, VS 2015                                              //
//  Platform:     Dell Inspiron 5520, Windows 10 Professional              //
//  Application:  Project for CSE681 - Software Modeling & Analysis				 //
//  Author:       Ammar Salman, ECEE, Syracuse University			             //
//                (313) 788-4694, assalman@syr.edu	  	                   //
/////////////////////////////////////////////////////////////////////////////
/*
 *   Module Operations
 *   -----------------
 *   This module demonstrates the TestHarness process with a user-friendly 
 *   informative GUI. The requests should be sent from the repository in
 *   order for this to work. 
 * 
 *   Public Interface
 *   ----------------
 *   It only holds one public method as the rest cannot be accessed except
 *   whithin the class itself because it is an interactive application
 *   TestHarness testHarness = new TestHarness();
 *   
 * 
 */
/*
 *   Build Process
 *   -------------
 *   - Required files:   TestHarness.xaml.cs
 *   - Compiler command: csc TestHarness.xaml.cs
 * 
 *   Maintenance History
 *   -------------------
 *   ver 1.0 : 16 November 2016
 *     - first release
 * 
 */

using System;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.ComponentModel;
using System.Threading;
using System.IO;
using System.Runtime.InteropServices;
using System.Runtime.Remoting;
using System.Diagnostics;
using System.Collections.Generic;
using BlockingQueue;
using Communication;
using XMLProcessing;

namespace RemoteTestHarness {
	/// <summary>Test Harness GUI and processing</summary>
	public partial class TestHarness : Window {
		/* ----------------------< Fields & Properties Declaration Region >--------------------- */
		#region
			
		/// <summary>Holds the host that receives messages from repository</summary>
		Receiver receiver;
		/// <summary>Sends communication stream messages to the repository </summary>
		Sender sender;

		/// <summary>Holds the test requests to process as XML strings</summary>
		BlockingQ<Message> TestRequests;
		/// <summary>Holds the messages to send to the repository</summary>
		BlockingQ<Message> sendingQueue;

		/// <summary> The receiver thread </summary>
		Thread mReceiver;
		/// <summary> The sender thread</summary>
		Thread mSender;

		/// <summary>Test requests processing threads</summary>
		List<Thread> RunningThreads;

		/// <summary>Specifies the number of cores on the running machine</summary>
		private int CoreCount { get; }

		#endregion
		/* -------------------< End of Fields & Properties Declaration Region >----------------- */


		/* --------------------------< Initialization Methods Region >-------------------------- */
		#region

		/// <summary>Imports console allocation command</summary>
		[DllImport("Kernel32.dll")]
		public static extern void AllocConsole();

		/// <summary>Initializes Test Harness server and prepares it to work</summary>
		public TestHarness() {
			InitializeComponent();
			AllocConsole();
			// Get the number of cores on the running machine
			CoreCount = Environment.ProcessorCount;
			Console.Write("\n  Found {0} cores on current machine", CoreCount);
			lblRunningTests.Content += " (Max: " + CoreCount + ")";

			InitializeTH();
			InitializeThreads();
		}

		/// <summary>Initializes Test Harness</summary>
		void InitializeTH() {
			Console.Title = "Test Harness Server";
			Console.Write("Test Harness - Console View".Title());

			// Initialize queues
			TestRequests = new BlockingQ<Message>();
			sendingQueue = new BlockingQ<Message>();

			if (!Directory.Exists(".\\TestHarness\\Results"))
				Directory.CreateDirectory(".\\TestHarness\\Results");

			receiver = new Receiver();
			receiver.SetFilesPath(".\\TestHarness\\Temp_Libraries");
			receiver.SetResultsPath(".\\TestHarness\\Results");
			sender = new Sender();
			sender.FilesPath = ".\\TestHarness\\Temp_Libraries";
			sender.ResultsPath = ".\\TestHarness\\Results";

			// Initialize communication and streaming hosts
			for (int i = 7000; i < 8000; i++)
				try {
					receiver.CreateCommHost(new Uri("http://localhost:" + i + "/IComm/TestHarness"));
					break;
				} catch (Exception) {
					Console.Write("\n  Failed to connect on port {0}", i);
				}
			for (int i = 7000; i < 8000; i++)
				try {
					receiver.CreateStreamHost(new Uri("http://localhost:" + i + "/IService/TestHarness"));
					break;
				} catch (Exception) {
					Console.Write("\n  Failed to connect on port {0}", i);
				}
			// Start the receiver thread
			mReceiver = new Thread(ReceiveMessages);
			mReceiver.Start();
			// create sender channels that connect to the repository
			// this starts the sender thread if successful
			CreateSenderChannels(8000, 8001);
			mSender = new Thread(SendMessages);
			mSender.Start();
			RunningThreads = new List<Thread>();
		}

		/// <summary>Initializes the running threads</summary>
		void InitializeThreads() {
			for (int i = 0; i < CoreCount; i++) {
				Thread t = new Thread(() => { ProcessTestRequest("Thread" + i.ToString()); });
				RunningThreads.Add(t);
				t.Start();
				Thread.Sleep(100);
			}
		}

		#endregion
		/* -----------------------< End of Initialization Methods Region >---------------------- */



		/* ----------------------------< Processing Threads Region >---------------------------- */
		#region
		/// <summary>The thread that processes received messages</summary>
		void ReceiveMessages() {
			while (true) {
				Message msg = receiver.GetMessage();
				if (msg.Body.ToLower().Equals("quit")) break;
				Console.Write("\n  Received message of type {0} from repository", msg.command);
				switch (msg.command) {
					case Message.Command.TestRequest:
						PrepareTestRequest(msg);
						break;
					default:
						Console.Write("\n  Unrecognized message type \"{0}\"", msg.command);
						break;
				}
			}
		}

		/// <summary>The thread that sends communication messages</summary>
		void SendMessages() {
			while (true) {
				Message msg = sendingQueue.deQ();
				if (msg.Body.ToLower().Equals("quit")) break;
				try {
					Console.Write("\n  Sending message of type {0} to repository ... ", msg.command);
					sender.PostMessage(msg, msg.Destination);
					Console.Write("Sent successfully");
				} catch (Exception) {
					Console.Write("Failed");
				}
			}
		}

		/// <summary>Processes test requests each in its AppDomain</summary>
		void ProcessTestRequest(string Name) {
			Console.Write("\n  {0} intitiated", Name);
			XMLReader xReader = new XMLReader();
			while (true) {
				Message msg  = TestRequests.deQ();
				// start timer
				Stopwatch watch = new Stopwatch(); watch.Start();

				// if quit enqueue message again so other threads quit when they see it
				if (msg.Body.ToLower().Equals("quit")) {
					TestRequests.enQ(msg); break;
				}
				
				xReader.ProcessXMLString(msg.Body);

				string s = xReader.Author + " - " + xReader.TestRequest;
				Dispatcher.BeginInvoke(new Action(() => { lstBoxPendingTests.Items.Remove(s);
					lstBoxRunningTests.Items.Add(Name+": "+s);}));

				string ResultFile =Path.Combine(sender.ResultsPath, xReader.Author +
					DateTime.Now.ToString("_MM_dd_yyyy_-_hh_mm_ss_") + xReader.TestRequest + ".log");
				StringBuilder Log = new StringBuilder();
				Log.Append("Author: "+xReader.Author+"\nTests:");
				// downloads the required files from repository
				DownloadTestFiles(ref xReader, Name, ref Log);

				StringBuilder innerLog = new StringBuilder();

				// start testing test drivers
				TestFiles(ref xReader, ref Log, ref innerLog,  Name);
				Log.Append("\n\n"+innerLog.ToString());

				// stop timer
				watch.Stop();
				
				// finish log
				Log.Append("\n\nElapsed Time: " + watch.Elapsed.Seconds + " seconds");
				// prepare results file
				File.WriteAllLines(ResultFile, Log.ToString().Split('\n'));

				//send results file to repository
				sender.UploadFile(ResultFile, FileTransferMessage.FileType.Result, "Repository");
				PrepareResultsMessage(msg.Owner, ResultFile);
				DeleteTestFiles(Name);

				Dispatcher.BeginInvoke(new Action(() => {
					lstBoxRunningTests.Items.Remove(Name + ": "+s);
					lstBoxFinishedTests.Items.Add(s);}));
			}
		}
		#endregion
		/* -------------------------< End of Processing Threads Region >------------------------ */


		/* -------------------------------< Test Methods Region >------------------------------- */
		#region
		
		/// <summary>Prepares test request for the threads to process</summary>
		/// <param name="msg">Specifies the test request in its body property</param>
		void PrepareTestRequest(Message msg) {
			XMLReader xReader = new XMLReader();
			xReader.ProcessXMLString(msg.Body);
			string s = xReader.Author + " - " + xReader.TestRequest;
			Dispatcher.BeginInvoke(new Action(()=> { lstBoxPendingTests.Items.Add(s); }), null);
			TestRequests.enQ(msg);
		}

		/// <summary>Downloads required test files to thread's folder and appends log</summary>
		/// <param name="xReader">Reference to the XMLReader involved</param>
		/// <param name="Name">The calling thread name</param>
		/// <param name="Log">Reference to log to append on it</param>
		void DownloadTestFiles(ref XMLReader xReader, string Name, ref StringBuilder Log) {
			// get all needed libraries before processing any tests
			foreach (var Test in xReader.TestDrivers) {
				Log.Append("\n\t\"" + Test.Key + "\"\n\t\tTest Drivers:");
				foreach (string TestDriver in Test.Value) {
					sender.DownloadLibrary(TestDriver, "Repository", ".\\" + Name);
					Log.Append("\n\t\t\t" + TestDriver);
				}
				Log.Append("\n\t\tLibraries:");
				foreach (string Library in xReader.Libraries[Test.Key]) {
					sender.DownloadLibrary(Library, "Repository", ".\\" + Name);
					Log.Append("\n\t\t\t" + Library);
				}
			}
			Dispatcher.BeginInvoke(new Action(() => {
				string[] names = Directory.GetFiles(Name, "*.dll");
				foreach (string s in names)
					lstBoxTempFiles.Items.Add(s);
			}));
		}

		/// <summary>Deletes temporary files loaded for the test request</summary>
		/// <param name="Name">Specifies the calling thread name</param>
		void DeleteTestFiles(string Name) {
			string[] names = Directory.GetFiles(Name, "*.dll");
			foreach (string s in names) {
				File.Delete(s);
			}
			Dispatcher.BeginInvoke(new Action(() => {
				foreach (string s in names)
					lstBoxTempFiles.Items.Remove(s);
			}));
		}

		/// <summary>Tests the Test Drivers specified by the calling thread</summary>
		/// <param name="xReader">Reference to the XMLReader involved</param>
		/// <param name="Log">Reference to the log to append</param>
		/// <param name="innerLog">Reference to the inner log. This helps orginizing the results file</param>
		void TestFiles(ref XMLReader xReader, ref StringBuilder Log, ref StringBuilder innerLog, string Name) {
			Log.Append("\n\nResults:");
			foreach (var Test in xReader.TestDrivers) {
				Log.Append("\n\t" + Test.Key + ":");
				innerLog.Append("\n" + Test.Key + ":\n\n");
				foreach (string TestDriver in Test.Value) {
					// load the file in a child AppDomain. This is useful as we need to unload the
					// assembly and that cannot be done if it was loaded directly
					AppDomain ChildDomain = AppDomain.CreateDomain(Name + "ChildDomain");
					ChildDomain.Load("TestAppDomain");
					ObjectHandle objH = ChildDomain.CreateInstance("TestAppDomain", "RemoteTestHarness.TestAppDomain");
					TestAppDomain Tester = (TestAppDomain)objH.Unwrap();
					Tester.path = Path.Combine(".\\" + Name, TestDriver);
					// perform testing
					Tester.execute();
					Log.Append("\n\t\t" + Tester.getLog());
					innerLog.Append(Tester.getLog(true) + "\n");
					// unload the assembly to set the assembly file free
					AppDomain.Unload(ChildDomain);
				}
			}
		}

		/// <summary>Prepares result message to inform the repository that 
		/// test is finished and result is present</summary>
		/// <param name="Owner">Specifies the Client who issued the test request</param>
		/// <param name="ResultFile">Specifies the result filename</param>
		void PrepareResultsMessage(string Owner, string ResultFile) {
			Message msg = new Message();
			msg.Owner = Owner;
			msg.Source = receiver.CommAddress;
			msg.Destination = "Repository";
			msg.command = Message.Command.Results;
			msg.Body = Path.GetFileName(ResultFile);
			sendingQueue.enQ(msg);
		}

		#endregion
		/* ---------------------------< End of Test Methods Region >---------------------------- */


		/* -------------------------------< Main Methods Region >------------------------------- */
		#region
		/// <summary>Creates sender channels to bind to repository</summary>
		/// <param name="port1">Specifies communication channel port</param>
		/// <param name="port2">Specifies streaming channel port</param>
		void CreateSenderChannels(int port1, int port2) {
			try {
				sender.CreateCommChannel("http://localhost:" + port1
					+ "/IComm/Repository", "Repository");
				Message msg = new Message();
				msg.Owner = "TestHarness";
				msg.Source = receiver.CommAddress;
				msg.command = Message.Command.ConnectionTest;
				Console.Write("\n  Sending test message to repository ... ");
				sender.PostMessage(msg, "Repository");
				Console.Write("Sent successfully");
			} catch (Exception) {
				Console.Write("Failed\n  Failed to create communication channel to Repository"
					+ " on port {0}", port1);
			}
			try {
				sender.CreateStreamingChannel("http://localhost:" + port2
					+ "/IStream/Repository", "Repository");
				File.Create("TestFile.test").Close();
				Thread.Sleep(100);
				Console.Write("\n  Sending test file to repository ... ");
				sender.UploadFile("TestFile.test", FileTransferMessage.
					FileType.ConnectionTest, "Repository");
				Console.Write("Sent successfully");
			} catch (Exception ex) {
				Console.Write("Failed\n  Failed to create streaming channel to Repository"
					+ " on port {0}", port2);
				Console.Write("\n  Details: {0}", ex.Message);
				return;
			}
		}

		/// <summary>Creates sender channels to bind to repository</summary>
		/// <param name="commUri">Specifies repository's communication address</param>
		/// <param name="streamUri">Specifies repository's streaming address</param>
		void CreateSenderChannels(string commUri, string streamUri) {
			try {
				sender.CreateCommChannel(commUri, "Repository");
				Message msg = new Message();
				msg.Owner = "TestHarness";
				msg.Source = receiver.CommAddress;
				msg.command = Message.Command.ConnectionTest;
				Console.Write("\n  Sending test message to repository ... ");
				sender.PostMessage(msg, "Repository");
				Console.Write("Sent successfully");
			} catch (Exception) {
				Console.Write("Failed\n  Failed to create communication channel to Repository"
					+ " on address {0}", commUri);
			}
			try {
				sender.CreateStreamingChannel(streamUri, "Repository");
				File.Create("TestFile.test").Close();
				Thread.Sleep(100);
				Console.Write("\n  Sending test file to repository ... ");
				sender.UploadFile("TestFile.test", FileTransferMessage.
					FileType.ConnectionTest, "Repository");
				Console.Write("Sent successfully");
			} catch (Exception ex) {
				Console.Write("Failed\n  Failed to create streaming channel to Repository"
					+ " on address {0}", streamUri);
				Console.Write("\n  Details: {0}", ex.Message);
				return;
			}
		}

		#endregion
		/* ---------------------------< End of Main Methods Region >---------------------------- */



		/* -------------------------------< Main Buttons Region >------------------------------- */
		#region
		/// <summary>Specifies action for the Exit MenuItem</summary>
		private void btnExitItem_Click(object sender, RoutedEventArgs e) {
			Close();
		}

		/// <summary>Specifies action for MenuItem that connects to
		/// repository by specifying only port numbers</summary>
		private void btnConnectRep_Click(object sender, RoutedEventArgs e) {
			// initialize ports
			int port1 = 8000, port2 = 8001;
			// create form to set the ports
			System.Windows.Forms.Form prompt = new System.Windows.Forms.Form();
			prompt.Width = 500;
			prompt.Height = 70;
			prompt.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedToolWindow;
			prompt.Text = "Connect to Repository (ports)";
			// initialize form components
			System.Windows.Forms.Label textLabel =
				new System.Windows.Forms.Label() { Left = 5, Top = 5, Width = 80, Text = "Comm Port:" };
			System.Windows.Forms.TextBox txtPort1 =
				new System.Windows.Forms.TextBox() { Left = 90, Top = 5 };
			System.Windows.Forms.Label textLabe2 =
				new System.Windows.Forms.Label() { Left = 190, Width = 80, Top = 5, Text = "Streaming Port:" };
			System.Windows.Forms.TextBox txtPort2 =
				new System.Windows.Forms.TextBox() { Left = 280, Top = 5 };
			System.Windows.Forms.Button confirmation =
				new System.Windows.Forms.Button() { Text = "Confirm", Left = 400, Width = 80, Top = 3 };
			confirmation.Click += (send, ex) => {
				// get port1 value from form
				int.TryParse(txtPort1.Text, out port1);
				// get port2 value from form
				int.TryParse(txtPort2.Text, out port2);
				prompt.Close();
			};
			// add components to form
			prompt.Controls.Add(confirmation);
			prompt.Controls.Add(textLabel);
			prompt.Controls.Add(textLabe2);
			prompt.Controls.Add(txtPort1);
			prompt.Controls.Add(txtPort2);
			// show form for user
			prompt.ShowDialog();
			// try establishing connection to repository
			Console.Write("\n  Connecting to repository on ports: {0}, {1}", port1, port2);
			CreateSenderChannels(port1, port2);
		}

		/// <summary>Specifies action for MenuItem that connects
		/// to repository by specifying complete addresses</summary>
		private void btnConnRepAddresses_Click(object sender, RoutedEventArgs e) {
			string commUri = "", streamUri = "";
			// create form to set the ports
			System.Windows.Forms.Form prompt = new System.Windows.Forms.Form();
			prompt.Width = 500;
			prompt.Height = 100;
			prompt.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedToolWindow;
			prompt.Text = "Connect to Repository (addresses)";
			// initialize form components
			System.Windows.Forms.Label textLabel =
				new System.Windows.Forms.Label() { Left = 5, Top = 5, Width = 100, Text = "Comm Host:", TabIndex = 10 };
			System.Windows.Forms.TextBox txtCommUri =
				new System.Windows.Forms.TextBox() {
					Left = 120, Top = 5, Width = 250,
					Text = "http://localhost:8000/IComm/Repository", TabIndex = 1
				};
			System.Windows.Forms.Label textLabe2 =
				new System.Windows.Forms.Label() { Left = 5, Width = 100, Top = 35, Text = "Streaming Host:", TabIndex = 10 };
			System.Windows.Forms.TextBox txtStreamUri =
				new System.Windows.Forms.TextBox() {
					Left = 120, Top = 35, Width = 250,
					Text = "http://localhost:8001/IStream/Repository", TabIndex = 2
				};
			System.Windows.Forms.Button confirmation =
				new System.Windows.Forms.Button() { Text = "Confirm", Left = 390, Width = 80, Top = 33, TabIndex = 3 };
			confirmation.Click += (send, ex) => {
				commUri = txtCommUri.Text; streamUri = txtStreamUri.Text; prompt.Close();
			};
			// add components to form
			prompt.Controls.Add(confirmation);
			prompt.Controls.Add(textLabel);
			prompt.Controls.Add(textLabe2);
			prompt.Controls.Add(txtCommUri);
			prompt.Controls.Add(txtStreamUri);
			// show form for user
			prompt.ShowDialog();
			// try establishing connection to repository
			CreateSenderChannels(commUri, streamUri);
		}
		#endregion
		/* ----------------------------< End of Main Buttons Region >--------------------------- */


		/* ----------------------------< Accessories Region Region >---------------------------- */
		#region
		/// <summary>Overrides the OnClosing method which fires on Window closing
		/// The need to override is to ensure all the other threads are correctly
		/// terminated</summary>
		protected override void OnClosing(CancelEventArgs e) {
			receiver.PostMessage(new Message() { Body = "quit" });
			TestRequests.enQ(new Message() { Body = "quit" });
			sendingQueue.enQ(new Message() { Body = "quit" });
			if (mReceiver.IsAlive) mReceiver.Join();
			if (mSender.IsAlive) mSender.Join();
			foreach (Thread t in RunningThreads)
				if (t.IsAlive) t.Join();
			Environment.Exit(0);
		}
		#endregion
		/* -------------------------< End of Accessories Region Region >------------------------ */

	}

	/* ----------------------------< Extension Methods Region >---------------------------- */
	#region
	public static class ExtensionMethods {
		public static void PerformClick(this Button btn) {
			btn.RaiseEvent(new RoutedEventArgs(Button.ClickEvent));
		}

		public static string Title(this string s) {
			s = "\n  " + s + "\n ";
			s += new string('-', s.Length - 2);
			s += "\n";
			return s;
		}
	}
	#endregion
	/* -------------------------< End of Extension Methods Region >------------------------ */

}
