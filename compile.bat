@echo off
REM ---------------------------------------------------------------------------
REM  Jebiga-Gaming - Gamemode und Filterscripts uebersetzen (Windows)
REM
REM  Erwartet pawncc.exe in .toolchain\ - siehe setup.ps1.
REM ---------------------------------------------------------------------------

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

REM  -d3   volle Debug-Informationen (Zeilennummern in Laufzeitfehlern)
REM  -;+   Semikolon am Anweisungsende verpflichtend
REM  -(+   Klammern um Kontrollstrukturen verpflichtend
REM
REM  Semikolon und Klammer sind fuer die Eingabeaufforderung Trennzeichen -
REM  ohne Anfuehrungszeichen wuerde sie die Optionen zerlegen.
set "OPT1=-d3"
set "OPT2=-;+"
set "OPT3=-(+"

set "INC1=-i%ROOT%"
set "INC2=-i%ROOT%\pawno\include"
set "INC3=-i%ROOT%\.toolchain\include"

echo == Gamemode ==
"%PAWNCC%" "%ROOT%\gamemodes\jebiga.pwn" "%INC1%" "%INC2%" "%INC3%" ^
    "-o%ROOT%\gamemodes\jebiga.amx" "%OPT1%" "%OPT2%" "%OPT3%"
if errorlevel 1 goto :failed

echo.
echo == Filterscripts ==
for %%F in ("%ROOT%\filterscripts\*.pwn") do (
    echo -- %%~nF
    "%PAWNCC%" "%%F" "%INC1%" "%INC2%" "%INC3%" ^
        "-o%ROOT%\filterscripts\%%~nF.amx" "%OPT1%" "%OPT2%" "%OPT3%"
    if errorlevel 1 goto :failed
)

echo.
echo Fertig.
endlocal
exit /b 0

:failed
echo.
echo Uebersetzung fehlgeschlagen.
endlocal
exit /b 1
