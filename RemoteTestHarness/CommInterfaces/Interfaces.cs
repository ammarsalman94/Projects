/////////////////////////////////////////////////////////////////////////////
//  IComm.cs - WCF communication interface													       //
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
 *   This interface defines the ServiceContract and the needed OperationContracts.
 *   Also, it defines the DataContract where there is a Message class used for 
 *   communication between several endpoints.
 * 
 *   Public Interface
 *   ----------------
 *   PostMessage(Message msg);
 *   UploadFile(FileTransferMessage msg);
 *   DownloadFile(string Filename, FileTransferMessage.FileType Type);
 * 
 */
/*
 *   Build Process
 *   -------------
 *   - Required files:   IComm.cs 
 *   - Compiler command: csc IComm.cs
 * 
 *   Maintenance History
 *   -------------------
 *   ver 2.0 : 12 November 2016
 *		 - second release
 *				- support for more types of communication messages
 *				- support for file streaming services
 *   ver 1.0 : 11 November 2016
 *     - first release
 * 
 */

using System.ServiceModel;
using System.Runtime.Serialization;
using System.IO;

namespace Communication {
	/// <summary>Communication interface</summary>
	[ServiceContract]
	public interface IComm {
		/// <summary>
		/// Implemented by Receiver to enqueue messages to its blocking queue<para/>
		/// See also: <seealso cref="Receiver.PostMessage(Message msg)"/>
		/// </summary>
		/// <param name="msg"></param>
		[OperationContract(IsOneWay = true)]
		void PostMessage(Message msg);
		
	}
	/// <summary>File streaming interface</summary>
	[ServiceContract]
	public interface IStreamService {
		/// <summary>Upload file to host</summary>
		/// <param name="msg">The message that contains the file to upload</param>
		[OperationContract(IsOneWay = true)]
		void UploadFile(FileTransferMessage msg);
		/// <summary>Download file from host</summary>
		/// <param name="filename">File name to download</param>
		/// <returns>Stream containing the downloaded file</returns>
		[OperationContract]
		Stream DownloadFile(string Filename, FileTransferMessage.FileType Type);
	}

	/// <summary>Used to generalize understanding between endpoints</summary>
	[DataContract]
	public class Message {
		/// <summary>Owner of the message</summary>
		[DataMember]
		public string Owner { get; set; }

		/// <summary>Sender's address which it will receive the callback on</summary>
		[DataMember]
		public string Source { get; set; } = "default";

		/// <summary>Destination address</summary>
		[DataMember]
		public string Destination { get; set; } = "default";

		/// <summary>Message command that specifies the response of the receiving end</summary>
		[DataMember]
		public Command command { get; set; }=Command.TestRequest;

		/// <summary>Message body usually in form of XML.ToString()</summary>
		[DataMember]
		public string Body { get; set; }="default";

		/// <summary>Command enum. Holds the message types exchanged between endpoints</summary>
		public enum Command {
			/// <summary>Connection test message type</summary>
			[EnumMember]
			ConnectionTest,

			/// <summary>Stream connection notifier</summary>
			[EnumMember]
			StreamNotifier,

			/// <summary>Test request message type</summary>
			[EnumMember]
			TestRequest,
			/// <summary>Test request validity</summary>
			[EnumMember]
			InvalidTestRequest,

			/// <summary>Result request message type</summary>
			[EnumMember]
			ResultRequest,
			/// <summary>Test results message type</summary>
			[EnumMember]
			Results,
			/// <summary>No test result found message type</summary>
			[EnumMember]
			NoResult,

			/// <summary>File list request message type</summary>
			[EnumMember]
			FileListRequest,
			/// <summary>File list message type</summary>
			[EnumMember]
			FileList,

			
		}
	}

	/// <summary>File streaming message class</summary>
	[MessageContract]
	public class FileTransferMessage {
		/// <summary>Gets or sets the filename</summary>
		[MessageHeader(MustUnderstand = true)]
		public string Filename { get; set; }

		/// <summary>Gets or sets file type</summary>
		[MessageHeader(MustUnderstand = true)]
		public FileType Type{get;set;}

		/// <summary>Gets or sets the file stream</summary>
		[MessageBodyMember(Order = 1)]
		public Stream TransferStream { get; set; }
		
		/// <summary>Specifies file type </summary>
		public enum FileType {
			/// <summary>Used to check if connection is established or not</summary>
			[EnumMember]
			ConnectionTest,
			/// <summary>Specifies DLL type file</summary>
			[EnumMember]
			Library,
			/// <summary>Specifies test result type file</summary>
			[EnumMember]
			Result
		}
	}
}
