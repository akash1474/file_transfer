@echo off
if /i [%1]==[]  (
	premake5 vs2022
	MsBuild FileTransfer.sln /p:configuration=Dist
)
"bin/file_transfer.exe"

