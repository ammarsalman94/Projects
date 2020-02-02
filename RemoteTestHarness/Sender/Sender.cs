/////////////////////////////////////////////////////////////////////////////
//  Sender.cs - demonstrate sending to a Receiver class in WCF			       //
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
 *   This module demonstrates sending messages to a Receiver object 
 *   created by another process. The binding type we used is WSHttpBinding
 *   as it is fast enough, secure and insures good message transmission.
 * 
 *   Public Interface
 *   ----------------
 *   Sender sender = new Sender();
 *   Sender sender2 = new Sender("http://localhost:port/IComm/Receiver");
 *   sender.CreateChannel("http://localhost:port/IComm/Receiver");
 *   Message msg; //set message attributes
 *   sender.Channel.PostMessage(msg);
 * 
 */
/*
 *   Build Process
 *   -------------
 *   - Required files:   Sender.cs 
 *   - Compiler command: csc Sender.cs
 * 
 *   Maintenance History
 *   -------------------
 *   ver 2.0 : 13 November 2016
 *		 - second release:
 *				- support for file transfer
 *				- support for multiple communication/streaming channels
 *   ver 1.0 : 11 November 2016
 *     - first release
 * 
 */

using System;
using System.Collections.Generic;
using System.ServiceModel;
using System.Threading;
using System.IO;

namespace Communication {
	/// <summary>Demonstrates sending messages and files to Receiver</summary>
	public class Sender {
		// Fields and properties declarations
		#region

		/// <summary>The communication channels used to bind to 
		/// Receiver on some specified address</summary>
		IDictionary<string, IComm> CommChannels;
		/// <summary>The streaming channels used to bind to 
		/// Receiver on some specified address</summary>
		IDictionary<string, IStreamService> StreamingChannels;
		/// <summary>Gets or sets libraries directory path</summary>
		public string FilesPath { get; set; } = "..\\Libraries";
		/// <summary>Gets or sets results directory path</summary>
		public string ResultsPath { get; set; } = "..\\Results";
		
		#endregion

		public Sender() {
			CommChannels = new Dictionary<string, IComm>();
			StreamingChannels = new Dictionary<string, IStreamService>();
		}

		// creating service channels
		#region

		/// <summary>Creates communication channel and binds it to 
		/// Receiver's host address</summary>
		/// <param name="url">Specifies the address of the host</param>
		/// <param name="ChannelName">Specifies local channel name</param>
		public void CreateCommChannel(string url, string ChannelName) {
			WSHttpBinding binding = new WSHttpBinding();
			EndpointAddress address = new EndpointAddress(url);
			CommChannels[ChannelName] = ChannelFactory<IComm>.CreateChannel(binding, address); ;
		}

		/// <summary>Creates streaming channel and binds it to 
		/// the Receiver's host address</summary>
		/// <param name="url">Specifies the address of the host</param>
		/// <param name="ChannelName">Specifies local channel name</param>
		public void CreateStreamingChannel(string url, string ChannelName) {
			BasicHttpSecurityMode securityMode = BasicHttpSecurityMode.None;
			BasicHttpBinding binding = new BasicHttpBinding(securityMode);

			binding.TransferMode = TransferMode.Streamed;
			binding.MaxReceivedMessageSize = 5368709120;
			EndpointAddress address = new EndpointAddress(url);

			 StreamingChannels[ChannelName] = ChannelFactory<IStreamService>
				.CreateChannel(binding, address);
		}
		#endregion

		// communication service method region
		#region
		/// <summary>Send message to specified host</summary>
		/// <param name="msg">Specifies message type and body</param>
		/// <param name="ChannelName">Specifies channel name used to send the message</param>
		public void PostMessage(Message msg, string ChannelName) {
			if (CommChannels[ChannelName] == null) {
				Console.Write("\n Unable to find the specified channel \"{0}\"", ChannelName);
				return;
			}
			CommChannels[ChannelName].PostMessage(msg);
		}
		#endregion

		// streaming service methods region
		#region
		/// <summary> Sends file to host using StreamChannel</summary>
		/// <param name="FilePath">Specifies the path of the file to send</param>
		public void UploadFile(string FilePath, FileTransferMessage.FileType Type
			, string ChannelName) {
			if (StreamingChannels[ChannelName] == null) {
				Console.Write("\n Unable to find the specifies channel \"{0}\"", ChannelName);
				return;
			}
			string Filename = Path.GetFileName(FilePath);
			using (var inputStream = new FileStream(FilePath, FileMode.Open)) {
				// Prepare file streaming message
				FileTransferMessage msg = new FileTransferMessage();
				msg.Type = Type;
				msg.Filename = Filename;
				msg.TransferStream = inputStream;
				// Send the message
				StreamingChannels[ChannelName].UploadFile(msg);
			}
		}

		/// <summary> Downloads file from host using StreamChannel</summary>
		/// <param name="Filename">Specifies the filename to download</param>
		public void DownloadFile(string Filename,
			FileTransferMessage.FileType Type, string ChannelName) {
			if (StreamingChannels[ChannelName] == null) {
				Console.Write("\n Unable to find the specified channel \"{0}\"", ChannelName);
				return;
			}
				string rfilename;
				switch (Type) {
					case FileTransferMessage.FileType.Library:
						rfilename = Path.Combine(FilesPath, Filename);
						break;
					case FileTransferMessage.FileType.Result:
						rfilename = Path.Combine(ResultsPath, Filename);
						break;
					default:
						return;
				}
				try {
					Stream strm = StreamingChannels[ChannelName].DownloadFile(Filename, Type);
					int BlockSize = 1024; byte[] block = new byte[BlockSize];
					// Make sure directories to save the downloaded file exist
					if (!Directory.Exists(FilesPath)) Directory.CreateDirectory(FilesPath);
					if (!Directory.Exists(ResultsPath)) Directory.CreateDirectory(ResultsPath);
					using (var outputStream = new FileStream(rfilename, FileMode.Create)) {
						while (true) {
							int bytesRead = strm.Read(block, 0, BlockSize);
							if (bytesRead > 0)
								outputStream.Write(block, 0, bytesRead);
							else
								break;
						}
					}
					Console.Write("\n  Received file \"{0}\"", Filename);
				} catch (Exception) {
					Console.Write("\n  Failed to get file \"{0}\" from server", Filename);
				}
		}

		/// <summary> Downloads library from host using StreamChannel</summary>
		/// <param name="Filename">Specifies the library name to download</param>
		public void DownloadLibrary(string Filename, string ChannelName, string dPath) {
			if (StreamingChannels[ChannelName] == null) {
				Console.Write("\n Unable to find the specified channel \"{0}\"", ChannelName);
				return;
			}
			// lock the channel to prevent multiple threads accessing the same file
			lock (StreamingChannels[ChannelName]) {
				string rfilename = Path.Combine(dPath, Filename);
				try {
					Stream strm = StreamingChannels[ChannelName].DownloadFile(Filename, FileTransferMessage.FileType.Library);
					int BlockSize = 1024; byte[] block = new byte[BlockSize];

					// Make sure the directory exists
					if (!Directory.Exists(dPath)) Directory.CreateDirectory(dPath);

					using (var outputStream = new FileStream(rfilename, FileMode.Create)) {
						while (true) {
							int bytesRead = strm.Read(block, 0, BlockSize);
							if (bytesRead > 0)
								outputStream.Write(block, 0, bytesRead);
							else
								break;
						}
					}
					Console.Write("\n  Received file \"{0}\"", Filename);
				} catch (Exception) {
					Console.Write("\n  Failed to get file \"{0}\" from server", Filename);
				}
			}
		}

		#endregion

		// Check channels region
		#region
		/// <summary>Determines whether certain communication channel exists or not</summary>
		/// <param name="key">Specifies communication channel name</param>
		/// <returns>Boolean (true or false) indicating whether channel was found or not</returns>
		public bool CommChannelExists(string key) {
			return CommChannels.ContainsKey(key);
		}

		/// <summary>Determines whether certain streaming channel exists or not</summary>
		/// <param name="key">Specifies streaming channel name</param>
		/// <returns>Boolean (true or false) indicating whether channel was found or not</returns>
		public bool StreamChannelExists(string key) {
			return StreamingChannels.ContainsKey(key);
		}
		#endregion

// test stub for Sender class region
#region
#if (TEST_SENDER)
		/// <summary>Test stub for the Sender class</summary>
		static void Main(string[] args) {
			Console.Write("\n  Creating Sender Instance \n --------------------------\n");
			Sender sender = new Sender();
			Message msg; int count = 0;
			Console.Write("\n   Sending communication messages\n  --------------------------------");
			while (true) {
				try {
					sender.CreateCommChannel("http://localhost:9090/IComm/Receiver", "Test");
					for (int i = 0; i < 5; i++) {
						// Create first message and send it
						msg = new Message();
						msg.command = Message.Command.TestRequest;
						msg.Body = "Test request from SENDER instance";
						Console.Write("\n  Sending message:\t{0}", msg.Body);
						sender.PostMessage(msg, "Test");
						Thread.Sleep(1000);
						// Create second message and send it
						msg = new Message();
						msg.command = Message.Command.ResultRequest;
						msg.Body = "Asking result for SENDER instance";
						Console.Write("\n  Sending message:\t{0}", msg.Body);
						sender.PostMessage(msg, "Test");
						Thread.Sleep(1000);
					}
					break;
				} catch (Exception) {
					// Try connecting 5 times before terminating
					Console.WriteLine(" Failed to connect after {0} trial(s)", count);
					Thread.Sleep(250);
					if ((count++) == 5) {
						Console.WriteLine("\n Terminating ... "); break;
					}
				}
			}
			Console.Write("\n\n   Sending files\n  ---------------");
			sender.CreateStreamingChannel("http://localhost:9091/IStream/Receiver", "Test");
			string filename = "..\\test.txt";
			if (!File.Exists(filename)) File.Create(filename); Thread.Sleep(50);
			Console.Write("\n Sending file \"{0}\" as library file", filename);
			sender.UploadFile(Path.GetFullPath(filename), FileTransferMessage.FileType.Library, "Test"); 
			Console.Write("\n Requesting library file \"{0}\"", Path.GetFileName(filename));
			sender.DownloadFile("test.txt", FileTransferMessage.FileType.Library, "Test");
			filename = "..\\test2.txt";
			if (!File.Exists(filename)) File.Create(filename); Thread.Sleep(50);
			Console.Write("\n Sending file \"{0}\" as library file", filename);
			sender.UploadFile(Path.GetFullPath(filename),FileTransferMessage.FileType.Library, "Test");
			Console.Write("\n Requesting result file \"{0}\"", Path.GetFileName(filename));
			sender.DownloadFile("test2.txt", FileTransferMessage.FileType.Result, "Test");
		}
#endif

#endregion
	}
}
