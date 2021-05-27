@echo off

set compilerFlags=-EHa- -Zi -Od -FC -WX -W4 -wd4505 -wd4456 -wd4201 -wd4100 -wd4189 User32.lib Gdi32.lib Winmm.lib

IF NOT EXIST ..\..\build mkdir ..\..\build
pushd ..\..\build

cl -nologo ..\rpg\code\win32_rpg.cpp %compilerFlags% 

popd
