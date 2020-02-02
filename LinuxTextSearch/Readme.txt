---------------------------------------------------------------------------------------
| Project : Text Search Component - Linux Version                                     |
| Course  : Distributed Objects - CSE-775, Professor Jim Fawcett, Spring 2017         |
| Author  : Ammar Salman, Syracuse University, (313) 788-4694, hoplite.90@hotmail.com |
---------------------------------------------------------------------------------------

In this project, I have created two shared libraries:
1. libTextSearch.so
2. libFileManager.so

The libraries should be moved to /usr/lib .. Run copylibs.sh script. If the libraries are not built, it will build them and move them. If they exist, it will only move them.

sh copylibs.sh

Note: it will ask for sudo password since it needs permission to copy the files to /usr/lib

There is a client that uses these libraries in the following order:
1. Client creates TextSearch instance
2. Client creates FileManager instance
3. Client passes TextSearch instance to FileManager instance
4. Client configures FileManager directory, search string, and including sub-dirs option
5. Client orders FileManager to send its files to TextSearch
6. Client gets results directly from TextSearch instance

The client takes command line arguments. To use the client in an automated way start run.sh script. It shows the usage when there are no command line args specified.
