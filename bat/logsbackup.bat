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

copy *.log %CurrentDate%
copy *.txt %CurrentDate%
copy *.ini %CurrentDate%
