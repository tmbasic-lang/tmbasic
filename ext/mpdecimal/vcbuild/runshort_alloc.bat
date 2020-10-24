@ECHO OFF

rem Build libmpdec tests.
cd ..\tests
copy /y Makefile.vc Makefile
nmake clean
nmake

rem # Download the official test cases (text files).
call gettests.bat

echo.
echo # ========================================================================
echo #                         libmpdec: static library
echo # ========================================================================
echo.

echo.
<nul (set /p x="Running official tests with allocation failures ... ")
echo.
echo.
runtest_alloc.exe official.decTest
if %errorlevel% neq 0 goto out

echo.
<nul (set /p x="Running additional tests with allocation failures ... ")
echo.
echo.
runtest_alloc.exe additional.decTest
if %errorlevel% neq 0 goto out

echo.
echo # ========================================================================
echo #                         libmpdec: shared library
echo # ========================================================================
echo.

echo.
<nul (set /p x="Running official tests with allocation failures ... ")
echo.
echo.
runtest_alloc_shared.exe official.decTest
if %errorlevel% neq 0 goto out

echo.
<nul (set /p x="Running additional tests with allocation failures ... ")
echo.
echo.
runtest_alloc_shared.exe additional.decTest
if %errorlevel% neq 0 goto out


rem Build libmpdec++ tests.
cd ..\tests++
copy /y Makefile.vc Makefile
nmake clean
nmake

rem # Copy or download the official test cases (text files).
call gettests.bat

echo.
echo # ========================================================================
echo #                        libmpdec++: static library
echo # ========================================================================
echo.

echo.
<nul (set /p x="Running official tests with allocation failures ... ")
echo.
echo.
runtest_alloc.exe official.topTest --threaded
if %errorlevel% neq 0 goto out

echo.
<nul (set /p x="Running additional tests with allocation failures ... ")
echo.
echo.
runtest_alloc.exe additional.topTest --threaded
if %errorlevel% neq 0 goto out

echo.
<nul (set /p x="Running API tests (single thread) ... ")
echo.
echo.
apitest.exe
if %errorlevel% neq 0 goto out

echo.
<nul (set /p x="Running API tests (threaded) ... ")
echo.
echo.
apitest.exe --threaded
if %errorlevel% neq 0 goto out


:out
set exitcode=%errorlevel%
cd ..\vcbuild
exit /b %exitcode%
