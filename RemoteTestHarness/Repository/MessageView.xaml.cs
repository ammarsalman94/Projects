/////////////////////////////////////////////////////////////////////////////
//  MessageView.xaml.cs - Message viewer for repository 									 //
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
 *   This module creates a user-friendly message view box for the repository
 * 
 *   Public Interface
 *   ----------------
 *   MessageView view = new MessageView(msg);
 *   view.Show();
 * 
 */
/*
 *   Build Process
 *   -------------
 *   - Required files:   MessageView.xaml.cs
 *   - Compiler command: csc MessageView.xaml.cs
 * 
 *   Maintenance History
 *   -------------------
 *   ver 1.0 : 17 November 2016
 *     - first release
 * 
 */

using System;
using System.Text;
using System.Windows;
using System.Windows.Documents;
using Communication;
using System.Diagnostics;

namespace Repository {
	/// <summary>
	/// Interaction logic for MessageView.xaml
	/// </summary>
	public partial class MessageView : Window {
		public MessageView(Message msg) {
			InitializeComponent();
			StringBuilder message = new StringBuilder();

			Hyperlink link = new Hyperlink();
			link.IsEnabled = true;
			link.Inlines.Add(msg.Source);
			link.NavigateUri = new Uri(msg.Source);
			link.RequestNavigate += (sender, args) => Process.Start(args.Uri.ToString());

			Paragraph p = new Paragraph();
			p.Inlines.Add(new Bold(new Run(" From:\t")));
			p.Inlines.Add(link);
			p.Inlines.Add(new Bold(new Run("\n To:\t")));
			p.Inlines.Add(new Run(msg.Destination));
			p.Inlines.Add(new Bold(new Run("\n Type:\t")));
			p.Inlines.Add(new Run(msg.command.ToString()));
			p.Inlines.Add(new Bold(new Run("\n\n Body:\n\t")));
			p.Inlines.Add(new Run(msg.Body.Replace("\n", "\n\t")));

			RichTxtBox.Document.Blocks.Add(p);
		}
	}
}
