using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Shapes;

using System.ComponentModel;

namespace CSRemoteClient {
  /// <summary>
  /// Interaction logic for ServerAddress.xaml
  /// </summary>
  public partial class ServerAddress : Window, INotifyPropertyChanged {
    public ServerAddress(string address, ulong port) {
      InitializeComponent();
      DataContext = this;
      Address = address;
      Port = port;
      FocusManager.SetFocusedElement(this, txtAddress);
      Keyboard.Focus(txtAddress);
    }

    private ulong _port;
    public ulong Port {
      get { return _port; }
      set {
        _port = value;
        OnPropertyChanged("Port");
      }
    }

    private string _address;
    public string Address {
      get { return _address; }
      set {
        _address = value;
        OnPropertyChanged("Address");
      }
    }

    private void Button_Click(object sender, RoutedEventArgs e) {
      DialogResult = true;
      Hide();
    }

    public event PropertyChangedEventHandler PropertyChanged;
    void OnPropertyChanged(string PropertyName) {
      PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(PropertyName));
    }

    private void TextBox_KeyUp(object sender, KeyEventArgs e) {
      if (e.Key == Key.Enter)
        Button_Click(this, new RoutedEventArgs());
    }

    private void txtBox_GotFocus(object sender, RoutedEventArgs e) {
      ((TextBox)sender).SelectAll();
    }

    private void txtBox_GotKeyboardFocus(object sender, KeyboardFocusChangedEventArgs e) {
      ((TextBox)sender).SelectAll();
    }

    private void txtBox_GotMouseCapture(object sender, MouseEventArgs e) {
      ((TextBox)sender).SelectAll();
    }
  }
}
