/////////////////////////////////////////////////////////////////////////////
//  Repository.xaml.cs - Repository GUI and processing										 //
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
 *   This module demonstrates the Repository GUI which offers all services for
 *   the repository. It can be used to:
 *   1. Forward test requests to TestHarness
 *   2. Send assemblies to TestHarness and Clients
 *   3. Get results from TestHarness
 *   4. Send results to Clients
 *   It supprts multi-threaded processing to send/receive messages/files.
 * 
 *   Public Interface
 *   ----------------
 *   It only holds one public method as the rest cannot be accessed except
 *   whithin the class itself because it is an interactive application
 *   RepositoryGUI client = new RepositoryGUI();
 *   
 * 
 */
/*
 *   Build Process
 *   -------------
 *   - Required files:   Repository.xaml.cs
 *   - Compiler command: csc Repository.xaml.cs
 * 
 *   Maintenance History
 *   -------------------
 *   ver 1.0 : 17 November 2016
 *     - first release
 * 
 */

using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows;
using System.Windows.Controls;
using BlockingQueue;
using Communication;
using System.Threading;
using System.IO;
using System.Runtime.InteropServices;
using System.Xml.Linq;
using System.ComponentModel;
using XMLProcessing;

namespace Repository {
	/// <summary>Repository GUI</summary>
	public partial class RepositoryGUI : Window {
		/* ----------------< Fields and properties declarations Region >---------------- */
		#region

		/// <summary>Holds the Repository host that receives messages
		/// from Clients and TestHarness</summary>
		Receiver receiver;
		/// <summary> The receiver thread </summary>
		Thread mReceiver;
		/// <summary> Sends data to the TestHarness and Clients </summary>
		Sender sender;
		/// <summary> The sender thread</summary>
		Thread mSender;
		
		/// <summary> The sending queue </summary>
		BlockingQ<Message> sendingQueue;

		/// <summary>Holds all library files in the repository</summary>
		List<string> FileList;
		/// <summary>Holds all result files in the repository </summary>
		List<string> ResultsList;

		/// <summary>Processes XML test requests</summary>
		XMLReader xReader;

		/// <summary>Holds records for all connected ends whether
		/// they were Clients or the Test Harness</summary>
		IDictionary<string, string> ConnectedEnds;
		/// <summary>Holds records for the connected streaming hosts</summary>
		IDictionary<string, string> ConnectedStreams;
		/// <summary>Holds the blocked messages of which the destination is unavailable</summary>
		IDictionary<string, List<Message>> BlockedMessages;
		/// <summary>Holds the blocked files of which the destination is unavailable</summary>
		IDictionary<string, List<FileTransferMessage>> BlockedFiles;

		/// <summary>Holds all received messages for history backup</summary>
		List<Message> ReceivedMessages;
		#endregion
		/* -------------< End of Fields and properties declarations Region >------------ */


		/* ----------------------< Initialization methods Region >---------------------- */
		#region
		/// <summary>Enables console view for event logging </summary>
		[DllImport("Kernel32")]
		public static extern void AllocConsole();

		/// <summary>Initialize repository and prepare it to work</summary>
		public RepositoryGUI() {
			InitializeComponent();
			// prepare the console
			AllocConsole();
			Console.Title = "Repository Server";
			Console.Write("Repository Console View".Title());
			txtPort1.Text = "8000";
			txtPort2.Text = "8001";
			// initialize files lists
			FileList = new List<string>();
			ResultsList = new List<string>();
			// initialize the sending queue
			sendingQueue = new BlockingQ<Message>();
			// initialize and set up the receiver
			receiver = new Receiver();
			receiver.SetFilesPath(".\\Repository\\Libraries");
			receiver.SetResultsPath(".\\Repository\\Results");
			btnCreateChannels.PerformClick();
			LibSearchRepository();
			ResSearchRepository();
			mReceiver = new Thread(ReceiveMessages);
			mReceiver.Start();

			sender = new Sender();
			mSender = new Thread(SendMessages);
			mSender.Start();
			ConnectedEnds = new Dictionary<string, string>();
			ConnectedStreams = new Dictionary<string, string>();
			BlockedMessages = new Dictionary<string, List<Message>>();
			BlockedFiles = new Dictionary<string, List<FileTransferMessage>>();
			xReader = new XMLReader();
			ReceivedMessages = new List<Message>();
		}
		#endregion
		/* ------------------< End of Initialization methods Region >------------------- */



		/* ---------------------------< Main methods Region >--------------------------- */
		#region
		/// <summary>Search set directory for assemblies</summary>
		private void LibSearchRepository() {
			FileList.Clear();
			lstBoxLibraries.Items.Clear();
			try {
				FileList.AddRange(Directory.GetFiles(receiver.GetFilesPath(), "*.dll"));
				FileList = FileList.Select(c => { c = Path.GetFileName(c); return c; }).ToList();
				foreach (string file in FileList)
					lstBoxLibraries.Items.Add(file);
			} catch (DirectoryNotFoundException) {
				Console.Write("\n  Current Libraries directory does not exist\n");
			}
		}

		/// <summary>Search set directory for results</summary>
		private void ResSearchRepository() {
			ResultsList.Clear();
			lstBoxResults.Items.Clear();
			try {
				ResultsList.AddRange(Directory.GetFiles(receiver.GetResultsPath(), "*.log"));
				ResultsList = ResultsList.Select(c => { c = Path.GetFileName(c); return c; }).ToList();
				foreach (string file in ResultsList) 
					lstBoxResults.Items.Add(file);
			} catch (DirectoryNotFoundException) {
				Console.Write("\n  Current Results directory does not exist\n");
			}
		}

		/// <summary>Sends file list to the corresponding client</summary>
		/// <param name="Client">Specifies where to send the message</param>
		private void SendFileList(string ClientID) {
			// prepare message body
			Message msg = new Message();
			msg.Source = receiver.CommAddress;
			msg.Destination = ClientID;
			msg.command = Message.Command.FileList;
			XDocument xdoc = new XDocument(new XElement("FilesList"));
			foreach (string s in FileList)
				xdoc.Element("FilesList").Add(new XElement("Filename", new XAttribute("value", Path.GetFileName(s))));
			msg.Body = xdoc.ToString();
			// put message to send
			sendingQueue.enQ(msg);
		}

		/// <summary>Processes XML test requests received from clients</summary>
		/// <param name="XML">Speicifies the XML body as string</param>
		private void ProcessXMLTestRequest(Message Msg) {
			Dispatcher.Invoke(() => { LibSearchRepository(); });
			Message msg = new Message();
			msg.Owner = Msg.Owner;
			msg.Source = receiver.CommAddress;
			if (!xReader.ProcessXMLString(Msg.Body)) {
				msg.Destination = Msg.Owner;
				msg.command = Message.Command.InvalidTestRequest;
				msg.Body = "Invalid XML content:\n" + Msg.Body;
				sendingQueue.enQ(msg);
				return;
			}
			
			// check each test in the request and see if all its files
			// exist in the repository
			string Body = ""; bool Invalid= false;
			foreach(var Test in xReader.TestDrivers) {
				Invalid = false;
				Body += "Test \"" + Test.Key + "\" following files are missing:";
				foreach (string TestDriver in Test.Value)
					if (!FileList.Contains(TestDriver)) {
						Body += "\n\t" + TestDriver; Invalid = true;
					}
				foreach (string Library in xReader.Libraries[Test.Key])
					if (!FileList.Contains(Library)) {
						Body += "\n\t" + Library; Invalid = true;
					}
				Body += "\n";
			}
			// if test request specifies non-existing files send client 
			// a message saying that the test request is invalid
			if (Invalid) {
				msg.Destination = Msg.Owner;
				msg.command = Message.Command.InvalidTestRequest;
				msg.Body = Body;
				sendingQueue.enQ(msg);
				return;
			}
			msg.Destination = "TestHarness";
			msg.command = Message.Command.TestRequest;
			msg.Body = Msg.Body;
			sendingQueue.enQ(msg);
		}

		/// <summary>Responds to client's request and sends the required test result </summary>
		/// <param name="Msg">Specifies the result request body</param>
		private void SendResult(Message Msg) {
			ThreadStart ts = new ThreadStart(() => {
				List<string> ToSend = new List<string>();
				foreach (string Filename in ResultsList) 
					if (Filename.StartsWith(Msg.Body.Trim())) ToSend.Add(Filename);
				// send no results reply if nothing was found
				if(ToSend.Count == 0) {
					Message msg = new Message();
					msg.Source = receiver.CommAddress;
					msg.Destination = Msg.Owner;
					msg.command = Message.Command.NoResult;
					msg.Body = "No results found for \"" + Msg.Body + "\"";
					sendingQueue.enQ(msg);
					return;
				}
				foreach (string Filename in ToSend) {
					Message msg = new Message();
					msg.Source = receiver.CommAddress;
					msg.Destination = Msg.Owner;
					msg.command = Message.Command.Results;
					msg.Body = Filename;
					sendingQueue.enQ(msg);
					string filePath = Path.GetFullPath(Path.Combine(receiver.GetResultsPath(), Filename));
					if (!sender.StreamChannelExists(ConnectedStreams[Msg.Owner]))
						sender.CreateStreamingChannel(ConnectedStreams[Msg.Owner], Msg.Owner);
					try {
						Console.Write("\n  Sending file {0} to {1} ... ", Filename, Msg.Owner);

						sender.UploadFile(filePath, FileTransferMessage.
							FileType.Result, Msg.Owner);
						Console.Write("Sent successfully");
					} catch (Exception) {
						Console.Write("Failed\n  Blocking file {0} untill {1} is connected", Filename, Msg.Owner);
						FileTransferMessage file = new FileTransferMessage();
						file.Filename = filePath;
						file.Type = FileTransferMessage.FileType.Result;
						if (BlockedFiles.ContainsKey(Msg.Owner))
							BlockedFiles[Msg.Owner].Add(file);
						else {
							List<FileTransferMessage> temp = new List<FileTransferMessage>();
							temp.Add(file);
							BlockedFiles.Add(Msg.Owner, temp);
						}
					}
				}
			});
			new Thread(ts).Start();
		}

		/// <summary>Forwards result file, received from TH, to the corresponding client</summary>
		/// <param name="Msg">Specifies the result message 
		/// received from the TestHarness</param>
		private void ForwardResult(Message Msg) {
			if (Msg.command != Message.Command.Results) return;
			Message msg = new Message();
			msg.Source = receiver.CommAddress;
			msg.Destination = Msg.Owner;
			msg.command = Message.Command.Results;
			msg.Body = Msg.Body;
			sendingQueue.enQ(msg);

			string filePath = Path.GetFullPath(Path.Combine(receiver.GetResultsPath(), msg.Body));
			if (!sender.StreamChannelExists(ConnectedStreams[Msg.Owner]))
				sender.CreateStreamingChannel(ConnectedStreams[Msg.Owner], Msg.Owner);
			try {
				Console.Write("\n  Sending file {0} to {1} ... ", Msg.Body, Msg.Owner);

				sender.UploadFile(filePath, FileTransferMessage.
					FileType.Result, Msg.Owner);
				Console.Write("Sent successfully");
			} catch (Exception) {
				Console.Write("Failed\n  Blocking file {0} untill {1} is connected", Msg.Body, Msg.Owner);
				FileTransferMessage file = new FileTransferMessage();
				file.Filename = filePath;
				file.Type = FileTransferMessage.FileType.Result;
				if (BlockedFiles.ContainsKey(Msg.Owner))
					BlockedFiles[Msg.Owner].Add(file);
				else {
					List<FileTransferMessage> temp = new List<FileTransferMessage>();
					temp.Add(file);
					BlockedFiles.Add(Msg.Owner, temp);
				}
			}
		}

		/// <summary>Moves the blocked messages to the sending queue</summary>
		/// <param name="Owner">Specifies the destination that was lost earlier</param>
		private void SendBlockedMessages(string Owner) {
			if (BlockedMessages.ContainsKey(Owner)) {
				foreach (Message msg in BlockedMessages[Owner])
					sendingQueue.enQ(msg);
				BlockedMessages[Owner].Clear();
			}
		}

		/// <summary>Sends the blocked files to their destination</summary>
		/// <param name="Owner">Specifies the destination of the files</param>
		private void SendBlockedFiles(string Owner) {
			if (!BlockedFiles.ContainsKey(Owner)) return;
			if (BlockedFiles[Owner].Count == 0) return;
			ThreadStart ts = new ThreadStart(()=> {
				if (!sender.StreamChannelExists(ConnectedStreams[Owner]))
					sender.CreateStreamingChannel(ConnectedStreams[Owner], Owner);
				foreach (var file in BlockedFiles[Owner]) {
					try {
						Console.Write("\n  Sending file {0} to {1} ... ", file.Filename, Owner);
						sender.UploadFile(file.Filename, file.Type, Owner);
						Console.Write("Sent successfully");
					} catch (Exception) {
						Console.Write("Failed\n  Blocking file {0} untill {1} is connected", file.Filename, Owner);
					}
				}
				BlockedFiles[Owner].Clear();
			});
			new Thread(ts).Start();
		}
		#endregion
		/* ------------------------< End of Main methods Region >----------------------- */


		/* ------------------------------< Threads Region >----------------------------- */
		#region

		/// <summary>The thread that processes messages received from TestHarness
		/// or other Clients</summary>
		private void ReceiveMessages() {
			while (true) {
				Message msg = receiver.GetMessage(); ReceivedMessages.Add(msg);
				if (msg.Body.ToLower().Equals("quit")) break;

				string listBoxItem;
				switch (msg.command) {
					case Message.Command.ConnectionTest:
						ConnectedEnds[msg.Owner] = msg.Source;
						listBoxItem = "From: " + msg.Owner + " @ " + msg.Source + " - Type: Connection Test";
						SendBlockedMessages(msg.Owner);
						break;
					case Message.Command.StreamNotifier:
						listBoxItem = "From: " + msg.Owner + " @ " + msg.Source + " - Type: Stream Notifier";
						ConnectedStreams[msg.Owner] = msg.Source;
						SendBlockedFiles(msg.Owner);
						break;
					case Message.Command.FileListRequest:
						listBoxItem = "From: " + msg.Source + " - Type: File List Request";
						SendFileList(msg.Owner);
						break;
					case Message.Command.TestRequest:
						listBoxItem = "From: " + msg.Source + " - Type: Test Request";
						ProcessXMLTestRequest(msg);
						break;
					case Message.Command.ResultRequest:
						listBoxItem = "From: " + msg.Source + " - Type: Result Request";
						SendResult(msg);
						break;
					case Message.Command.Results:
						listBoxItem = "From: " + msg.Source + " - Type: Result - Name: " + msg.Body;
						ForwardResult(msg);
						Dispatcher.BeginInvoke(new Action(() => {ResSearchRepository();}));
						break;
					default:
						listBoxItem = "From: " + msg.Source + " - Type: Unknown";
						break;
				}
				Dispatcher.BeginInvoke(new Action(()=> { lstBoxMessages.Items.Add(listBoxItem); }));
				Console.Write("\n  Received message of type {0} from {1}", msg.command, msg.Source);
			}
		}

		/// <summary>The thread that sends messages to their corresponding clients</summary>
		private void SendMessages() {
			while (true) {
				Message msg = sendingQueue.deQ();
				if (msg.Body.ToLower().Equals("quit")) break;
				if (!sender.CommChannelExists(ConnectedEnds[msg.Destination]))
					sender.CreateCommChannel(ConnectedEnds[msg.Destination], msg.Destination);
				try {
					Console.Write("\n  Sending message of type {0} to {1} ... ", msg.command, msg.Destination);
					sender.PostMessage(msg, msg.Destination);
					Console.Write("Sent successfully");
				} catch (Exception) {
					Console.Write("Failed");
					Console.Write("\n  Blocking message until {0} is available", msg.Destination);
					if(BlockedMessages.ContainsKey(msg.Destination))
						BlockedMessages[msg.Destination].Add(msg);
					else {
						List<Message> temp = new List<Message>();
						temp.Add(msg);
						BlockedMessages.Add(msg.Destination, temp);
					}
				}
			}
		}
		#endregion
		/* ---------------------------< End of Threads Region >------------------------- */


		/* ----------------------------< Main Buttons Region >-------------------------- */
		#region
		private void btnCreateChannels_Click(object sender, RoutedEventArgs e) {
			int port1=0, port2=0;
			try {
				int.TryParse(txtPort1.Text, out port1);
				receiver.CreateCommHost(new Uri("http://localhost:" + port1
					+ "/IComm/Repository"));
			} catch (Exception) {
				Console.Write("\n  Failed to create communication host on port {0}", port1);
			}
			try {
				int.TryParse(txtPort2.Text, out port2);
				receiver.CreateStreamHost(new Uri("http://localhost:" + port2
					+ "/IStream/Repository"));
			} catch (Exception) {
				Console.Write("\n  Failed to create streaming host on port {0}", port2);
				return;
			}
		}

		private void SetFilesPath_Click(object sender, RoutedEventArgs e) {
			System.Windows.Forms.FolderBrowserDialog fbd =
				new System.Windows.Forms.FolderBrowserDialog();
			fbd.Description = "Select libraries directory";
			if (fbd.ShowDialog() == System.Windows.Forms.DialogResult.OK)
				receiver.SetFilesPath(fbd.SelectedPath);
		}

		private void SetResultsPath_Click(object sender, RoutedEventArgs e) {
			System.Windows.Forms.FolderBrowserDialog fbd =
				new System.Windows.Forms.FolderBrowserDialog();
			fbd.Description = "Select results directory";
			if (fbd.ShowDialog() == System.Windows.Forms.DialogResult.OK)
				receiver.SetResultsPath(fbd.SelectedPath);
		}

		private void btnRefreshLibraries_Click(object sender, RoutedEventArgs e) {
			LibSearchRepository();
		}

		private void btnRefreshResults_Click(object sender, RoutedEventArgs e) {
			ResSearchRepository(); 
		}
		#endregion
		/* ------------------------< End of Main Buttons Region >----------------------- */


		/* -------------------------< Accessory buttons Region >------------------------ */
		#region 
		private void ExitMenuItem_Click(object sender, RoutedEventArgs e) {
			Close();
		}

		protected override void OnClosing(CancelEventArgs e) {
			receiver.PostMessage(new Message() { Body = "quit" });
			sendingQueue.enQ(new Message() { Body = "quit" });
			if (mReceiver.IsAlive) mReceiver.Join();
			if (mSender.IsAlive) mSender.Join();
			Environment.Exit(0);
		}

		private void lstBoxMessages_MouseDoubleClick(object sender, System.Windows.Input.MouseButtonEventArgs e) {
			MessageView view = new MessageView(ReceivedMessages[lstBoxMessages.SelectedIndex]);
			view.Show();
		}
		
		#endregion
		/* ----------------------< End of Accessory buttons Region >-------------------- */
	}


	/* --------------------------< Extension methods Region >------------------------- */
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
	/* -----------------------< End of Extension methods Region >--------------------- */

}
