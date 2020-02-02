"`n`nStarting Client without any arguments"
.\WFM_Client.exe 

"`n`nStarting Client taking test directory and *.h files searching for 'ammar'" 
.\WFM_Client.exe --path .\test --search ammar --patterns *.h

"`n`nStarting Client taking test directory and *.cpp files without including sub-directories searching for 'ammar'"
.\WFM_Client.exe --path .\test --search ammar --patterns *.cpp

"`n`nStarting Client taking test directory and *.cpp files with including sub-directories searching for 'ammar'"
.\WFM_Client.exe --path .\test --search ammar --patterns *.cpp -subs

"`n`nStarting Client taking test directory searching all files for 'nothing'"
.\WFM_Client.exe --path .\test --search nothing --patterns *.* -subs