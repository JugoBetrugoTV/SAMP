@echo off
REM Jebiga-Gaming - Server starten (Windows)

setlocal
set "ROOT=%~dp0"
cd /d "%ROOT%"

if not exist "gamemodes\jebiga.amx" (
    echo gamemodes\jebiga.amx fehlt - bitte zuerst compile.bat ausfuehren.
    exit /b 1
)

set "SERVER="
if exist "samp-server.exe" set "SERVER=samp-server.exe"
if exist "omp-server.exe"  set "SERVER=omp-server.exe"

if "%SERVER%"=="" (
    echo Es wurde keine Serveranwendung gefunden.
    echo Erwartet wird samp-server.exe oder omp-server.exe im Projektverzeichnis.
    echo Siehe README, Abschnitt "Serverpaket besorgen".
    exit /b 1
)

findstr /C:"CHANGE_ME_BEFORE_FIRST_START" server.cfg >nul 2>&1
if not errorlevel 1 (
    echo Warnung: das rcon_password in server.cfg ist noch der Platzhalter.
)

if not exist "scriptfiles\accounts" mkdir "scriptfiles\accounts"
if not exist "scriptfiles\logs"     mkdir "scriptfiles\logs"

echo Starte %SERVER% ...
"%SERVER%"
