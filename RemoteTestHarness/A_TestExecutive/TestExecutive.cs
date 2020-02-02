/////////////////////////////////////////////////////////////////////////////
//  TestExecutive.cs - Demonstrates Project Services 											 //
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
 *   This module demonstrates how the system as a whole works together.
 *   It also demonstrates meeting the requirements for the project.
 * 
 *   Public Interface
 *   ----------------
 *   None. This project just demonstrates at startup and does not
 *   contain any useable classes. It is bound to the project only.
 * 
 */
/*
 *   Build Process
 *   -------------
 *   - Required files:   TestExecutive.cs 
 *   - Compiler command: csc TestExecutive.cs
 * 
 *   Maintenance History
 *   -------------------
 *   ver 1.0 : 17 November 2016
 *     - first release
 * 
 */

using System;
using System.Diagnostics;
using System.Threading;
using System.Runtime.InteropServices;

namespace A_TestExecutive {
	class TestExecutive {

		[DllImport("user32.dll")]
		static extern bool ShowWindow(IntPtr hWnd, int nCmdShow);

		private static Process rep, th, c1, c2, c3;

		/// <summary>Prepares the processes to start</summary>
		static void Startup() {
			"   STARTING TEST EXECUTIVE TO DEMONSTRATE REQUIREMENTS   ".Title('=');
			"Requirement #1: Implemented in C# .. Done".Title();
			Console.Write("\n");
			"Starting Repository".Title('=');
			rep = Process.Start("Repository.exe");
			Console.Write("\n Reporitory Started\n");
			Thread.Sleep(500);
			ShowWindow(rep.MainWindowHandle, 6);

			"Starting Test Harness".Title('=');
			th = Process.Start("Remote TestHarness.exe");
			Console.Write("\n Test Harness Started\n");
			Thread.Sleep(500);
			ShowWindow(th.MainWindowHandle, 6);

			"Starting Clients \"Ammar Salman\" & \"Odai Salman\"".Title('=');
			"Requirement #2: Test requests in form of XML and libraries in form of DLL .. Done".Title();
			Console.Write("  Clients are sending:\n"
				+ "    Requests: test1.xml, test2.xml, test3.xml\n"
				+ "    Libraries: TestLibrary1.dll, TestLibrary2.dll and TestLibrary3.dll");
			Console.Write("\n");

			c1 = Process.Start("Client.exe", "\"Ammar Salman\" true TestLibrary1.dll TestLibrary2.dll"
				+" TestLibrary3.dll test1.xml test2.xml");
			Thread.Sleep(1000);
			ShowWindow(c1.MainWindowHandle, 6);
			c2 = Process.Start("Client.exe", "\"Odai Salman\" true TestLibrary3.dll test3.xml");
			Thread.Sleep(1000);
			ShowWindow(c2.MainWindowHandle, 6);
		}

		/// <summary>Demonstrates meeting the requirements </summary>
		static void Demo() {
			"Requirement #3: file validation before testing .. Done".Title();
			Console.Write("  Test request 'test2.xml' specifies 'foo.dll' library which does not exist." +
				"\n  Sending client a message indicating that library does not exist" +
				"\n  Please check Client Ammar Salman messages for more details\n");
			Thread.Sleep(1000);

			"Requirement #4: enqueuing tests and executing them concurrently .. Done".Title();
			Console.Write("  Please refer to TestHarness.xaml.cs:Method:TestFiles for more details");
			Console.Write("\n  Also refer to Remote TestHarness window which shows concurrent processing\n");

			"Requirement #5: each test driver derives from ITest .. Done".Title();
			Console.Write("  Please look at TestAllDomain.cs: 'ITest Interface' region implementation\n");

			"Requirement #6: Slightly different implementation but serves same purpose".Title();
			Console.Write("  I have discussed it with the professor and Clients only send files to"
					  + "\n  the repository which in turn sends the requests to the  Test Harness.");
			Console.Write("\n  Please refer to the attached 'Brief Summary.txt' for further details\n");

			Thread.Sleep(5000);
			ShowWindow(rep.MainWindowHandle, 2);

			"Requirement #7: Test Harness should send its results .. Done".Title();
			Console.Write("  Please check the repository window for the received result file from" +
				" Test Harness. Then check the client where the detailed log is present\n");
			Thread.Sleep(1000);

			"Requirement #8: Storing the results collaborativly".Title();
			Console.Write("  Test Harness and Repository both have records for the test result");
			Console.Write("\n  Please refer to Repository Window and Test Harness storage for details\n");
			Thread.Sleep(1000);

			"Requirement #9: support for client queries".Title();
			Console.Write("  Clients already posted get result requests. Check their GUIs \n");

			"Requirement #10: All communication should be implementaed using WCF .. Done".Title();
			Console.Write("  Please refer to CommunicationInterfaces.Interfaces.cs for details\n");

			"Requirement #11: Clients should have GUI. Repository & Test Harness may have GUI .. You can see them!".Title();

			"Requirement #12: Shall include means for test time execution .. Done".Title();
			Console.Write("  Check Clients' results messages that specify the time elapsed for each test request\n");

			"Requirement #13: includes Test Exective .. This is it!".Title();

			"Requirement #14: includes brief summary".Title();
			Console.Write("  Check solution for 'Biref Summary.txt'\n");

		}

		/// <summary>Demonstrates extra features added to the project</summary>
		static void DemoExtra() {
			Console.Write("\n\nDemonstrating extra work: I  designed  the  system in a way  that when some  client  disconnects"
			  + "\n  and the repository had a message for that client it will not discard the message. Instead, the"
			  + "\n  repository will block the message until the client is  connected  again.  However, even if the"
			  + "\n  client connected on another address the repository will send the  blocked  messages on the new"
			  + "\n  address. The same concept applies to files.");
			Console.Write("\n\n  Starting Client 'Salman Salman'. This client sends test request (test4.xml) ...");
			c3 = Process.Start("Client.exe", "\"Salman Salman\" true TestLibrary3.dll test4.xml");
			Thread.Sleep(5000);
			Console.Write("\n  Shutting down client 'Salman Salman'");
			c3.Kill();
			Console.Write("\n  Waiting before launching 'Salman Salman' again .. ");
			Thread.Sleep(20000);
			Console.Write("\n  Starting client 'Salman Salman' again ..");
			c3 = Process.Start("Client.exe", "\"Salman Salman\"");
			Console.Write("\n  Please check client 'Salman Salman' received messages\n\n");
		}

#if (TEST_TEST)
		static void Main(string[] args) {
			Console.Title = "Test Executive - Project Deminstration";
			Startup();
			Demo();
			DemoExtra();
		}
#endif
	}

	public static class Exts {
		public static void Title(this string s, char underline = '-') {
			Console.Write("\n  {0}", s);
			Console.Write("\n {0}\n", new string(underline, s.Length + 2));
		}
	}
}
