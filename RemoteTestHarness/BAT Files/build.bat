@setlocal enableextensions
@cd /d "%~dp0"

call "%programfiles(x86)%\MSBuild\14.0\Bin\MSBuild.exe" ..\RemoteTestHarness.sln /p:Configuration=Debug /p:Platform="Any CPU"

COPY ..\BlockingQueue\bin\Debug\BlockingQueue.dll .
COPY ..\XMLReader\bin\Debug\XMLReader.dll .

COPY ..\CommInterfaces\bin\Debug\CommInterfaces.dll .
COPY ..\Receiver\bin\Debug\Receiver.dll .
COPY ..\Sender\bin\Debug\Sender.dll .

COPY ..\Repository\bin\Debug\Repository.exe .

COPY ..\TestAppDomain\bin\Debug\TestAppDomain.dll .
COPY "..\RemoteTestHarness\bin\Debug\Remote TestHarness.exe" .

COPY ..\Client\bin\Debug\Client.exe .

COPY "..\A_TestExecutive\bin\Debug\Test Executive.exe" .

PAUSE