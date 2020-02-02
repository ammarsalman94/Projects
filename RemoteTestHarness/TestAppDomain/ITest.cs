/////////////////////////////////////////////////////////////////////////////
//  ITest.cs - Interface with test() and getLog() methods									 //
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
 *   This module demonstrates an interface that holds two methods:
 *   test(): returns boolean (True or False) indicating whether test was passed
 *   getLog(): returns short/detailed log for the test
 * 
 *   Public Interface
 *   ----------------
 *   test()
 *   getLog()
 *   getLog(true)
 * 
 */
/*
 *   Build Process
 *   -------------
 *   - Required files:   ITest.cs 
 *   - Compiler command: csc ITest.cs
 * 
 *   Maintenance History
 *   -------------------
 *   ver 1.0 : 04 October 2016
 *     - first release
 * 
 */

namespace RemoteTestHarness
{
    public interface ITest
    {
        bool test();
        string getLog(bool detailed); 
    }
}