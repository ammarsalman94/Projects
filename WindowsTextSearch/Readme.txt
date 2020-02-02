---------------------------------------------------------------------------------------
| Project : Text Search Component - Windows Version                                   |
| Course  : Distributed Objects - CSE-775, Professor Jim Fawcett, Spring 2017         |
| Author  : Ammar Salman, Syracuse University, (313) 788-4694, hoplite.90@hotmail.com |
---------------------------------------------------------------------------------------

In this project, I have created two shared libraries:
1. TextSearchCom.dll
2. FileMgr.dll

The libraries should be build first in Visual Studio 2015 (running as admin).
Please note that MIDI compiler will not generate everything correct. There is a file
that will need to be modified for everything to work. 


There is a client that uses these libraries in the following order:
1. Client creates TextSearch instance
2. Client creates FileManager instance
3. Client passes TextSearch instance to FileManager instance
4. Client configures FileManager directory, search string, and including sub-dirs option
5. Client orders FileManager to send its files to TextSearch
6. Client gets results directly from TextSearch instance

The client takes command line arguments. To use the client in an automated way start run.ps1 script. 
It shows the usage when there are no command line args specified as well as when they are specified.