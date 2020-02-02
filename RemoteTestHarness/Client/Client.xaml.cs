/////////////////////////////////////////////////////////////////////////////
//  Client.xaml.cs - Client GUI and processing														 //
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
 *   This module demonstrates the client GUI which offers all services for
 *   the end client. It can be used to:
 *   1. Send test requests
 *   2. Send assemblies
 *   3. Get files in the repository
 *   4. Get test results
 *   5. Get assemblies from the repository
 *   It supprts multi-threaded processing to send/receive messages/files.
 * 
 *   Public Interface
 *   ----------------
 *   It only holds one public method as the rest cannot be accessed except
 *   whithin the class itself because it is an interactive application
 *   ClientGUI client = new ClientGUI();
 *   
 * 
 */
/*
 *   Build Process
 *   -------------
 *   - Required files:   Client.xaml.cs
 *   - Compiler command: csc Client.xaml.cs
 * 
 *   Maintenance History
 *   -------------------
 *   ver 1.0 : 16 November 2016
 *     - first release
 * 
 */

using System;
using System.Windows;
using System.Windows.Controls;
using Microsoft.Win32;
using System.Runtime.InteropServices;
using System.Threading;
using System.ComponentModel;
using Communication;
using BlockingQueue;
using System.IO;
using System.Xml.Linq;
using System.Windows.Input;
using System.Collections.Generic;

namespace Client {
	/// <summary>Holds the client activity</summary>
	public partial class ClientGUI : Window {

		/// <summary>Specifies the client name which connected to the system</summary>
		public string ClientName { get; set; }

		/* ----------------< Fields and properties declarations Region >---------------- */
		#region
		/// <summary>
		/// Holds the Client host that receives messages from Repository
		/// </summary>
		Receiver receiver;
		/// <summary> The receiver thread </summary>
		Thread mReceiver;
		/// <summary> The sender thread</summary>
		Thread mSender;
		/// <summary> Sends communication stream messages to the repository </summary>
		Sender sender;
		/// <summary>Holds the messages to send to the repository</summary>
		BlockingQ<Message> sendingQueue;
		/// <summary>Holds the files to download from the repository</summary>
		BlockingQ<FileTransferMessage> FilesToDownload;
		/// <summary>The thread that downloads the required files from the repository</summary>
		Thread fDownloader;
		/// <summary>Holds all received messages for display purposes</summary>
		List<Message> ReceivedMessages;
		#endregion
		/* -------------< End of Fields and properties declarations Region >------------ */


		/* ----------------------< Initialization methods Region >---------------------- */
		#region
		/// <summary>Enables console view for event logging </summary>
		[DllImport("Kernel32")]
		public static extern void AllocConsole();

		/// <summary>Initializes the client and prepares it to work</summary>
		/// <param name="ClientName">Specifies the client name on which it will be 
		/// contacting the repository through</param>
		/// <param name="Demo">Specifies creation mode. If true then it will 
		/// demonstrate action automatically at startup</param>
		/// <param name="XML">Specifies target XML test request for demonstration</param>
		public ClientGUI(string ClientName, bool Demo = false, string[] Files = null) {
			InitializeComponent();
			this.ClientName = ClientName;
			InitializeClient();
			if (Demo) Demonstrate(Files);
		}
		/// <summary>Sets up the client to work</summary>
		void InitializeClient() {
			string s = ClientName.Substring(0, 1).ToUpper() + ClientName.Substring(1);
			Title = s + " - Client Window";
			txtAuthor.Text = ClientName;
			// start the console
			AllocConsole();
			
			Console.Title = s + " Client Console";
			Console.Write(("Client: " + s + " - Console View").Title());

			// Initialize main fields
			sendingQueue = new BlockingQ<Message>();
			FilesToDownload = new BlockingQ<FileTransferMessage>();

			receiver = new Receiver();
			receiver.SetFilesPath(".\\" + ClientName + "\\Libraries");
			receiver.SetResultsPath(".\\" + ClientName + "\\Results");
			sender = new Sender();
			sender.FilesPath = ".\\" + ClientName + "\\Libraries";
			sender.ResultsPath = ".\\" + ClientName + "\\Results";

			// Initialize communication and streaming hosts
			for (int i = 9000; i < 10000; i++)
				try {
					receiver.CreateCommHost(new Uri("http://localhost:" + i + "/IComm/Client"));
					receiver.CreateStreamHost(new Uri("http://localhost:" + i + "/IService/Client"));
					break;
				} catch (Exception) {
					Console.Write("\n  Failed to connect on port {0}", i);
					Thread.Sleep(100);
				}
			Console.Write("\n  Client communication host: {0}\n  Client streaming host: {1}"
				, receiver.CommAddress, receiver.StreamAddress);
			// Start the receiver thread
			mReceiver = new Thread(ReceiveMessages);
			mReceiver.Start();
			// create sender channels that connect to the repository
			// this starts the sender thread if successful
			CreateSenderChannels(8000, 8001);
			mSender = new Thread(SendMessages);
			mSender.Start();
			fDownloader = new Thread(DownloadFiles);
			fDownloader.Start();
			ReceivedMessages = new List<Message>();
		}

		#endregion
		/* -------------------< End of Initialization methods Region >------------------ */


		/* ---------------------------< Main methods Region >--------------------------- */
		#region
		/// <summary>Creates sender channels to bind to repository</summary>
		/// <param name="port1">Specifies communication channel port</param>
		/// <param name="port2">Specifies streaming channel port</param>
		void CreateSenderChannels(int port1, int port2) {
			try {
				sender.CreateCommChannel("http://localhost:" + port1
					+ "/IComm/Repository", "Repository");
				Message msg = new Message();
				msg.Owner = ClientName;
				msg.Source = receiver.CommAddress;
				msg.command = Message.Command.ConnectionTest;
				Console.Write("\n  Sending communication host address to repository ... ");
				sender.PostMessage(msg, "Repository");
				Console.Write("Sent successfully");
				msg = new Message();
				msg.Owner = ClientName;
				msg.Source = receiver.StreamAddress;
				msg.command = Message.Command.StreamNotifier;
				Console.Write("\n  Sending streaming host address to repository ... ");
				sender.PostMessage(msg, "Repository");
				Console.Write("Sent successfully");
				Console.Write("\n  Communication channel established with repository on port: {0}", port1);
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
				Console.Write("\n  Streaming channel established with repository on port: {0}\n", port2);
			} catch (Exception ex) {
				Console.Write("Failed\n  Failed to create streaming channel to Repository"
					+ " on port {0}\n", port2);
				Console.Write("\n  Details: {0}\n", ex.Message);
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
				msg.Owner = ClientName;
				msg.Source = receiver.CommAddress;
				msg.command = Message.Command.ConnectionTest;
				Console.Write("\n  Sending test message to repository ... ");
				sender.PostMessage(msg, "Repository");
				Console.Write("Sent successfully");
				msg = new Message();
				msg.Owner = ClientName;
				msg.Source = receiver.StreamAddress;
				msg.command = Message.Command.StreamNotifier;
				Console.Write("\n  Sending streaming host address to repository ... ");
				sender.PostMessage(msg, "Repository");
				Console.Write("Sent successfully");
				Console.Write("\n  Communication channel established with repository on address: {0}", commUri);
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
				Console.Write("\n  Streaming channel established with repository on address: {0}", streamUri);
			} catch (Exception ex) {
				Console.Write("Failed\n  Failed to create streaming channel to Repository"
					+ " on address {0}", streamUri);
				Console.Write("\n  Details: {0}\n", ex.Message);
				return;
			}
		}

		/// <summary>Processes results when received from repository</summary>
		/// <param name="msg">Specifies the received message</param>
		void ProcessResultsMessage(Message msg) {
			// double check for message type
			if (msg.command != Message.Command.Results) return;
			//XDocument xdoc = XDocument.Parse(@msg.Body);
			
		}

		/// <summary>Processes received repository files list</summary>
		/// <param name="msg">Specifies the received message</param>
		void ProcessFileListMessage(Message msg) {
			// double check for message type
			if (msg.command != Message.Command.FileList) return;
			try {
				Dispatcher.Invoke(() => { lstBoxRepFiles.Items.Clear(); });
				XDocument xdoc = new XDocument();
				xdoc = XDocument.Parse(msg.Body);
				foreach (var element in xdoc.Root.Elements()) 
					Dispatcher.Invoke(() => {
						lstBoxRepFiles.Items.Add(element.Attribute("value").Value);
						
					});
				
			} catch (Exception) {
				Console.Write("\n  Failed to process XML:\n{0}\n", msg.Body);
			}
		}
		
		/// <summary>Prepares XML test request and sends it to repository</summary>
		/// <param name="FilePath">Specifies the XML file path</param>
		void ProcessNSendXML(string FilePath) {
			try {
				// if invalid XML it will throw error
				XDocument xdoc = XDocument.Load(FilePath);
				// prepare test request message
				Message msg = new Message();
				msg.Owner = receiver.CommAddress;
				msg.Source = receiver.CommAddress;
				msg.Destination = "Repository";
				msg.command = Message.Command.TestRequest;
				msg.Body = xdoc.ToString();
				// place it in the sending queue
				sendingQueue.enQ(msg);
			} catch (Exception) {
				Console.Write("\n  Bad XML file: {0}", Path.GetFileName(FilePath));
			}
		}

		/// <summary>Sends DLL file to the repository</summary>
		/// <param name="FilePath">Specifies the file path</param>
		void SendLibrary(string FilePath) {
			try {
				Console.Write("\n  Sending library \"{0}\" to repository ... ", Path.GetFileName(FilePath));
				sender.UploadFile(FilePath, FileTransferMessage.FileType.Library, "Repository");
				Console.Write("Sent Successfully");
			} catch (Exception) {
				Console.Write("Failed\n  Cannot send file {0}. Check "+
					"if connection is established with the repository", Path.GetFileName(FilePath));
			}
		}


		#endregion
		/* ------------------------< End of Main methods Region >----------------------- */


		/* ------------------------------< Threads Region >----------------------------- */
		#region
		/// <summary>The thread that processes received messages</summary>
		void ReceiveMessages() {
			while (true) {
				Message msg = receiver.GetMessage();
				if (msg.Body.ToLower().Equals("quit")) break;
				ReceivedMessages.Add(msg);
				Console.Write("\n  Received message of type {0} from Repository", msg.command);
				switch (msg.command) {
					case Message.Command.Results:
						Dispatcher.BeginInvoke(new Action(()=> {
							lstBoxMsgs.Items.Add("Test Result: "+ msg.Body);
						}));
						ProcessResultsMessage(msg);
						break;
					case Message.Command.NoResult:
						Dispatcher.BeginInvoke(new Action(()=> {
							lstBoxMsgs.Items.Add("No Results found for - "+msg.Body); }));
						break;
					case Message.Command.FileList:
						Dispatcher.BeginInvoke(new Action(() => {
							lstBoxMsgs.Items.Add("Repository File List - Check 'Repository Files' list");
						}));
						ProcessFileListMessage(msg);
						break;
					case Message.Command.InvalidTestRequest:
						Dispatcher.BeginInvoke(new Action(() => {
							lstBoxMsgs.Items.Add("The sent test request is invalid");
						}));
						break;
					default:
						Dispatcher.BeginInvoke(new Action(() => {
							lstBoxMsgs.Items.Add("Unrecognized message type");
						}));
						break;
				}
			}
		}

		/// <summary>The thread that sends communication messages</summary>
		void SendMessages() {
			while (true) {
				Message msg = sendingQueue.deQ();
				if (msg.Body.ToLower().Equals("quit")) break;
				msg.Owner = ClientName;
				try {
					Console.Write("\n  Sending message of type {0} to repository ... ", msg.command);
					sender.PostMessage(msg, msg.Destination);
					Console.Write("Sent successfully");
				} catch (Exception) {
					Console.Write("Failed");
				}
			}
		}

		/// <summary>The thread that processes the input files and 
		/// prepares the suitable messages to send (communication or file messages) </summary>
		void PrepareFiles() {
			bool con = false;
			Dispatcher.Invoke(() => {
				con = lstBoxFilesToSend.HasItems;
				btnAddFiles.IsEnabled = false;
				btnSendFiles.IsEnabled = false;
			});
			while (con) {
				Message msg = new Message();
				string Filename = "";
				Dispatcher.Invoke(() => {
					Filename = lstBoxFilesToSend.Items.GetItemAt(0).ToString();
					lstBoxFilesToSend.Items.RemoveAt(0);
				});
				if (!Path.HasExtension(Filename)) {
					Console.Write("\n  Invalid file {0}", Filename);
					Dispatcher.Invoke(() => { con = lstBoxFilesToSend.HasItems; });
					continue;
				}
				Thread.Sleep(100);
				string ext = Path.GetExtension(Filename).ToLower();
				switch (ext) {
					case ".xml":
						ProcessNSendXML(Filename);
						break;
					case ".dll":
						SendLibrary(Filename);
						break;
					default:
						Console.Write("\n  Invalid file {0}", Filename);
						break;
				}
				Dispatcher.Invoke(() => { con = lstBoxFilesToSend.HasItems; });

			}
			Dispatcher.Invoke(() => {
				con = lstBoxFilesToSend.HasItems;
				btnAddFiles.IsEnabled = true;
				btnSendFiles.IsEnabled = true;
			});
		}

		/// <summary>Downloads files from repository</summary>
		void DownloadFiles() {
			while (true) {
				FileTransferMessage file = FilesToDownload.deQ();
				if (file.Filename.ToLower().Equals("quit")) break;
				try {
					sender.DownloadFile(file.Filename, file.Type,"Repository");
				} catch (Exception) {
					Console.Write("\n  Unable to download file {0} from the repository", file.Filename);
				}
			}
		}
		#endregion
		/* ---------------------------< End of Threads Region >------------------------- */


		/* ----------------------------< Main Buttons Region >-------------------------- */
		#region
		private void btnGetResults_Click(object sender, RoutedEventArgs e) {
			Message msg = new Message();
			msg.Owner = ClientName;
			msg.Source = receiver.CommAddress;
			msg.Destination = "Repository";
			msg.command = Message.Command.ResultRequest;
			var date = "";
			string popup = "Date was not set\nThis would get results for all test by \"" +
				txtAuthor.Text + "\"\nAre you sure you want to continue?";

			if (testDate.SelectedDate != null)
				date = testDate.SelectedDate.Value.ToString("_MM_dd_yyyy");
			else if (MessageBox.Show(popup,"Confirm", MessageBoxButton.YesNo
				, MessageBoxImage.Warning, MessageBoxResult.No) == MessageBoxResult.No) {
				return;
			}
			msg.Body = txtAuthor.Text + date;
			sendingQueue.enQ(msg);
		}

		private void btnAddFiles_Click(object sender, RoutedEventArgs e) {
			OpenFileDialog ofd = new OpenFileDialog();
			ofd.Filter = "All Supported Types| *.xml;*.dll| XML Files | *.xml|"
									+" Dynamic Link Library Files | *.dll";
			ofd.Multiselect = true;
			if(ofd.ShowDialog() == true) {
				string[] Filenames = ofd.FileNames;
				foreach(string Filename in Filenames) 
					lstBoxFilesToSend.Items.Add(Filename);
			}
		}

		private void btnSendFiles_Click(object sender, RoutedEventArgs e) {
			if (!lstBoxFilesToSend.HasItems) {
				Console.Write("\n  There are no files to send .. ");
				return;
			}
			new Thread(PrepareFiles).Start();
		}
		
		private void btnGetFiles_Click(object sender, RoutedEventArgs e) {
			Message msg = new Message();
			msg.Source = receiver.CommAddress;
			msg.Destination = "Repository";
			msg.command = Message.Command.FileListRequest;
			sendingQueue.enQ(msg);
		}

		private void ConnectRep_Click(object sender, RoutedEventArgs e) {
			// initialize ports
			int port1=8000, port2=8001;
			// create form to set the ports
			System.Windows.Forms.Form prompt = new System.Windows.Forms.Form();
			prompt.Width = 500;
			prompt.Height = 70;
			prompt.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedToolWindow;
			prompt.Text = "Connect to Repository (ports)";
			// initialize form components
			System.Windows.Forms.Label textLabel =
				new System.Windows.Forms.Label() { Left = 5, Top = 5, Width=80, Text = "Comm Port:" };
			System.Windows.Forms.TextBox txtPort1 =
				new System.Windows.Forms.TextBox() { Left = 90, Top = 5 };
			System.Windows.Forms.Label textLabe2 =
				new System.Windows.Forms.Label() { Left = 190, Width = 80, Top = 5, Text = "Streaming Port:" };
			System.Windows.Forms.TextBox txtPort2 =
				new System.Windows.Forms.TextBox() { Left = 280, Top = 5 };
			System.Windows.Forms.Button confirmation = 
				new System.Windows.Forms.Button() 
				{ Text = "Confirm", Left = 400, Width = 80, Top = 3 };
			confirmation.Click += (send, ex) => {
				// get port1 value from form
				int.TryParse(txtPort1.Text, out port1);
				// get port2 value from form
				int.TryParse(txtPort2.Text, out port2);
				prompt.Close(); };
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

		private void LnkConnRep_Click(object sender, RoutedEventArgs e) {
			string commUri = "", streamUri = "";
			// create form to set the ports
			System.Windows.Forms.Form prompt = new System.Windows.Forms.Form();
			prompt.Width = 500;
			prompt.Height = 100;
			prompt.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedToolWindow;
			prompt.Text = "Connect to Repository (addresses)";
			// initialize form components
			System.Windows.Forms.Label textLabel =
				new System.Windows.Forms.Label() { Left = 5, Top = 5, Width = 100, Text = "Comm Host:", TabIndex=10};
			System.Windows.Forms.TextBox txtCommUri =
				new System.Windows.Forms.TextBox() { Left = 120, Top = 5, Width = 250,
					Text = "http://localhost:8000/IComm/Repository", TabIndex = 1 };
			System.Windows.Forms.Label textLabe2 =
				new System.Windows.Forms.Label() { Left = 5, Width = 100, Top = 35, Text = "Streaming Host:", TabIndex=10 };
			System.Windows.Forms.TextBox txtStreamUri =
				new System.Windows.Forms.TextBox() { Left = 120, Top = 35, Width = 250,
					Text = "http://localhost:8001/IStream/Repository", TabIndex = 2 };
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
		/* -------------------------< End of Main Buttons Region >---------------------- */


		/* ----------------------------< Accessories Region >--------------------------- */
		#region
		private void Demonstrate(string[] FilePaths) {
			foreach (string XMLPath in FilePaths)
				lstBoxFilesToSend.Items.Add(XMLPath);
			btnSendFiles.PerformClick();
			int count = lstBoxMsgs.Items.Count;
			testDate.SelectedDate = DateTime.Now;
			btnGetResults.PerformClick();
		}

		private void ExitMenuItem_Click(object sender, RoutedEventArgs e) {
			Close();
		}

		protected override void OnClosing(CancelEventArgs e) {
			receiver.PostMessage(new Message() { Body = "quit" });
			sendingQueue.enQ(new Message() { Body = "quit" });
			FilesToDownload.enQ(new FileTransferMessage() { Filename = "quit" });
			if(mReceiver.IsAlive) mReceiver.Join();
			if(mSender.IsAlive) mSender.Join();
			if (fDownloader.IsAlive) fDownloader.Join();
			Environment.Exit(0);
		}

		private void lstBoxRepFiles_MouseDoubleClick(object sender, MouseButtonEventArgs e) {
			int index = lstBoxRepFiles.SelectedIndex;
			if (index != System.Windows.Forms.ListBox.NoMatches) {
				FileTransferMessage msg = new FileTransferMessage();
				msg.Filename = lstBoxRepFiles.Items.GetItemAt(index).ToString();
				msg.Type = FileTransferMessage.FileType.Library;
				FilesToDownload.enQ(msg);
			}
		}

		private void lstBoxMsgs_MouseDoubleClick(object sender, MouseButtonEventArgs e) {
			int index = lstBoxMsgs.SelectedIndex;
			if(index != System.Windows.Forms.ListBox.NoMatches) {
				MessageView view = new MessageView(ReceivedMessages[index], @receiver.GetResultsPath());
				view.Show();
			}
		}

		#endregion
		/* -------------------------< End of Accessories Region >----------------------- */


	}

	/* --------------------------< Extension methods Region >------------------------- */
	#region
	public static class ExtensionMethods {
		public static void PerformClick(this Button btn) {
			btn.RaiseEvent(new RoutedEventArgs(Button.ClickEvent));
		}

		public static string Title(this string s) {
			s = "\n  " + s + "\n ";
			s += new string('-', s.Length-2);
			s += "\n";
			return s;
		}
	}
	#endregion
	/* -----------------------< End of Extension methods Region >--------------------- */
}
