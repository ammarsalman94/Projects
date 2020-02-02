@setlocal enableextensions
@cd /d "%~dp0"

START "Repository" .\Repository.exe
START "Remote Test Harness" ".\Remote TestHarness.exe"
START "Client" .\Client.exe

EXIT