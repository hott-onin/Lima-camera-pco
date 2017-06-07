@echo off
set PCODIR="C:\ProgramData\pco"
set BATDIR="c:\blissadm\pco\bat"
set LOGSONDIR="%BATDIR%\pcoLogsON"
set APPLICATION="%BATDIR%\getDate.py"
set DATEFILE="%PCODIR%\enviroment.txt"

c:
cd %PCODIR%

python %APPLICATION% > %DATEFILE%
set /P CurrentDate=<%DATEFILE%

md %CurrentDate%


move *.log %CurrentDate%
move *.txt %CurrentDate%
copy *.ini %CurrentDate%

rem =========================================

set >> %DATEFILE%
copy %LOGSONDIR%\*.ini .
copy %LOGSONDIR%\*.log .

