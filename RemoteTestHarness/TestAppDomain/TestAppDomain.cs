/////////////////////////////////////////////////////////////////////////////
//  TestAppDomain.cs - demonstrate processing an assembly									 //
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
 *   This module demonstrates testing an assembly and returning the test log
 * 
 *   Public Interface
 *   ----------------
 *   TestAppDomain Tester = new TestAppDomain();
 *   Tester.execute(".\Assembly.dll");
 * 
 */
/*
 *   Build Process
 *   -------------
 *   - Required files:   TestAppDomain.cs 
 *   - Compiler command: csc TestAppDomain.cs
 * 
 *   Maintenance History
 *   -------------------
 *   ver 2.0 : 15 November 2016
 *		 - second release:
 *				- Major fixes and imporvements
 *				- Support for testing only the Test() method if it exists
 *   ver 1.0 : 04 October 2016
 *     - first release
 * 
 */

using System;
using System.IO;
using System.Linq;
using System.Reflection;
namespace RemoteTestHarness {
	public class TestAppDomain : MarshalByRefObject, ITest {

		/* ----------------------< Fields & Properties Declarations Region >---------------------- */
		#region
		// -----< reporting variables >-----
		/// <summary>Holds the log for the testing operation</summary>
		string log;
		/// <summary>Specifies whether the test was successful or not</summary>
		bool testResult;
		/// <summary>Specifies the target assembly's name</summary>
		string AssemblyName;

		// -----< execution essential variables >-----
		/// <summary>Specifies the target assembly's path</summary>
		public string path { get; set; }
		/// <summary>Used to capture the execution output of the assembly functions</summary>
		StringWriter writer = new StringWriter();

		#endregion
		/* -------------------< End of Fields & Properties Declarations Region >------------------ */

		/* -----------------------------------< Constructors >------------------------------------ */
		/// <summary>Returns instance of TestAppDomain</summary>
		public TestAppDomain() { path = ""; log = ""; testResult = true; }
		/// <summary>Returns instance of TestAppDomain</summary>
		/// <param name="testFPath">Specifies the target assembly's path</param>
		public TestAppDomain(string testFPath) { path = testFPath; log = ""; testResult = true; }


		/* ------------------------< Implementing ITest Interface Region >------------------------ */
		#region
		// -----< ITest implemented methods >-----
		/// <summary>Obtains test result for the loaded assembly</summary>
		/// <returns>Boolean (true or false) indicating whether the test was passed or not</returns>
		public bool test() { return testResult; }
		/// <summary>Obtains the log for the performed test</summary>
		/// <param name="detailed">Indicates whether a detailed 
		/// log is requested or not</param>
		/// <returns>Returns short or detailed log based on 'detailed' parameter</returns>
		public string getLog(bool detailed = false) {
			if (detailed)
				return "Test Result for " +
							AssemblyName + " : " +
							testResult.passed() + "\n" + log;
			return "Test Result for " + AssemblyName +" : " + testResult.passed();
		}
		#endregion
		/* ---------------------< End of Implementing ITest Interface Region >-------------------- */



		/* -----------------------< Processing & Testing Assemblies Region >---------------------- */
		#region
		// -----< test the specified assembly >-----
		/// <summary>Starts testing the specified assembly</summary>
		public void execute() {
			var originalConsoleOut = Console.Out;
			Console.SetOut(writer);
			Assembly assembly;

			// using Assembly.LoadFrom to load the DLL with ALL OF ITS DEPENDANCIES
			assembly = Assembly.LoadFrom(path);
			AssemblyName = assembly.GetName().Name;
			// test all classes retrieved from the current assembly 
			foreach (Type type in assembly.GetTypes()) {
				testType(type);
			}
			// restore Console.Out
			Console.SetOut(originalConsoleOut);
			
		}

		// ---------< THE FOLLOWING METHODS ARE USED TO REDUCE LOOPS COMPLEXITY >---------

		// -----< test a specified class from the assembly >-----
		/// <summary>Tests classes found in the assembly</summary>
		/// <param name="type">Specifies the class to test</param>
		private void testType(Type type) {
			// add loging information
			log += ("  Testing Class " + type).title(true) + "\n";
			

			MethodInfo method = type.GetMethod("Test");
			Console.Write("\n  {0}  {1}", method, type);
			if (method != null) {
				log += "\n  ---< Testing " + method + " >---\n";
				log += "\n  Output:\n\t";
				try {
					object instance = Activator.CreateInstance(type);
					method.Invoke(instance, null);
					writer.Flush();
					string s = writer.GetStringBuilder().ToString();
					log += s.Replace("\n", "\n\t");
					log += "\n  Test Succeeded\n";
				} catch (Exception ex) {
					testResult = false;
					string s = (ex.InnerException != null) ? ex.InnerException.Message : "";
					log += "\n  Test Failed\n\n  Details: " + s + "\n";
				}
				return;
			}
			// ask for constructors test
			log += "  Testing Constructors".title();
			testConstructors(type.GetConstructors());
			// ask for public methods test
			log += "  Testing Public Methods".title();
			testMethods(type, BindingFlags.Public);
			// ask for non public methods test
			log += "  Testing Non-Public Methods".title();
			testMethods(type, BindingFlags.NonPublic);
		}

		// -----< test constructors of the class >-----
		/// <summary>Tests constructors found in the class specified by the type</summary>
		/// <param name="constructors">Specifies paramet list of the calling class</param>
		private void testConstructors(ConstructorInfo[] constructors) {
			// a constructor might need parameters to test it
			object[] parameters;
			// for every constructor
			foreach (ConstructorInfo constructor in constructors) {
				log += "\n  ---< Testing " + constructor + " >---\n";
				log += "\n  Output:\n\t";
				// get the parameters needed for the constructor
				parameters = generateParameters(constructor.GetParameters());
				try {
					// try calling the constructor 
					object foo = constructor.Invoke(parameters);
					writer.Flush();
					string s = writer.GetStringBuilder().ToString();
					if (foo == null) log += "  <No Output>";
					else log += foo.ToString().Replace("\n", "\n\t");
					log += s.Replace("\n", "\n\t");
					log += "\n  Test Succeeded\n";
				} catch (Exception ex) {
					testResult = false;
					string s = (ex.InnerException != null) ? ex.InnerException.Message : "";
					log += "\n  Test Failed\n\n  Details: " + s + "\n";
				}
			}
		}

		// -----< test methods of the class >-----
		// the process is the same we used in the constructors testing
		/// <summary>Tests methods found in the class specified by the type</summary>
		/// <param name="caller">Specifies the calling class of which methods are to be tested</param>
		/// <param name="flags">Specifies which methods to test (eg. public methods)</param>
		private void testMethods(Type caller, BindingFlags flags) {
			object[] parameters; object instance;
			foreach (MethodInfo method in caller.GetMethods(BindingFlags.Instance | flags)) {
				log += "\n  ---< Testing " + method + " >---\n";
				log += "\n  Output:\n\t";
				parameters = generateParameters(method.GetParameters());
				instance = Activator.CreateInstance(caller);
				try {
					object foo = method.Invoke(instance, parameters);
					writer.Flush();
					string s = writer.GetStringBuilder().ToString();
					if (foo == null) log += "  <No Output>";
					else log += foo.ToString().Replace("\n", "\n\t");
					log += s.Replace("\n", "\n\t");
					log += "\n  Test Succeeded\n";
				} catch (Exception ex) {
					testResult = false;
					string s = (ex.InnerException != null) ? ex.InnerException.Message : "";
					log += "\n  Test Failed\n\n  Details: " + s + "\n";
				}
			}
		}

		// -----< generate paramters to test constructors/methods >-----
		/// <summary>Generates default parameters for the calling constructor/method</summary>
		/// <param name="parameters">Specifies list of parameters to generate according to</param>
		/// <returns>Returns default values for the specified parameters</returns>
		private object[] generateParameters(ParameterInfo[] parameters) {
			object[] obj = new object[parameters.Count()];
			int i = 0;
			foreach (ParameterInfo parameter in parameters) {
				if (parameter.ParameterType.GetConstructor(Type.EmptyTypes) == null) {
					if (parameter.ParameterType == typeof(string)) obj[i] = "";
					else if (parameter.ParameterType == typeof(string[])) { string[] temp = { "", "" }; obj[i] = temp; } else obj[i] = parameter.DefaultValue;
				} else
					obj[i] = Activator.CreateInstance(parameter.ParameterType);
				++i;
			}
			return obj;
		}

		#endregion
		/* --------------------< End of Processing & Testing Assemblies Region >------------------ */


		/* -------------------< TEST STUB >------------------- */
#if (TEST_APPDOMAIN)
		static void Main(string[] args) {
			string path = @".\TestHarnessTestLibrary.dll";
			if (File.Exists(path)) {
				TestAppDomain Tester = new TestAppDomain(path);
				Tester.execute();
				Console.Write("\n Using test() to get result: {0}", Tester.test());
				Console.Write("\n Using getLog() to get short summary:\n  {0}", Tester.getLog());
				Console.Write("\n\n Using getLog(true) to get detailed summary:\n{0}", Tester.getLog(true));
			}
		}
#endif

	}

	/* -----------------------< Extension Methods Region >---------------------- */
	#region
	// -----< extension methods for logging purposes >-----
	static public class ExtensionM {
		static public string title(this string astring, bool mainTitle = false, char underline = '-') {
			astring = new string(underline, 10) + "< " + astring + " >" + new string(underline, 10) + "\n";
			string s = new string(underline, astring.Length - 1) + "\n";
			if (mainTitle) return s + s + astring + s + s;
			else return s + astring + s;
		}
		static public string passed(this bool abool) {
			if (abool) return "Passed";
			else return "Failed";
		}
	}
	#endregion
	/* --------------------< End of Extension Methods Region >------------------ */
}
