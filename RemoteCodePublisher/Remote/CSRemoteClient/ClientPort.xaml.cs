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
  /// Interaction logic for ClientPort.xaml
  /// </summary>
  public partial class ClientPort : Window, INotifyPropertyChanged {
    public ClientPort(ulong port) {
      InitializeComponent();
      DataContext = this;
      Port = port;
      FocusManager.SetFocusedElement(this, txtPort);
      Keyboard.Focus(txtPort);
      txtPort.SelectAll();
    }

    private ulong _port;
    public ulong Port {
      get { return _port; }
      set {
        _port = value;
        OnPropertyChanged("Port");
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

    private void txtPort_GotFocus(object sender, RoutedEventArgs e) {
      ((TextBox)sender).SelectAll();
    }

    private void txtPort_GotKeyboardFocus(object sender, KeyboardFocusChangedEventArgs e) {
      ((TextBox)sender).SelectAll();
    }

    private void txtPort_GotMouseCapture(object sender, MouseEventArgs e) {
      ((TextBox)sender).SelectAll();
    }
  }
}
