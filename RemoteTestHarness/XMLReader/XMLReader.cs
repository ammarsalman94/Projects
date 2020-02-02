/////////////////////////////////////////////////////////////////////////////
//  XMLReader.cs - demonstrate parsing XML test requests						       //
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
 *   This module demonstrates parsing XML file and returning the required 
 *   information to process the test request indicated by the XML file.
 * 
 *   Public Interface
 *   ----------------
 *   XMLReader xReader = new XMLReader();
 *   xReader.ProcessXMLFile(".\\TestRequest.XML);
 *   xReader.ProcessXMLString(XMLString);
 * 
 */
/*
 *   Build Process
 *   -------------
 *   - Required files:   XMLReader.cs 
 *   - Compiler command: csc XMLReader.cs
 * 
 *   Maintenance History
 *   -------------------
 *   ver 2.0 : 15 November 2016
 *		 - second release:
 *				- Minor fixes and imporvements
 *				- Support for parsing XML in form of a string
 *   ver 1.0 : 05 October 2016
 *     - first release
 * 
 */

using System;
using System.Collections.Generic;
using System.Linq;
using System.IO;
using System.Xml.Linq;

namespace XMLProcessing {
	/// <summary>Parses XML test requests and returns the required information for testing</summary>
	public class XMLReader {
		/* ----------------------< Fields and Properties Declarations Region >---------------------- */
		#region
		/// <summary>Holds the parsed XML document</summary>
		private XDocument xDoc { get; set; }

		/// <summary>Specifies the request's name</summary>
		public string TestRequest { get; set; }

		/// <summary>Specifies the Author</summary>
		public string Author { get; set; }

		/// <summary>Holds Test Drivers for each specified test</summary>
		public IDictionary<string, List<string>> TestDrivers { get; set; }
		/// <summary>Holds the required Libraries for each specified test</summary>
		public IDictionary<string, List<string>> Libraries { get; set; }

		#endregion
		/* -------------------< End of Fields and Properties Declarations Region >------------------ */

		/// <summary>Returns new instance of XMLReader</summary>
		public XMLReader() {
			xDoc = new XDocument();
			TestDrivers = new Dictionary<string, List<string>>();
			Libraries = new Dictionary<string, List<string>>();
		}

		/* ----------------------------------< Processing Region >---------------------------------- */
		#region

		/// <summary>Processes XML file and sets-up the required files and the author name</summary>
		/// <param name="XMLPath">Specifies XML file path to process</param>
		public bool ProcessXMLFile(string XMLPath) {
			// Check if file exists
			if (File.Exists(XMLPath)) {
				TestDrivers.Clear();
				Libraries.Clear();
				TestRequest = "";
				Author = "";
				try {
					// load the XML
					xDoc = XDocument.Load(XMLPath);

					TestRequest = xDoc.Descendants("testRequest").First().Attribute("name").Value;
					// set the author to the author indicated by the XML
					Author = xDoc.Descendants("author").First().Value;

					foreach (var Test in xDoc.Descendants("test")) {
						TestDrivers.Add(Test.Attribute("name").Value, new List<string>());
						Libraries.Add(Test.Attribute("name").Value, new List<string>());
						// get the Test Drivers indicated by the XML 
						foreach (var node in Test.Descendants("testDriver"))
							TestDrivers[Test.Attribute("name").Value].Add(node.Value);
						// get the required Libraries indicated by the XML
						foreach (var node in Test.Descendants("library"))
							Libraries[Test.Attribute("name").Value].Add(node.Value);
					}

					return true;

				} catch (Exception) {
					Console.Write("\n  A problem occured while processing {0}", Path.GetFileName(XMLPath));
					return false;
				}

			} else {
				Console.Write("\n  Unable to find {0}. File does not exist", Path.GetFileName(XMLPath));
				return false;
			}
		}


		/// <summary>Processes XML represented by string and sets up the author and files information</summary>
		/// <param name="XML">Specifies the XML string to process</param>
		public bool ProcessXMLString(string XML) {
			try {
				// clear old XMLs information
				TestDrivers.Clear();
				Libraries.Clear();
				TestRequest = "";
				Author = "";
				// parse the text as XML
				xDoc = XDocument.Parse(XML);
				TestRequest = xDoc.Descendants("testRequest").First().Attribute("name").Value;
				// get the test author
				Author = xDoc.Descendants("author").First().Value;

				foreach (var Test in xDoc.Descendants("test")) {
					TestDrivers.Add(Test.Attribute("name").Value, new List<string>());
					Libraries.Add(Test.Attribute("name").Value, new List<string>());
					// get the Test Drivers indicated by the XML 
					foreach (var node in Test.Descendants("testDriver"))
						TestDrivers[Test.Attribute("name").Value].Add(node.Value);
					// get the required Libraries indicated by the XML
					foreach (var node in Test.Descendants("library"))
						Libraries[Test.Attribute("name").Value].Add(node.Value);
				}
				return true;

			} catch (Exception) {
				Console.Write("\n  A problem occured while processing the XML string");
				return false;
			}
		}

		#endregion
		/* -------------------------------< End of Processing Region >------------------------------ */

/* ---------------------< TEST STUB >--------------------- */
#if (TEST_XMLREADER)
		static void Main(string[] args) {
			XMLReader reader = new XMLReader();
			Console.Write("\n Processing XML Test Request from XML File");
			Console.Write("\n-------------------------------------------");
			reader.ProcessXMLFile(".\\test1.xml");
			
			Console.Write("\n  Author: {0}", reader.Author);
			foreach (var Test in reader.TestDrivers) {
				Console.Write("\n  Test Name: {0}", Test.Key);
				Console.Write("\n\tTest Drivers: ");
				foreach (string TestDriver in Test.Value)
					Console.Write("\n\t\t{0}", TestDriver);
				Console.Write("\n\tLibraries: ");
				foreach (string Library in reader.Libraries[Test.Key])
					Console.Write("\n\t\t{0}", Library);
			}

			XDocument xDox = XDocument.Load(".\\test1.xml");

			Console.Write("\n\n\n Processing XML Test Request from XML String");
			Console.Write("\n---------------------------------------------");
			reader.ProcessXMLString(xDox.ToString());

			
			Console.Write("\n  Author: {0}", reader.Author);
			foreach (var Test in reader.TestDrivers) {
				Console.Write("\n  Test Name: {0}", Test.Key);
				Console.Write("\n\tTest Drivers: ");
				foreach (string TestDriver in Test.Value)
					Console.Write("\n\t\t{0}", TestDriver);
				Console.Write("\n\tLibraries: ");
				foreach (string Library in reader.Libraries[Test.Key])
					Console.Write("\n\t\t{0}", Library);
			}
			Console.WriteLine();
		}
#endif

	}
}
