@echo off
setlocal

set "SCRIPT_DIR=%~dp0"
powershell -NoProfile -ExecutionPolicy Bypass -File "%SCRIPT_DIR%clang-format-windows.ps1" %*
exit /b %ERRORLEVEL%
