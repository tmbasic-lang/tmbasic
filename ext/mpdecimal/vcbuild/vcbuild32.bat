@ECHO off

"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere" -latest -property installationPath > vcpath.txt
set /p vcpath=<vcpath.txt
del vcpath.txt
call "%vcpath%\VC\Auxiliary\Build\vcvarsall.bat" x86

if not exist dist32 mkdir dist32
if exist dist32\* del /q dist32\*

cd ..\libmpdec
copy /y Makefile.vc Makefile
nmake clean
nmake MACHINE=ppro

copy /y libmpdec-2.5.0.lib ..\vcbuild\dist32
copy /y libmpdec-2.5.0.dll ..\vcbuild\dist32
copy /y libmpdec-2.5.0.dll.lib ..\vcbuild\dist32
copy /y libmpdec-2.5.0.dll.exp ..\vcbuild\dist32
copy /y mpdecimal.h ..\vcbuild\dist32


cd ..\libmpdec++
copy /y Makefile.vc Makefile
nmake clean
nmake

copy /y "libmpdec++-2.5.0.lib" ..\vcbuild\dist32


cd ..\vcbuild



