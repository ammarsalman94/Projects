/////////////////////////////////////////////////////////////////////////////
//  MessageView.xaml.cs - Message viewer for client			 									 //
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
 *   This module creates a user-friendly message view box for the client
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
using System.IO;
using Communication;
using System.Diagnostics;

namespace Client {
	/// <summary>
	/// Interaction logic for MessageView.xaml
	/// </summary>
	public partial class MessageView : Window {
		public MessageView(Message msg, string ResultPath) {
			InitializeComponent();
			StringBuilder message = new StringBuilder();
			Paragraph p = new Paragraph();
			p.Inlines.Add(new Bold(new Run(" From:\t")));
			Hyperlink link = new Hyperlink();
			link.IsEnabled = true;
			link.Inlines.Add(msg.Source);
			link.NavigateUri = new Uri(msg.Source);
			link.RequestNavigate += (sender, args) => Process.Start(args.Uri.ToString());

			p.Inlines.Add(link);

			p.Inlines.Add(new Bold(new Run("\n To:\t")));
			p.Inlines.Add(new Run(msg.Destination));
			p.Inlines.Add(new Bold(new Run("\n Type:\t")));
			p.Inlines.Add(new Run(msg.command.ToString()));
			p.Inlines.Add(new Bold(new Run("\n\n Body:\n\t")));

			if (msg.command == Message.Command.Results) {
				Hyperlink fileLink = new Hyperlink();
				fileLink.IsEnabled = true;
				string filePath = Path.Combine(@ResultPath, msg.Body);
				filePath = Path.GetFullPath(filePath);
				fileLink.Inlines.Add(msg.Body);
				fileLink.NavigateUri = new Uri(filePath);
				try {
					string content = File.ReadAllText(filePath);
					fileLink.RequestNavigate += (sender, args) => Process.Start(filePath);
					p.Inlines.Add(fileLink);
					p.Inlines.Add(new Bold(new Run("\n Result Log:\n")));
					p.Inlines.Add(new Run(content));
				} catch (Exception) {
					p.Inlines.Add(fileLink);
					p.Inlines.Add(new Bold(new Run("\n< Unable to open the specified file" +
						". Please loop it up in \"" + ResultPath + "\"")));
				}
			} else
				p.Inlines.Add(new Run(msg.Body.Replace("\n", "\n\t")));

			RichTxtBox.Document.Blocks.Add(p);
		}
	}
}
