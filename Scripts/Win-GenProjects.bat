@echo off
pushd %~dp0\..\
call vendor\bin\Windows\premake\premake5.exe --os=windows vs2019
popd
PAUSE