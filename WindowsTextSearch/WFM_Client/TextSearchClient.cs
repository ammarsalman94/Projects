/////////////////////////////////////////////////////////////////////////////////////////
// TextSearchClient.cs: Defines client to test TextSearch and FileManager components   //
//                                                                                     //
// Platform:     Dell Inspiron 5520 - Windows 10 Professional, Visual Studio 2015      //
// Application:  Distributed Objects - CSE-775, Project 1, Spring 2017                 //
// Author:       Ammar Salman, Syracuse University                                     //
//               (313) 788-4694, hoplite.90@hotmail.com                                //
/////////////////////////////////////////////////////////////////////////////////////////

using System;
using System.Threading;
using FileMgrLib;
using TextSearchComLib;
using System.IO;

namespace WFM_Client {
	class TextSearchClient {
		static ITextSearch txtSearch;
		static int counter = 0;

		static void ShowUsage() {
			Console.Write("\r\n Usage: --path C:\\test --search \"some string\" [ --patterns \"*.txt *.h\" -subs ]");
			Console.Write("\r\n  --path:     specifies path to working directory - MUST BE SET");
			Console.Write("\r\n  --search:   specifies the string to look for in the files - MUST BE SET");
			Console.Write("\r\n  --patterns: specifies the patterns as one string (default *.*)");
			Console.Write("\r\n  -subs:      if specified all sub-directories will be included in the search\r\n\r\n");
		}

		static bool ProcessCommandLineArgs(string[] args, ref string wPath, 
			ref string toSearch, ref string Patterns, ref bool incDirs) {
			wPath = "dummy";
			toSearch = "dummy";
			Patterns = "*.*";
			try {
				for (int i = 0; i < args.Length; i++) {
					if (args[i].Equals("--path")) wPath = args[i + 1];
					if (args[i].Equals("--patterns")) Patterns = args[i + 1];
					if (args[i].Equals("--search")) toSearch = args[i + 1];
					if (args[i].Equals("-subs")) incDirs = true;
				}
				if (wPath.Equals("dummy") || toSearch.Equals("dummy") || !Directory.Exists(wPath)) return false;
			} catch {
				return false;
			}
			return true;
		}

		static void ThreadProc() {
			"Starting Thread To Get Results From TextSearch".Title('-', 25);
			string file, searched;
			bool res;
			// blocking call 
			while (true) {
				txtSearch.GetResult(out file, out searched, out res);
				if (file.Equals("quit")) {
					Console.WriteLine($"\r\n\r\n  Received quit message. Terminating CSharp receiver thread ... Processed {counter} file(s)\r\n\r\n");
					break;
				}
				Console.Write($"\r\n  {(++counter).ToString().PadLeft(2)} - Searched for: '{searched}' -- Found: {res.ToString().PadRight(5)} - File: {file}");
			}
		}

		static int Main(string[] args) {
			string wDir = "", sString = "", Patterns = ""; bool incDirs = false;
			if (!ProcessCommandLineArgs(args, ref wDir, ref sString, ref Patterns, ref incDirs)) {
				ShowUsage();
				return 1;
			}

			try {
				"Starting CSharpClient for TextSearch & FileManager Components".Title('=', 22);
				Console.Write("\r\n  Creating proxy to ITextSearch COM ATL Library");
				txtSearch = new TextSearch();
				Console.Write("\r\n  Creating proxy to IFileManager COM ATL Library");
				IFileManager fmgr = new FileManager();
				Console.Write("\r\n  Passing the instance of TextSearch to FileManager instance");
				fmgr.ConfigureSearcher(txtSearch);
				Console.Write($"\r\n  Setting working directory of IFileManager to: '{wDir}' and filter pattern to: '{Patterns}'");
				fmgr.SetWorkingDirectory(wDir, Patterns, incDirs);
				Console.Write($"\r\n  Setting search string to '{sString}'");
				fmgr.SetSearchString(sString);
				Console.Write("\r\n  FileManager instance is now sending the found files to TextSearch instance");
				fmgr.PerformOperations();
				Console.Write("\r\n  Ordering FileManager to terminate its ITextSearch instance after it has finished by sending 'quit' message\r\n");
				fmgr.TerminateSearcher();

				Console.Write("\r\n\r\n  Files found by FileManager: ");
				int c = 0;
				while (fmgr.good()) {
					string file = "";
					fmgr.GetFile(out file);
					// last entry is always null in filemanager
					if (file == null) break;
					Console.Write($"\r\n  {(++c).ToString().PadLeft(2)} - '{file}'");
				}
				Console.WriteLine();
				Thread t = new Thread(ThreadProc);
				t.Start();
				t.Join();
			} catch(Exception ex) {
				Console.WriteLine($"\r\n\r\n  Error thrown. Details: {ex.Message}");
				return 1;
			}
			return 0;
		}
	}

	static class exts {
		public static void Title(this string str, char delimeter = '=', int append = 3) {
			str = new string(' ', append) + str + new string(' ', append);
			string temp = "".PadLeft(str.Length, delimeter);
			Console.WriteLine("\r\n  {0}", str);
			Console.WriteLine("  {0}", temp);
		}
	}
}
