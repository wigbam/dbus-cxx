@echo off
setlocal enableextensions enabledelayedexpansion

REM delay is specified in centiseconds
set wait_time=%1
set _tail=%*
call set _tail=%%_tail:*%1=%%

call :wait %wait_time%

%_tail% || exit /b 1
goto :eof

:wait
call :gettime wait0
:w2
call :gettime wait1
set /A waitt = wait1-wait0
if !waitt! lss %1 goto :w2
goto :eof

:gettime
set hh=%time:~0,2%
set mm=%time:~3,2%
set ss=%time:~6,2%
set cc=%time:~-2%
set /A %1=hh*360000+mm*6000+ss*100+cc
goto :eof
