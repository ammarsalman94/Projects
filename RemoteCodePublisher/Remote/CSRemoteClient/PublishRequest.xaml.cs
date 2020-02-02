using System;
using System.Windows;
using System.Windows.Controls;
using System.ComponentModel;

namespace CSRemoteClient {
  /// <summary>
  /// Interaction logic for PublishRequest.xaml
  /// </summary>
  public partial class PublishRequest : Window, INotifyPropertyChanged {
    public PublishRequest(string Directories) {
      InitializeComponent();
      if (Directories == null) throw new ArgumentNullException();
      foreach (string dir in Directories.Split('\n'))
        lstDirectories.Items.Add(dir);
      DataContext = this;
      IISPublishCheck = false;
      AutoDownloadCheck = false;
    }

    private bool _IISPublishCheck;
    public bool IISPublishCheck {
      get { return _IISPublishCheck; }
      set {
        _IISPublishCheck = value;
        OnPropertyChanged("IISPublishCheck");
      }
    }

    private bool _AutoDownloadCheck;
    public bool AutoDownloadCheck {
      get { return _AutoDownloadCheck; }
      set {
        _AutoDownloadCheck = value;
        OnPropertyChanged("AutoDownloadCheck");
      }
    }

    private string _PublishDirectory;
    public string PublishDirectory { get { return _PublishDirectory; }  }


    private void lstDirectories_SelectionChanged(object sender, SelectionChangedEventArgs e) {
      _PublishDirectory = lstDirectories.SelectedItem.ToString();
    }

    private void Publish_Clicked(object sender, RoutedEventArgs e) {
      if (lstDirectories.SelectedIndex == -1) {
        if (MessageBox.Show("You have not selected any directory to publish.\nWould you like to select directory?",
          "No Publish Directory Selected", MessageBoxButton.YesNo, MessageBoxImage.Error, MessageBoxResult.Yes) == MessageBoxResult.Yes) {
          return;
        } else {
          Cancel_Clicked(this, new RoutedEventArgs());
          return;
        }
      }

      string displaymsg = "Requesting server to publish files in:\n" +
        PublishDirectory + "\n\nOptions:\nPublish to IIS: " + IISPublishCheck;
      displaymsg += "\nAutomatic Download: " + AutoDownloadCheck + "\n\nWould you like to continue?";

      if (MessageBox.Show(displaymsg, "Confirm Publish Request",
        MessageBoxButton.YesNo, MessageBoxImage.Question, MessageBoxResult.Yes) == MessageBoxResult.Yes) {
        DialogResult = true;
        Hide();
      }
    }

    private void Cancel_Clicked(object sender, RoutedEventArgs e) {
      DialogResult = false;
      Hide();
    }

    public event PropertyChangedEventHandler PropertyChanged;
    void OnPropertyChanged(string PropertyName) {
      PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(PropertyName));
    }

    
  }
}
