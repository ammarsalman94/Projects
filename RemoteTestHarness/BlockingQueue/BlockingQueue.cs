/////////////////////////////////////////////////////////////////////////////
//  BlockingQueue.cs - Blocking queue implementation 											 //
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
 *   This module implements a Blocking Queue which can be used by multiple
 *   threads and ensures mutual exclusion. It supports generic types and so
 *   it can be used on anything.
 * 
 *   Public Interface
 *   ----------------
 *   BlockingQ<string> queue = new BlockingQ<string>();
 *   queue.enQ("some string");
 *   string s = queue.deQ();
 * 
 */
/*
 *   Build Process
 *   -------------
 *   - Required files:   BlockingQueue.cs 
 *   - Compiler command: csc BlockingQueue.cs
 * 
 *   Maintenance History
 *   -------------------
 *   ver 1.0 : 9 November 2016
 *     - first release
 * 
 */

using System;
using System.Collections.Generic;
using System.Threading;

namespace BlockingQueue {
	/// <summary>Blocking queue for multi-threaded environments</summary>
	public class BlockingQ<T> {
		/// <summary>Uses C# normal queue</summary>
		private Queue<T> Q;

		/// <summary>Returns an instance of BlockingQ</summary>
		public BlockingQ() {
			Q = new Queue<T>();
		}

		/// <summary>Enqueue an element to the queue</summary>
		/// <param name="t">Specifies the element to enqueue</param>
		/// <returns>Boolean indicating whether the enqueue was
		/// successful or not</returns>
		public bool enQ(T t) {
			lock (Q) {
				try {
					Q.Enqueue(t);
					Monitor.Pulse(Q);
					return true;
				} catch (Exception) {
					return false;
				}
			}
		}

		/// <summary>Dequeues an element from the queue</summary>
		/// <returns>The dequeued element</returns>
		public T deQ() {
			lock (Q) {
				while (Q.Count == 0)
					Monitor.Wait(Q);
				T t = Q.Dequeue();
				return t;
			}
		}

	}

	/// <summary>Test stub for the BlockingQ class</summary>
	class Program {
#if (TEST_BLOCKINGQUEUE)
		static void Main(string[] args)
        {
            BlockingQ<string> queue = new BlockingQ<string>();
            Console.WriteLine("\n Press enter to terminate ");
            Console.WriteLine("\n--------------------------");
            
            new Thread(() => {  queue.enQ("Thread1 is adding a message"); }).Start();
            new Thread(() => { for (int i = 0; i < 10; i++) { queue.enQ("Thread2 is adding a message"); } }).Start();
            new Thread(() => { while (true) { queue.deQ(); Console.WriteLine(" By Thread3");  } }).Start();
            new Thread(() => { while (true) { queue.deQ(); Console.WriteLine(" By Thread4"); } }).Start();
            new Thread(() => { Console.Read(); Environment.Exit(0); }).Start();

        }
#endif
	}
}
