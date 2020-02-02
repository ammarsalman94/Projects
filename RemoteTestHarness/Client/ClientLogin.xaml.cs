using System;
using System.Linq;
using System.Windows;
using System.Windows.Controls;

namespace Client {
	/// <summary>
	/// Interaction logic for ClientLogin.xaml
	/// </summary>
	public partial class ClientLogin : Window {
		public ClientLogin() {
			InitializeComponent();
			string[] args = Environment.GetCommandLineArgs();
			if(args.Length == 2) {
				string ClientName = args[1];
				ClientGUI client = new ClientGUI(ClientName);
				client.Show();
				Hide();
			}
			else if (args.Length >= 3) {
				string ClientName = args[1];
				bool b; bool.TryParse(args[2], out b);
				string[] files;
				if (b) {
					files = args.Skip(3).ToArray();
					ClientGUI client = new ClientGUI(ClientName, b, files);
					client.Show();
				} else {
					ClientGUI client = new ClientGUI(ClientName);
					client.Show();
				}
				this.Hide();
			}
		}

		private void Button_Click(object sender, RoutedEventArgs e) {
			if (!txtClient.Text.Equals("")) {
				ClientGUI Client = new ClientGUI(txtClient.Text);
				Client.Show();
				Hide();
			} else {
				MessageBox.Show("Please specify client name", "Error", MessageBoxButton.OK, MessageBoxImage.Error);
			}
		}

		private void txtClient_KeyDown(object sender, System.Windows.Input.KeyEventArgs e) {
			if(e.Key==System.Windows.Input.Key.Enter)
				btnProceed.RaiseEvent(new RoutedEventArgs(Button.ClickEvent));
		}
	}
}
