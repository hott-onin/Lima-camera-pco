@echo off
c:
cd C:\ProgramData\pco

@echo off & for /F "tokens=1-4 delims=/ " %%A in ('date/t') do (
set DateDay=%%A
set DateMonth=%%B
set DateYear=%%C
)

@echo off & for /F "tokens=1-4 delims=: " %%D in ('time/t') do (
set DateHour=%%D
set DateMin=%%E
)

set CurrentDate=%DateYear%-%DateMonth%-%DateDay%-%DateHour%%DateMin%

echo %CurrentDate%

md %CurrentDate%

move *.log %CurrentDate%
move *.txt %CurrentDate%
copy *.ini %CurrentDate%

rem =========================================

echo %CurrentDate% > PCO_CDlg.txt
echo %CurrentDate% > PCO_Conv.txt
echo %CurrentDate% > SC2_Cam.txt
