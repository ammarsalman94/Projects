DEL Output.txt
DEL db_autosave.xml
DEL db2_autosave.xml
DEL simpleDB.xml

CALL "%programfiles(x86)%\Microsoft Visual Studio 14.0\Common7\IDE\devenv.exe" "NoSQLDB.sln" /Clean debug

CALL "%programfiles(x86)%\Microsoft Visual Studio 14.0\Common7\IDE\devenv.exe" "NoSQLDB.sln" /Build debug

PAUSE