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
using System.Windows.Navigation;

using System.IO;
using System.Threading;

using System.ComponentModel;

namespace CSRemoteClient {
  /// <summary>
  /// Interaction logic for MainWindow.xaml
  /// </summary>
  public partial class MainWindow : Window, INotifyPropertyChanged {

    #region initialization and termination

    public MainWindow() {
      InitializeComponent();
      DataContext = this;
      port = 10000;
      serverPort = 9090;
      serverAddress = "localhost";
      ThisFullAddress = "localhost:" + port;
      ServerFullAddress = serverAddress + ":" + serverPort;
      logger = new StringBuilder();
      workingDirectory = "";
      LabelLocalFiles = "Files in current working directory: "; 
    }

    private void Window_Loaded(object sender, RoutedEventArgs e) {
      shim = new Shim();
      StatusText = "Initialized Client";
      AppendLogger(StatusText);
      receiving = true;
      thread = new Thread(ThreadProc);
      thread.Start();
    }

    private void Window_Closing(object sender, CancelEventArgs e) {
      receiving = false;
      shim.Dispose();
      try {
        Environment.Exit(0);
      } catch (Exception) { }
    }

    #endregion

    #region connections and directory setup

    private bool CheckConnections() {
      if (!Listening) {
        if (MessageBox.Show("Client is not listening on any port. You will not be able to receive any messages back.\nWould you like to start listening?",
          "Client Receiver Not Initialized", MessageBoxButton.YesNo, MessageBoxImage.Asterisk) == MessageBoxResult.Yes)
          ClientPort_Click(this, new RoutedEventArgs());
        if (!Listening) return false;
      }

      if (!ConnectedToServer) {
        if (MessageBox.Show("Client is not connected to any server.\nWould you like to connect before proceeding?",
          "Client Not Connected", MessageBoxButton.YesNo, MessageBoxImage.Asterisk) == MessageBoxResult.Yes)
          ServerAddress_Click(this, new RoutedEventArgs());
        if (!ConnectedToServer) return false;
      }
      return true;
    }

    private void ClientPort_Click(object sender, RoutedEventArgs e) {
      ClientPort cp = new ClientPort(port);
      if (cp.ShowDialog().Value) {
        port = cp.Port;
        if (shim.Listen(port)) {
          StatusText = "Client started listening on port " + port;
          ThisFullAddress = "localhost:" + port;
          Listening = true;
        } else {
          StatusText = "Client failed to start listening on port " + port;
          Listening = false;
        }
        AppendLogger(StatusText);
      }
      OnPropertyChanged("TitleLabel");
    }

    private void ServerAddress_Click(object sender, RoutedEventArgs e) {
      ServerAddress sa = new ServerAddress(serverAddress, serverPort);
      if (sa.ShowDialog().Value) {
        serverAddress = sa.Address;
        serverPort = sa.Port;
        ServerFullAddress = serverAddress + ":" + serverPort;
        if (shim.SendTestMessage(ServerFullAddress)) {
          ConnectedToServer = true;
          StatusText = "Successfully connected to server @ " + ServerFullAddress;
          AppendLogger(StatusText);
        } 
        else {
          ConnectedToServer = false;
          StatusText = "Failed to connect to server @ " + ServerFullAddress;
          AppendLogger(StatusText); 
        }
      }
    }

    private void WorkingDirectory_Click(object sender, RoutedEventArgs e) {
      System.Windows.Forms.FolderBrowserDialog browser = new System.Windows.Forms.FolderBrowserDialog();
      if (browser.ShowDialog() == System.Windows.Forms.DialogResult.OK) {
        workingDirectory = browser.SelectedPath;
        shim.SetWorkingDirectory(browser.SelectedPath);
        StatusText = "Working directory set to: " + browser.SelectedPath;
        AppendLogger(StatusText);
      }
      OnPropertyChanged("TitleLabel");
    }

    private void RefreshLocalList_Click(object sender, RoutedEventArgs e) {
      if (workingDirectory == "") {
        if (MessageBox.Show("Client working directory is not set.\nWould you like to set it first?",
          "Client Working Directory Not Available", MessageBoxButton.YesNo,
          MessageBoxImage.Asterisk, MessageBoxResult.Yes) == MessageBoxResult.Yes)
          WorkingDirectory_Click(this, new RoutedEventArgs());
        if (workingDirectory == "") return;
      }

      new Thread(() => {
        Dispatcher.Invoke(new Action(() => { lstLocalFiles.Items.Clear(); }));
        string[] foundfiles = Directory.GetFiles(workingDirectory, "*.*", SearchOption.AllDirectories);
        string[] filteredFiles = foundfiles.Where(file => (Path.GetExtension(file) == ".h"
                  || Path.GetExtension(file) == ".cpp" || Path.GetExtension(file) == ".cs"
                  || Path.GetExtension(file) == ".html" || Path.GetExtension(file) == ".htm")).ToArray();
        foreach (string file in filteredFiles)
          Dispatcher.Invoke(new Action(() => { lstLocalFiles.Items.Add(MakeRelativePath(workingDirectory, file)); }));
        LabelLocalFiles = "Files in current working directory: (found: " + filteredFiles.Length + " files)";
      }).Start();
    }

    #endregion

    private void ClearLogger_Click(object sender, RoutedEventArgs e) {
      logger.Clear();
      OnPropertyChanged("LogString");
    }

    #region messages to server buttons functions

    private void GetFileList_Click(object sender, RoutedEventArgs e) {
      if (!CheckConnections()) return;

      if (lstCategories.HasItems && lstCategories.SelectedIndex != -1) {
        AppendLogger("Sending Message (Type: GetFileList) to " + ServerFullAddress);
        shim.PostMessage(HttpMessageBuilder.FileListRequestMessage(ServerFullAddress, ThisFullAddress, (string)lstCategories.SelectedItem));
      }
      else if (lstCategories.HasItems) {
        MessageBox.Show("Please select category before attempting to get files list",
          "No Category Selected", MessageBoxButton.OK, MessageBoxImage.Hand);
      } else {
        if (MessageBox.Show("Category list is empty. Would you like to get category list from Remote Publisher?",
          "Empty Category List", MessageBoxButton.YesNo, MessageBoxImage.Question, MessageBoxResult.Yes) == MessageBoxResult.Yes)
          GetCategories_Click(this, new RoutedEventArgs());
      }
    }

    private void GetCategories_Click(object sender, RoutedEventArgs e) {
      if (!CheckConnections()) return;
      AppendLogger("Sending Message (Type: GetCategories) to " + ServerFullAddress);
      shim.PostMessage(HttpMessageBuilder.CategoryRequestMessage(ServerFullAddress, ThisFullAddress));
    }

    private void GetAllFiles_Click(object sender, RoutedEventArgs e) {
      if (!CheckConnections()) return;
      AppendLogger("Sending Message (Type: GetAllFiles) to " + ServerFullAddress);
      shim.PostMessage(HttpMessageBuilder.AllFilesRequestMessage(ServerFullAddress, ThisFullAddress));
    }

    private void SendFiles_Click(object sender, RoutedEventArgs e) {
      if (!CheckConnections()) return;
      var selectedFiles = lstLocalFiles.SelectedItems;
      new Thread(() => {
        foreach (var file in selectedFiles) {
          AppendLogger("Sending file \"" + file.ToString() + "\" to " + ServerFullAddress);
          shim.PostMessage(HttpMessageBuilder.SendFileMessage(ServerFullAddress, ThisFullAddress, file.ToString()));
        }
        Dispatcher.Invoke(new Action(() => { lstLocalFiles.SelectedItems.Clear(); }));
      }).Start();
    }


    private void Files_DownloadS_Click(object sender, RoutedEventArgs e) {
      if (lstFiles.SelectedIndex == -1) {
        return;
      }
      string filename = lstFiles.Items.GetItemAt(lstFiles.SelectedIndex).ToString();
      AppendLogger("Sending download file (" + filename + ") request to " + ServerFullAddress);
      shim.PostMessage(HttpMessageBuilder.GetFileRequestMessage(ServerFullAddress,
        ThisFullAddress, filename));
    }

    private void Files_DeleteS_Click(object sender, RoutedEventArgs e) {
      if (lstFiles.SelectedIndex == -1) {
        return;
      }
      string filename = lstFiles.Items.GetItemAt(lstFiles.SelectedIndex).ToString();

      AppendLogger("Sending delete file (" + filename + ") request to " + ServerFullAddress);
      shim.PostMessage(HttpMessageBuilder.DeleteFileRequestMessage(ServerFullAddress,
        ThisFullAddress, filename));
      lstFiles.Items.RemoveAt(lstFiles.SelectedIndex);

      for (int i = 0; i < lstAllFiles.Items.Count; ++i)
        if (lstAllFiles.Items[i].ToString() == filename) {
          lstAllFiles.Items.RemoveAt(i);
          break;
        }
    }

    private void AllFiles_DownloadS_Click(object sender, RoutedEventArgs e) {
      if (lstAllFiles.SelectedIndex == -1) {
        return;
      }
      foreach (var item in lstAllFiles.SelectedItems) {
        string filename = item.ToString();
        AppendLogger("Sending download file (" + filename + ") request to " + ServerFullAddress);
        shim.PostMessage(HttpMessageBuilder.GetFileRequestMessage(ServerFullAddress,
          ThisFullAddress, filename));
      }
    }

    private void AllFiles_DeleteS_Click(object sender, RoutedEventArgs e) {
      if (lstAllFiles.SelectedIndex == -1) {
        return;
      }
      foreach (var item in lstAllFiles.SelectedItems) {
        string filename = item.ToString();

        AppendLogger("Sending delete file (" + filename + ") request to " + ServerFullAddress);
        shim.PostMessage(HttpMessageBuilder.DeleteFileRequestMessage(ServerFullAddress,
          ThisFullAddress, filename));
        lstAllFiles.Items.RemoveAt(lstAllFiles.SelectedIndex);

        for (int i = 0; i < lstFiles.Items.Count; ++i)
          if (lstFiles.Items[i].ToString() == filename) {
            lstFiles.Items.RemoveAt(i);
            break;
          }
      }
    }

    async private void PublishRequest_Click(object sender, RoutedEventArgs e) {
      if (!CheckConnections()) return;
      if (!receivedDirectories) {
        receivedDirectories = false;
        StatusText = "Requesting server's directory list...";
        AppendLogger(StatusText);
        shim.PostMessage(HttpMessageBuilder.GetDirectoriesMessage(ServerFullAddress, ThisFullAddress));
        while (!receivedDirectories)
          await Task.Delay(100);
        StatusText = "Received server's directory list";
        AppendLogger(StatusText);
      }
      PublishRequest pReq = new PublishRequest(serverDirectories);
      if (pReq.ShowDialog().Value) {
        shim.PostMessage(HttpMessageBuilder.PublishRequestMessage(ServerFullAddress, ThisFullAddress, 
          pReq.PublishDirectory, pReq.IISPublishCheck, pReq.AutoDownloadCheck));
      }
    }

    private void GetPublishedWebpages_Click(object sender, RoutedEventArgs e) {
      if (!CheckConnections()) return;
      shim.PostMessage(HttpMessageBuilder.GetPublishedMessage(ServerFullAddress, ThisFullAddress));
    }

    private void DeleteWebpages_Click(object sender, RoutedEventArgs e) {

    }

    private void DownloadWebpages_Click(object sender, RoutedEventArgs e) {

    }


    #endregion

    #region receiving thread functionality

    private void ThreadProc() {
      while (receiving) {
        string msg = shim.GetMessage();
        HandleMessage(msg);
        if (msg.Contains("Quit")) break;
      }
    }

    void HandleMessage(string Message) {
      HttpMessage msg = HttpMessage.FromString(Message);
      if (msg == null) return;
      AppendLogger("Received Message (Type: " + msg.Type + ") from: " + msg.GetAttributeValue("FromAddr"));
      if(msg.Type == "Categories" && msg.Body != null) {
        Dispatcher.Invoke(new Action(() => { lstCategories.Items.Clear(); }));
        foreach (string Category in msg.Body.Split('\n'))
          Dispatcher.Invoke(new Action(() => { lstCategories.Items.Add(Category); }));
      }
      if (msg.Type == "FileList" && msg.Body != null) {
        Dispatcher.Invoke(new Action(() => { lstFiles.Items.Clear(); }));
        foreach (string Filename in msg.Body.Split('\n'))
          Dispatcher.Invoke(new Action(() => { lstFiles.Items.Add(Filename); }));
      }
      if (msg.Type == "AllFiles" && msg.Body != null) {
        Dispatcher.Invoke(new Action(() => { lstAllFiles.Items.Clear(); }));
        foreach (string Filename in msg.Body.Split('\n'))
          Dispatcher.Invoke(new Action(() => { lstAllFiles.Items.Add(Filename); }));
      }
      if (msg.Type == "Directories" && msg.Body != null) {
        receivedDirectories = true;
        serverDirectories = msg.Body;
      }
      if (msg.Type == "PublishedWebpages" && msg.Body != null) {
        string IIS_Server = serverAddress + ":" + msg.Attributes["Port"] + "/";
        Dispatcher.Invoke(new Action(() => { lstPublished.Items.Clear(); }));
        foreach (string webpage in msg.Body.Split('\n'))
          Dispatcher.Invoke(new Action(() => { lstPublished.Items.Add(IIS_Server + webpage); }));
      }
    }

    #endregion

   


    /// <summary>
    /// Creates a relative path from one file or folder to another.
    /// </summary>
    /// <param name="fromPath">Contains the directory that defines the start of the relative path.</param>
    /// <param name="toPath">Contains the path that defines the endpoint of the relative path.</param>
    /// <returns>The relative path from the start directory to the end path or <c>toPath</c> if the paths are not related.</returns>
    /// <exception cref="ArgumentNullException"></exception>
    /// <exception cref="UriFormatException"></exception>
    /// <exception cref="InvalidOperationException"></exception>
    public string MakeRelativePath(string fromPath, string toPath) {
      if (String.IsNullOrEmpty(fromPath)) throw new ArgumentNullException("fromPath");
      if (String.IsNullOrEmpty(toPath)) throw new ArgumentNullException("toPath");

      Uri fromUri = new Uri(fromPath);
      Uri toUri = new Uri(toPath);

      if (fromUri.Scheme != toUri.Scheme) { return toPath; } // path can't be made relative.

      Uri relativeUri = fromUri.MakeRelativeUri(toUri);
      string relativePath = Uri.UnescapeDataString(relativeUri.ToString());

      if (toUri.Scheme.Equals("file", StringComparison.InvariantCultureIgnoreCase)) {
        relativePath = relativePath.Replace(Path.AltDirectorySeparatorChar, Path.DirectorySeparatorChar);
      }
      relativePath = relativePath.Substring(relativePath.IndexOf('\\'));

      return relativePath;
    }

    private void lstPublished_MouseDoubleClick(object sender, MouseButtonEventArgs e) {
      if (lstPublished.SelectedIndex == -1) return;
      System.Diagnostics.Process.Start("http://"+lstPublished.SelectedItem.ToString());
    }
  }
}
