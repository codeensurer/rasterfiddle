REM @echo off
set platform=ux64

Set EFAL_DLLS_DIR=..\..\..
if "%1-%2" == "x64-Debug" set platform=ux64
if "%1-%2" == "x64-Release" set platform=ux64
if "%1-%2" == "Win32-Debug" set platform=uw32
if "%1-%2" == "Win32-Release" set platform=uw32

if NOT EXIST %EFAL_DLLS_DIR%\export\NUL goto insitu
goto xport

:insitu
Set EFAL_DLLS_DIR=..\..\..\..\..
if "%1-%2" == "x64-Debug" set platform=dux64
if "%1-%2" == "x64-Release" set platform=ux64
if "%1-%2" == "Win32-Debug" set platform=duw32
if "%1-%2" == "Win32-Release" set platform=uw32


:xport
if "%1" == "x64" set DotNetArch=x64
if "%1" == "Win32" set DotNetArch=x86
echo %1-%2
echo %platform%

echo copy /Y sample_commands.txt  %EFAL_DLLS_DIR%\export\%platform%
echo copy /Y runJFAL.bat  %EFAL_DLLS_DIR%\export\%platform%
echo copy /Y ..\EFALShell\%1\%2\EFALShell.exe  %EFAL_DLLS_DIR%\export\%platform%
echo copy /Y ..\NFAL\%1\%2\NFAL.dll  %EFAL_DLLS_DIR%\export\%platform%
echo copy /Y ..\NFALShell\bin\%DotNetArch%\%2\NFALShell.exe  %EFAL_DLLS_DIR%\export\%platform%
echo copy /Y ..\JFAL\%1\%2\*.DLL %EFAL_DLLS_DIR%\export\%platform%
echo copy /Y ..\JFAL\%1\%2\*.JAR %EFAL_DLLS_DIR%\export\%platform%

echo copy /Y ..\Text2TAB\%1\%2\Text2TAB.exe  %EFAL_DLLS_DIR%\export\%platform%
echo copy /Y ..\WKT2WKB\%1\%2\WKT2WKB.exe  %EFAL_DLLS_DIR%\export\%platform%

copy /Y sample_commands.txt  %EFAL_DLLS_DIR%\export\%platform%
copy /Y runJFAL.bat  %EFAL_DLLS_DIR%\export\%platform%
copy /Y ..\Text2TAB\%1\%2\Text2TAB.exe  %EFAL_DLLS_DIR%\export\%platform%
copy /Y ..\WKT2WKB\%1\%2\WKT2WKB.exe  %EFAL_DLLS_DIR%\export\%platform%
copy /Y ..\EFALShell\%1\%2\EFALShell.exe  %EFAL_DLLS_DIR%\export\%platform%
copy /Y ..\NFAL\%1\%2\NFAL.dll  %EFAL_DLLS_DIR%\export\%platform%
copy /Y ..\NFALShell\bin\%DotNetArch%\%2\NFALShell.exe  %EFAL_DLLS_DIR%\export\%platform%
copy /Y ..\JFAL\%1\%2\*.DLL %EFAL_DLLS_DIR%\export\%platform%
copy /Y ..\JFAL\%1\%2\*.JAR %EFAL_DLLS_DIR%\export\%platform%

