@echo off
chcp 65001 >nul 2>&1
cd /d "%~dp0\.."

set "VCVARS="

if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" (
    for /f "usebackq tokens=*" %%i in (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -find VC\Auxiliary\Build\vcvars64.bat`) do (
        set "VCVARS=%%i"
    )
)
if "%VCVARS%"=="" if exist "%ProgramFiles%\Microsoft Visual Studio\2026\Community\VC\Auxiliary\Build\vcvars64.bat" set "VCVARS=%ProgramFiles%\Microsoft Visual Studio\2026\Community\VC\Auxiliary\Build\vcvars64.bat"
if "%VCVARS%"=="" if exist "%ProgramFiles%\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" set "VCVARS=%ProgramFiles%\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
if "%VCVARS%"=="" if exist "%ProgramFiles%\Microsoft Visual Studio\2026\Professional\VC\Auxiliary\Build\vcvars64.bat" set "VCVARS=%ProgramFiles%\Microsoft Visual Studio\2026\Professional\VC\Auxiliary\Build\vcvars64.bat"
if "%VCVARS%"=="" if exist "%ProgramFiles%\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars64.bat" set "VCVARS=%ProgramFiles%\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars64.bat"
if "%VCVARS%"=="" if exist "%ProgramFiles%\Microsoft Visual Studio\2026\Enterprise\VC\Auxiliary\Build\vcvars64.bat" set "VCVARS=%ProgramFiles%\Microsoft Visual Studio\2026\Enterprise\VC\Auxiliary\Build\vcvars64.bat"
if "%VCVARS%"=="" if exist "%ProgramFiles%\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars64.bat" set "VCVARS=%ProgramFiles%\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars64.bat"
if "%VCVARS%"=="" if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\2026\BuildTools\VC\Auxiliary\Build\vcvars64.bat" set "VCVARS=%ProgramFiles(x86)%\Microsoft Visual Studio\2026\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
if "%VCVARS%"=="" if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat" set "VCVARS=%ProgramFiles(x86)%\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"

if "%VCVARS%"=="" (
    echo [TEST FAILED] Visual Studio C++ toolchain not found.
    exit /b 1
)

call "%VCVARS%" >nul 2>&1
if errorlevel 1 (
    echo [TEST FAILED] vcvars64.bat failed.
    exit /b 1
)

if not exist tests\bin mkdir tests\bin

cl /nologo /EHsc /W3 /utf-8 /D_WIN32_WINNT=0x0A00 /DNTDDI_VERSION=0x0A00000B /DWIN32_LEAN_AND_MEAN ^
    tests\window_manager_sort_tests.cpp src\window_manager.cpp ^
    /Fe:tests\bin\window_manager_sort_tests.exe ^
    /link user32.lib
if errorlevel 1 exit /b 1

tests\bin\window_manager_sort_tests.exe
if errorlevel 1 exit /b 1

echo [TEST PASSED] window_manager_sort_tests
