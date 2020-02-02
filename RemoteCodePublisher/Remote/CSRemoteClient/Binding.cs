using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using System.Threading;
using System.Windows;
using System.ComponentModel;



namespace CSRemoteClient {
  public partial class MainWindow : Window, INotifyPropertyChanged {
    private Shim shim;
    private Thread thread;
    private bool receiving;
    private ulong port;
    private bool Listening;
    private bool ConnectedToServer;
    private ulong serverPort;
    private string serverAddress;
    private string serverDirectories;
    private bool receivedDirectories; 

    private string workingDirectory;

    private string ServerFullAddress;
    private string _thisFullAddress;
    public string ThisFullAddress {
      get { return _thisFullAddress; }
      set {
        _thisFullAddress = value;
        OnPropertyChanged("ThisFullAddress");
      }
    }

    public string TitleLabel {
      get { return ThisFullAddress + " - Directory: " + workingDirectory; }
    }

    private string _labelLocalFiles;
    public string LabelLocalFiles {
      get { return _labelLocalFiles; }
      set {
        _labelLocalFiles = value;
        OnPropertyChanged("LabelLocalFiles");
      }
    }


    private string _statusText;
    public string StatusText {
      get { return _statusText; }
      set {
        _statusText = value;
        OnPropertyChanged("StatusText");
      }
    }


    StringBuilder logger;
    public string LogString {
      get { return logger.ToString(); }
    }

    public event PropertyChangedEventHandler PropertyChanged;
    void OnPropertyChanged(string PropertyName) {
      PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(PropertyName));
    }

    void AppendLogger(string toAppend) {
      logger.Append(toAppend).Append("\n");
      OnPropertyChanged("LogString");
    }
  }
}
