///////////////////////////////////////////////////////////////////////////////////
// HttpMessageBuilder.cs : Defines HttpMessage class, and static class named     //
//                         HttpMessageBuilder                                    //
// ver 1.0                                                                       //
//                                                                               //
// Platform     : Dell Inspiron 5520, Windows 10 Pro, Visual Studio 2015         //
// Application  : CSE-687 OOD - Project 4                                        //
// Author       : Ammar Salman, EECS Department, Syracuse University             //
//                (313)-788-4694, hoplite.90@hotmail.com                         //
///////////////////////////////////////////////////////////////////////////////////
/* 
 * Package description: this package defines HttpMessage and static class
 * HttpMessageBuilder. HttpMessage is used on received messages as they
 * are received as strings. HttpMessage has a static method that returns
 * HttpMessage instance given a string value. This is useful for the GUI
 * to be able to handle the messages correctly.
 * 
 * HttpMessageBuilder is a static class that returns strings in form of
 * Http Messages. Those strings are pushed to the C++ client which creates
 * C++ HttpMessage instances and sends them to the server.
 * 
 */
/*
 * Public Interface:
 * =========================
 * HttpMessage.FromString(strMsg);
 * HttpMessage msg = new HttpMessage();
 * msg.AddAttribute("Type", "Value");
 * msg.RemoteAttribute("Type", "Value");
 * msg.Type = "TypeValue";
 * msg.Body = "Body";
 * 
 * HttpMessageBuilder.PublishRequestMessage(ToAddr, FromAddr, "C:\\Test", "C:\\Out", "*.h", true);
 * HttpMessageBuilder.AllFilesRequestMessage(ToAddr, FromAddr);
 * HttpMessageBuilder.CategoryRequestMessage(ToAddr, FromAddr);
 * HttpMessageBuilder.DeleteFileRequestMessage(ToAddr, FromAddr, "file.h");
 * HttpMessageBuilder.FileListRequestMessage(ToAddr, FromAddr, "HeaderFiles");
 * HttpMessageBuilder.GetFileRequestMessage(ToAddr, FromAddr, "file.h");
 * HttpMessageBuilder.SendFileMessage(ToAddr, FromAddr, "file.h");
 * 
 * Required Files:
 * =========================
 * HttpMessageBuilder.cs 
 * 
 * Build Proceedure:
 * =========================
 * devenv CSRemoteClient.csproj /rebuild
 * 
 * Maintainence History:
 * =========================
 * ver 1.0 04, April, 2017
 *   - first release
 * 
 */

using System.Text;
using System.Collections.Generic;

namespace CSRemoteClient {

  /// <summary>
  /// .NET HttpMessage class. Designed similar to the provided C++ version.
  /// </summary>
  public class HttpMessage {
    // Attributes are hashmap to prevent setting the same attribute twice
    private Dictionary<string, string> _Attributes;
    public Dictionary<string, string> Attributes { get { return _Attributes; } }

    private string _Body;
    /// <summary>
    /// Body automatically sets the "content-length" attribute to make it easier for use.
    /// </summary>
    public string Body {
      get { return _Body; }
      set {
        _Body = value;
        _Attributes["content-length"] = _Body.Length.ToString();
      }
    }

    /// <summary>
    /// Returns message type.
    /// </summary>
    public string Type {
      get { return Attributes["Type"]; }
      set { Attributes["Type"] = value; }
    }

    /// <summary>
    /// Returns new HttpMessage instance.
    /// </summary>
    public HttpMessage() {
      _Attributes = new Dictionary<string, string>();
    }

    /// <summary>
    /// Sets Http Message given attribute name to given value.
    /// </summary>
    /// <param name="AttributeName">The attribute name.</param>
    /// <param name="Value">The desired value.</param>
    public void SetAttribute(string AttributeName, string Value) {
      _Attributes[AttributeName] = Value;
    }

    /// <summary>
    /// Gets attribute value given its name
    /// </summary>
    /// <param name="AttributeName">The attribute name.</param>
    /// <returns>Value of attribute.</returns>
    public string GetAttributeValue(string AttributeName) {
      return _Attributes[AttributeName];
    }

    /// <summary>
    /// Remote attribute from attribute list
    /// </summary>
    /// <param name="AttributeName">Attribute name</param>
    /// <returns>Boolean indicating whether or not the attribute was removed</returns>
    public bool RemoveAttribute(string AttributeName) {
      if (_Attributes.ContainsKey(AttributeName)) {
        _Attributes.Remove(AttributeName);
        return true;
      }
      return false;
    }

    /// <summary>
    /// Extracts HttpMessage from string.
    /// </summary>
    /// <param name="HttpMessageString">String of an Http Message</param>
    public static HttpMessage FromString(string HttpMessageString) {
      if (HttpMessageString == null) return null;
      HttpMessage msg = new HttpMessage();

      string[] lines = HttpMessageString.Split(new char[] { '\n' }, System.StringSplitOptions.None);
      bool isBody = false;
      foreach(string line in lines) {
        if(line == "") {
          isBody = true;
          continue;
        }
        if (isBody) {
          msg.Body += line + '\n';
        }
        string Attribute, Value;
        int pos = line.IndexOf(':');
        if (pos == -1) continue;
        Attribute = line.Substring(0, pos).Trim();
        Value = line.Substring(pos + 1).Trim();
        msg.Attributes[Attribute] = Value;
      }
      return msg;
      
    }

  }

  public static class HttpMessageBuilder {
    /// <summary>
    /// Creates publish request message 
    /// </summary>
    /// <param name="ToAddr">Address and port of the receiver</param>
    /// <param name="FromAddr">Address and port of the sender</param>
    /// <param name="PublishDirectory">Directory of files to publish on the Remote Server</param>
    /// <returns>Http Message string that requests publish from Remote Publisher</returns>
    public static string PublishRequestMessage(string ToAddr, string FromAddr, string PublishDirectory, bool IIS, bool AutoDown) {
      StringBuilder sb = new StringBuilder();
      sb.Append("Type:Publish");
      sb.Append("\nToAddr:").Append(ToAddr);
      sb.Append("\nFromAddr:").Append(FromAddr);
      sb.Append("\nDirectory:").Append(PublishDirectory);
      sb.Append("\nIIS:").Append(IIS);
      sb.Append("\nAutoDown:").Append(AutoDown);
      string msgBody = "Publish Request Message Body";
      sb.Append("\ncontent-length:").Append(msgBody.Length);
      sb.Append("\n\n").Append(msgBody);
      return sb.ToString();

    }

    /// <summary>
    /// Creates category list request message
    /// </summary>
    /// <param name="ToAddr">Address and port of the receiver</param>
    /// <param name="FromAddr">Address and port of the sender</param>
    /// <returns>Http Message string that requests list of categories found on Remote Publisher</returns>
    public static string CategoryRequestMessage(string ToAddr, string FromAddr) {
      StringBuilder sb = new StringBuilder();
      sb.Append("Type:GetCategories");
      sb.Append("\nToAddr:").Append(ToAddr);
      sb.Append("\nFromAddr:").Append(FromAddr);
      string msgBody = "Category List Request Message Body";
      sb.Append("\ncontent-length:").Append(msgBody.Length);
      sb.Append("\n\n").Append(msgBody);
      return sb.ToString();
    }

    /// <summary>
    /// Creates file list request message
    /// </summary>
    /// <param name="ToAddr">Address and port of the receiver</param>
    /// <param name="FromAddr">Address and port of the sender</param>
    /// <param name="Category">Category of the requested file list</param>
    /// <returns>Http Message string that requests file list from Remote Publisher</returns>
    public static string FileListRequestMessage(string ToAddr, string FromAddr, string Category) {
      StringBuilder sb = new StringBuilder();
      sb.Append("Type:GetFileList");
      sb.Append("\nToAddr:").Append(ToAddr);
      sb.Append("\nFromAddr:").Append(FromAddr);
      sb.Append("\nCategory:").Append(Category);
      string msgBody = "File List Message Body";
      sb.Append("\ncontent-length:").Append(msgBody.Length);
      sb.Append("\n\n").Append(msgBody);
      return sb.ToString();
    }

    /// <summary>
    /// Creates all files list request message
    /// </summary>
    /// <param name="ToAddr">Address and port of the receiver</param>
    /// <param name="FromAddr">Address and port of the sender</param>
    /// <returns>Http Message string that requests all files list from Remote Publisher</returns>
    public static string AllFilesRequestMessage(string ToAddr, string FromAddr) {
      StringBuilder sb = new StringBuilder();
      sb.Append("Type:GetAllFiles");
      sb.Append("\nToAddr:").Append(ToAddr);
      sb.Append("\nFromAddr:").Append(FromAddr);
      string msgBody = "All Files List Request Message Body";
      sb.Append("\ncontent-length:").Append(msgBody.Length);
      sb.Append("\n\n").Append(msgBody);
      return sb.ToString();
    }

    /// <summary>
    /// Creates file download request message
    /// </summary>
    /// <param name="ToAddr">Address and port of the receiver</param>
    /// <param name="FromAddr">Address and port of the sender</param>
    /// <param name="Filename">Filename to download from RemotePublisher</param>
    /// <returns>Http Message string that requests file download from Remote Publisher</returns>
    public static string GetFileRequestMessage(string ToAddr, string FromAddr, string Filename) {
      StringBuilder sb = new StringBuilder();
      sb.Append("Type:GetFile");
      sb.Append("\nToAddr:").Append(ToAddr);
      sb.Append("\nFromAddr:").Append(FromAddr);
      sb.Append("\nFilename:").Append(Filename);
      string msgBody = "File Request Message Body";
      sb.Append("\ncontent-length:").Append(msgBody.Length);
      sb.Append("\n\n").Append(msgBody);
      return sb.ToString();
    }

    /// <summary>
    /// Creates file upload message. This message tells Remote Publisher to download the files from its Socket.
    /// </summary>
    /// <param name="ToAddr">Address and port of the receiver</param>
    /// <param name="FromAddr">Address and port of the sender</param>
    /// <param name="Filename">Filename to upload to the Remote Publisher</param>
    /// <returns>Http Message string that tells Remote Publisher to download file</returns>
    public static string SendFileMessage(string ToAddr, string FromAddr, string Filename) {
      StringBuilder sb = new StringBuilder();
      sb.Append("Type:File");
      sb.Append("\nToAddr:").Append(ToAddr);
      sb.Append("\nFromAddr:").Append(FromAddr);
      sb.Append("\nFilename:").Append(Filename);
      return sb.ToString();
    }

    /// <summary>
    /// Creates delete file request message.
    /// </summary>
    /// <param name="ToAddr">Address and port of the receiver</param>
    /// <param name="FromAddr">Address and port of the sender</param>
    /// <param name="Filename">Filename to delete from Remote Publisher</param>
    /// <returns>Http Message string that requests file deletion from Remote Publisher</returns>
    public static string DeleteFileRequestMessage(string ToAddr, string FromAddr, string Filename) {
      StringBuilder sb = new StringBuilder();
      sb.Append("Type:DeleteFile");
      sb.Append("\nToAddr:").Append(ToAddr);
      sb.Append("\nFromAddr:").Append(FromAddr);
      sb.Append("\nFilename:").Append(Filename);
      string msgBody = "File Message Body";
      sb.Append("\ncontent-length:").Append(msgBody.Length);
      sb.Append("\n\n").Append(msgBody);
      return sb.ToString();
    }

    public static string GetDirectoriesMessage(string ToAddr, string FromAddr) {
      StringBuilder sb = new StringBuilder();
      sb.Append("Type:GetDirectories");
      sb.Append("\nToAddr:").Append(ToAddr);
      sb.Append("\nFromAddr:").Append(FromAddr);
      string msgBody = "Get Directories Message Body";
      sb.Append("\ncontent-length:").Append(msgBody.Length);
      sb.Append("\n\n").Append(msgBody);
      return sb.ToString();
    }

    public static string GetPublishedMessage(string ToAddr, string FromAddr) {
      StringBuilder sb = new StringBuilder();
      sb.Append("Type:GetPublishedWebpages");
      sb.Append("\nToAddr:").Append(ToAddr);
      sb.Append("\nFromAddr:").Append(FromAddr);
      string msgBody = "Get Published Message Body";
      sb.Append("\ncontent-length:").Append(msgBody.Length);
      sb.Append("\n\n").Append(msgBody);
      return sb.ToString();
    }
  }
}
