@echo off
setlocal
cd /d "%~dp0"

set "SDK8_BIN=C:\Program Files (x86)\Windows Kits\8.0\bin\x86"
set "FXC=%SDK8_BIN%\fxc.exe"

if not exist "%FXC%" (
    echo ERROR: fxc.exe not found:
    echo %FXC%
    exit /b 1
)

"%FXC%" /nologo /T vs_4_0 /E VSMain /Fo vertex.cso shader.hlsl
if errorlevel 1 exit /b 1

"%FXC%" /nologo /T ps_4_0 /E PSMain /Fo pixel.cso shader.hlsl
if errorlevel 1 exit /b 1

echo.
echo Shaders compiled successfully.
