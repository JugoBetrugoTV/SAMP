<#
    Jebiga-Gaming - Einrichtung unter Windows

    Holt den Pawn-Compiler und die SA-MP-Includes, richtet die
    Laufzeitverzeichnisse ein und stellt server.cfg auf Windows um.

    Aufruf in PowerShell im Projektverzeichnis:

        powershell -ExecutionPolicy Bypass -File .\setup.ps1

    Was dieses Skript NICHT tut: Plugins bauen. Unter Windows werden Plugins
    mit Visual Studio uebersetzt; der Aufwand lohnt nicht, weil es fuer alle
    hier verwendeten Plugins fertige .dll-Dateien gibt. Das Skript legt den
    Ordner plugins\ an und sagt, welche Dateien hineingehoeren.
#>

$ErrorActionPreference = 'Stop'

$Root       = Split-Path -Parent $MyInvocation.MyCommand.Path
$Toolchain  = Join-Path $Root '.toolchain'
$IncludeDir = Join-Path $Root 'pawno\include'
$BuildDir   = Join-Path $Root '.build'
$PluginDir  = Join-Path $Root 'plugins'

$SampStdlibRepo   = 'https://github.com/pawn-lang/samp-stdlib.git'
$SampStdlibCommit = '8ffb055624308b25521665b60e78b5e6e6b3717f'
$CompilerRelease  = 'https://github.com/pawn-lang/compiler/releases/download/v3.10.10/pawnc-3.10.10-windows.zip'
$MysqlRepo        = 'https://github.com/pBlueG/SA-MP-MySQL.git'
$MysqlVersion     = 'R41-4'

function Write-Step($Text) { Write-Host "==> $Text" -ForegroundColor Cyan }
function Write-Fail($Text) { Write-Host "Fehler: $Text" -ForegroundColor Red; exit 1 }

if (-not (Get-Command git -ErrorAction SilentlyContinue)) {
    Write-Fail 'git wird benoetigt, ist aber nicht installiert.'
}

New-Item -ItemType Directory -Force -Path $Toolchain, $IncludeDir, $BuildDir, $PluginDir | Out-Null
New-Item -ItemType Directory -Force -Path (Join-Path $Root 'scriptfiles\accounts'),
                                         (Join-Path $Root 'scriptfiles\logs') | Out-Null

# --- SA-MP-Includes ---------------------------------------------------------
if (Test-Path (Join-Path $IncludeDir 'a_samp.inc')) {
    Write-Step 'SA-MP-Includes bereits vorhanden - uebersprungen.'
} else {
    Write-Step "Hole SA-MP-Includes ($($SampStdlibCommit.Substring(0,8)))..."
    $stdlib = Join-Path $BuildDir 'samp-stdlib'
    if (Test-Path $stdlib) { Remove-Item -Recurse -Force $stdlib }

    git clone $SampStdlibRepo $stdlib
    git -C $stdlib checkout --quiet $SampStdlibCommit
    Copy-Item (Join-Path $stdlib '*.inc') $IncludeDir
    Write-Step "$((Get-ChildItem $IncludeDir -Filter *.inc).Count) Includes installiert."
}

# --- Pawn-Compiler ----------------------------------------------------------
if (Test-Path (Join-Path $Toolchain 'pawncc.exe')) {
    Write-Step 'Pawn-Compiler bereits vorhanden - uebersprungen.'
} else {
    Write-Step 'Lade Pawn-Compiler fuer Windows...'
    $zip = Join-Path $BuildDir 'pawnc-windows.zip'

    try {
        Invoke-WebRequest -Uri $CompilerRelease -OutFile $zip -UseBasicParsing
    } catch {
        Write-Fail @"
Der Compiler konnte nicht geladen werden ($($_.Exception.Message)).
Bitte pawnc-3.10.10-windows.zip von Hand herunterladen von
  https://github.com/pawn-lang/compiler/releases
und pawncc.exe samt pawnc.dll nach .toolchain\ entpacken.
"@
    }

    $extract = Join-Path $BuildDir 'pawnc'
    if (Test-Path $extract) { Remove-Item -Recurse -Force $extract }
    Expand-Archive -Path $zip -DestinationPath $extract -Force

    Get-ChildItem $extract -Recurse -Include 'pawncc.exe','pawnc.dll' |
        ForEach-Object { Copy-Item $_.FullName $Toolchain -Force }

    $compilerIncludes = Get-ChildItem $extract -Recurse -Directory -Filter 'include' | Select-Object -First 1
    if ($compilerIncludes) {
        New-Item -ItemType Directory -Force -Path (Join-Path $Toolchain 'include') | Out-Null
        Copy-Item (Join-Path $compilerIncludes.FullName '*.inc') (Join-Path $Toolchain 'include') -Force
    }
    Write-Step 'Compiler installiert.'
}

# --- MySQL-Include ----------------------------------------------------------
if (Test-Path (Join-Path $IncludeDir 'a_mysql.inc')) {
    Write-Step 'a_mysql.inc bereits vorhanden - uebersprungen.'
} else {
    Write-Step "Erzeuge a_mysql.inc ($MysqlVersion)..."
    $mysql = Join-Path $BuildDir 'mysql'
    if (Test-Path $mysql) { Remove-Item -Recurse -Force $mysql }

    git clone --depth 1 $MysqlRepo $mysql
    (Get-Content (Join-Path $mysql 'a_mysql.inc.in') -Raw) `
        -replace '@MYSQL_PLUGIN_VERSION@', $MysqlVersion |
        Set-Content (Join-Path $IncludeDir 'a_mysql.inc') -Encoding UTF8
}

# --- server.cfg auf Windows umstellen ---------------------------------------
# Zeilenweise, nicht ueber einen Regex-Scriptblock: den gibt es erst ab
# PowerShell 6, auf Windows ist 5.1 die Voreinstellung.
$cfgPath = Join-Path $Root 'server.cfg'
if (Test-Path $cfgPath) {
    $lines   = Get-Content $cfgPath
    $changed = $false

    for ($i = 0; $i -lt $lines.Count; $i++) {
        if ($lines[$i] -like 'plugins *' -and $lines[$i] -like '*.so*') {
            $lines[$i] = $lines[$i].Replace('.so', '.dll')
            $changed = $true
        }
    }

    if ($changed) {
        Write-Step 'Stelle die Plugin-Endungen in server.cfg auf .dll um...'
        Set-Content $cfgPath $lines -Encoding UTF8
    }
}

Write-Host ''
Write-Host 'Einrichtung abgeschlossen.' -ForegroundColor Green
Write-Host @'
  .\compile.bat       Gamemode und Filterscripts uebersetzen
  .\samp-server.exe   Server starten

Noch zu erledigen:

  1. Serverpaket besorgen (samp-server.exe bzw. omp-server.exe) - siehe README.

  2. Plugins nach plugins\ legen. Benoetigt werden:
       streamer.dll      Pflicht, traegt die Custom Map
       crashdetect.dll   dringend empfohlen beim Einfahren
       sscanf.dll        optional
       mysql.dll         nur bei Enabled=1 in scriptfiles\mysql.ini
     Alle vier gibt es als fertige Windows-Builds bei den jeweiligen Projekten.

  3. rcon_password in server.cfg aendern.
'@
