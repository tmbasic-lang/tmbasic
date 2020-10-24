@ECHO off

"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere" -latest -property installationPath > vcpath.txt
set /p vcpath=<vcpath.txt
del vcpath.txt
call "%vcpath%\VC\Auxiliary\Build\vcvarsall.bat" x64

if not exist dist64 mkdir dist64
if exist dist64\* del /q dist64\*

cd ..\libmpdec
copy /y Makefile.vc Makefile
nmake clean
nmake MACHINE=x64

copy /y libmpdec-2.5.0.lib ..\vcbuild\dist64
copy /y libmpdec-2.5.0.dll ..\vcbuild\dist64
copy /y libmpdec-2.5.0.dll.lib ..\vcbuild\dist64
copy /y libmpdec-2.5.0.dll.exp ..\vcbuild\dist64
copy /y mpdecimal.h ..\vcbuild\dist64


cd ..\libmpdec++
copy /y Makefile.vc Makefile
nmake clean
nmake

copy /y "libmpdec++-2.5.0.lib" ..\vcbuild\dist64

cd ..\vcbuild



