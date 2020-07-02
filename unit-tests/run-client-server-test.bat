@echo off
setlocal enableextensions enabledelayedexpansion

REM Start server
REM Would have been preferable to use 'wmic PROCESS call create' to get the exact PID but wmic does not propagate
REM environment variables which results in DBUS_SESSION_BUS_ADDRESS exported by 'dbus-run-session' being lost
for /F %%i in ("%1") do @set TEST_EXEC=%%~nxi
start /high /b "" %1 server %2

REM Get server PID
set /A PID_COUNT=0
for /F "TOKENS=2 delims=," %%a in ('tasklist /v /fo csv /NH /FI "IMAGENAME eq %TEST_EXEC%"') do (
  set PID=%%~a
  set /A PID_COUNT=PID_COUNT+1
  REM Ensure there was only a single instance of server process found so as not to kill something else by accident
  if !PID_COUNT! GTR 1 goto :server_multiple_instances_error
)
SET "PID_N="&for /f "delims=0123456789" %%i in ("%PID%") do set var=%%i
if defined PID_N goto :server_startup_error

REM Run client
%1 client %2 && set EXIT_CODE=0 || set EXIT_CODE=1

taskkill /f /PID %PID% 2>nul >nul
tasklist /fi "pid eq %PID%" 2>nul | find "%PID%" >nul
if NOT %ERRORLEVEL%==0 goto :end
goto :server_shutdown_error

:end
exit /b %EXIT_CODE%

:server_startup_error
echo Failed - unable to launch the server
exit /b 1

:server_multiple_instances_error
echo Failed - multiple server instances detected
exit /b 1

:server_shutdown_error
echo Failed - unable to verify the server has stopped
exit /b 1
