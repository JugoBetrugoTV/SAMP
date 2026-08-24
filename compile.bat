@echo off
REM Jebiga-Gaming - Gamemode und Filterscripts uebersetzen (Windows)
REM
REM Erwartet pawncc.exe in .toolchain\ - siehe setup.ps1.

setlocal enabledelayedexpansion
set "ROOT=%~dp0"
set "ROOT=%ROOT:~0,-1%"
set "PAWNCC=%ROOT%\.toolchain\pawncc.exe"

if not exist "%PAWNCC%" (
    echo Der Pawn-Compiler fehlt. Bitte zuerst setup.ps1 ausfuehren.
    exit /b 1
)
if not exist "%ROOT%\pawno\include\a_samp.inc" (
    echo Die SA-MP-Includes fehlen. Bitte zuerst setup.ps1 ausfuehren.
    exit /b 1
)

set "OPTS=-d3 -;+ -(+"
set "INCLUDES=-i"%ROOT%" -i"%ROOT%\pawno\include" -i"%ROOT%\.toolchain\include""

echo == Gamemode ==
"%PAWNCC%" "%ROOT%\gamemodes\jebiga.pwn" %INCLUDES% -o"%ROOT%\gamemodes\jebiga.amx" %OPTS%
if errorlevel 1 goto :failed

echo.
echo == Filterscripts ==
for %%F in ("%ROOT%\filterscripts\*.pwn") do (
    echo -- %%~nF
    "%PAWNCC%" "%%F" %INCLUDES% -o"%ROOT%\filterscripts\%%~nF.amx" %OPTS%
    if errorlevel 1 goto :failed
)

echo.
echo Fertig.
exit /b 0

:failed
echo.
echo Uebersetzung fehlgeschlagen.
exit /b 1
