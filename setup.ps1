<#
    Jebiga-Gaming - Einrichtung unter Windows

    Richtet alles ein, was zum Uebersetzen und Starten noetig ist:
      - Pawn-Compiler
      - SA-MP-Includes
      - Plugins (streamer, crashdetect, sscanf2) samt passender Includes
      - a_mysql.inc fuer die optionale Datenbankanbindung
      - Laufzeitverzeichnisse

    Aufruf in PowerShell im Projektverzeichnis:

        powershell -ExecutionPolicy Bypass -File .\setup.ps1

    Das Skript ist auf Windows PowerShell 5.1 ausgelegt - die Fassung, die
    Windows mitbringt. Es vermeidet deshalb bewusst Sprachmittel, die es erst
    ab PowerShell 6 gibt.

    Scheitert ein einzelner Download, bricht das Skript nicht ab: es merkt sich
    den Ausfall, macht weiter und sagt am Ende genau, was fehlt und woher es
    kommt. Ein halb eingerichteter Baum ohne Hinweis waere schlimmer.
#>

$ErrorActionPreference = 'Stop'
[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12

$Root       = Split-Path -Parent $MyInvocation.MyCommand.Path
$Toolchain  = Join-Path $Root '.toolchain'
$IncludeDir = Join-Path $Root 'pawno\include'
$BuildDir   = Join-Path $Root '.build'
$PluginDir  = Join-Path $Root 'plugins'

$SampStdlibRepo   = 'https://github.com/pawn-lang/samp-stdlib.git'
$SampStdlibCommit = '8ffb055624308b25521665b60e78b5e6e6b3717f'
$MysqlRepo        = 'https://github.com/pBlueG/SA-MP-MySQL.git'
$MysqlVersion     = 'R41-4'

$CompilerUrl  = 'https://github.com/pawn-lang/compiler/releases/download/v3.10.10/pawnc-3.10.10-windows.zip'
$CompilerPage = 'https://github.com/pawn-lang/compiler/releases'

<#
    Plugins als fertige Windows-Builds.

    Wichtig: aus jedem Archiv wird nicht nur die .dll uebernommen, sondern auch
    die mitgelieferte .inc. So passen Include und Binary immer zusammen - unter
    Linux baut setup.sh aus einem anderen Stand (der aeltere Release-Zweig
    laesst sich dort nicht mehr bauen), und ein gemischtes Paar waere die Art
    Fehler, die erst zur Laufzeit auffaellt.

    Die Adressen konnten in der Entwicklungsumgebung nicht abgerufen werden.
    Falls ein Download ins Leere laeuft, nennt das Skript die Release-Seite.
#>
$Plugins = @(
    @{ Name     = 'streamer'
       Required = $true
       Url      = 'https://github.com/samp-incognito/samp-streamer-plugin/releases/download/v2.9.6/samp-streamer-plugin-2.9.6.zip'
       Page     = 'https://github.com/samp-incognito/samp-streamer-plugin/releases'
       Dll      = 'streamer.dll'
       Note     = 'Pflicht - ohne den Streamer fehlt die Custom Map.' }

    @{ Name     = 'crashdetect'
       Required = $false
       Url      = 'https://github.com/Zeex/samp-plugin-crashdetect/releases/download/v4.22/crashdetect-4.22-win32.zip'
       Page     = 'https://github.com/Zeex/samp-plugin-crashdetect/releases'
       Dll      = 'crashdetect.dll'
       Note     = 'Zeigt bei Laufzeitfehlern Datei und Zeile statt einer Speicheradresse.' }

    @{ Name     = 'sscanf'
       Required = $false
       Url      = 'https://github.com/maddinat0r/sscanf/releases/download/v2.13.8/sscanf-2.13.8.zip'
       Page     = 'https://github.com/maddinat0r/sscanf/releases'
       Dll      = 'sscanf.dll'
       Note     = 'Das Gamemode braucht sscanf nicht; eigene Filterscripts profitieren davon.' }
)

$Missing = @()

function Write-Step($Text) { Write-Host "==> $Text" -ForegroundColor Cyan }
function Write-Warn($Text) { Write-Host "    $Text" -ForegroundColor Yellow }
function Write-Fail($Text) { Write-Host "Fehler: $Text" -ForegroundColor Red; exit 1 }

function Get-File($Url, $Target) {
    Invoke-WebRequest -Uri $Url -OutFile $Target -UseBasicParsing
}

# ---------------------------------------------------------------------------
# Voraussetzungen
# ---------------------------------------------------------------------------
if (-not (Get-Command git -ErrorAction SilentlyContinue)) {
    Write-Fail 'git wird benoetigt. Zu bekommen unter https://git-scm.com/download/win'
}

New-Item -ItemType Directory -Force -Path $Toolchain, $IncludeDir, $BuildDir, $PluginDir | Out-Null
New-Item -ItemType Directory -Force -Path (Join-Path $Root 'scriptfiles\accounts'),
                                         (Join-Path $Root 'scriptfiles\logs') | Out-Null

# ---------------------------------------------------------------------------
# SA-MP-Includes
# ---------------------------------------------------------------------------
if (Test-Path (Join-Path $IncludeDir 'a_samp.inc')) {
    Write-Step 'SA-MP-Includes bereits vorhanden - uebersprungen.'
} else {
    Write-Step "Hole SA-MP-Includes ($($SampStdlibCommit.Substring(0,8)))..."
    $stdlib = Join-Path $BuildDir 'samp-stdlib'
    if (Test-Path $stdlib) { Remove-Item -Recurse -Force $stdlib }

    git clone --quiet $SampStdlibRepo $stdlib
    git -C $stdlib checkout --quiet $SampStdlibCommit
    Copy-Item (Join-Path $stdlib '*.inc') $IncludeDir
    Write-Step "$((Get-ChildItem $IncludeDir -Filter *.inc).Count) Includes installiert."
}

# ---------------------------------------------------------------------------
# Pawn-Compiler
# ---------------------------------------------------------------------------
if (Test-Path (Join-Path $Toolchain 'pawncc.exe')) {
    Write-Step 'Pawn-Compiler bereits vorhanden - uebersprungen.'
} else {
    Write-Step 'Lade Pawn-Compiler...'
    $zip     = Join-Path $BuildDir 'pawnc-windows.zip'
    $extract = Join-Path $BuildDir 'pawnc'

    try {
        Get-File $CompilerUrl $zip
        if (Test-Path $extract) { Remove-Item -Recurse -Force $extract }
        Expand-Archive -Path $zip -DestinationPath $extract -Force

        Get-ChildItem $extract -Recurse -Include 'pawncc.exe','pawnc.dll' |
            ForEach-Object { Copy-Item $_.FullName $Toolchain -Force }

        $compilerIncludes = Get-ChildItem $extract -Recurse -Directory -Filter 'include' |
            Select-Object -First 1
        if ($compilerIncludes) {
            New-Item -ItemType Directory -Force -Path (Join-Path $Toolchain 'include') | Out-Null
            Copy-Item (Join-Path $compilerIncludes.FullName '*.inc') (Join-Path $Toolchain 'include') -Force
        }
        Write-Step 'Compiler installiert.'
    } catch {
        Write-Fail @"
Der Pawn-Compiler konnte nicht geladen werden.
  $($_.Exception.Message)

Bitte pawnc-3.10.10-windows.zip von Hand holen:
  $CompilerPage
und pawncc.exe samt pawnc.dll nach .toolchain\ entpacken,
die mitgelieferten .inc-Dateien nach .toolchain\include\.
"@
    }
}

# ---------------------------------------------------------------------------
# Plugins
# ---------------------------------------------------------------------------
foreach ($plugin in $Plugins) {
    $target = Join-Path $PluginDir $plugin.Dll

    if (Test-Path $target) {
        Write-Step "$($plugin.Name) bereits vorhanden - uebersprungen."
        continue
    }

    Write-Step "Lade $($plugin.Name)..."
    $zip     = Join-Path $BuildDir "$($plugin.Name).zip"
    $extract = Join-Path $BuildDir $plugin.Name

    try {
        Get-File $plugin.Url $zip
        if (Test-Path $extract) { Remove-Item -Recurse -Force $extract }
        Expand-Archive -Path $zip -DestinationPath $extract -Force

        $dll = Get-ChildItem $extract -Recurse -Filter $plugin.Dll | Select-Object -First 1
        if (-not $dll) { throw "$($plugin.Dll) war nicht im Archiv." }
        Copy-Item $dll.FullName $PluginDir -Force

        # Passende Includes aus demselben Archiv uebernehmen
        Get-ChildItem $extract -Recurse -Filter '*.inc' |
            ForEach-Object { Copy-Item $_.FullName $IncludeDir -Force }

        Write-Step "$($plugin.Name) installiert."
    } catch {
        Write-Warn "$($plugin.Name) konnte nicht geladen werden: $($_.Exception.Message)"
        Write-Warn "Release-Seite: $($plugin.Page)"
        $Missing += $plugin
    }
}

# ---------------------------------------------------------------------------
# MySQL-Include
# ---------------------------------------------------------------------------
if (Test-Path (Join-Path $IncludeDir 'a_mysql.inc')) {
    Write-Step 'a_mysql.inc bereits vorhanden - uebersprungen.'
} else {
    Write-Step "Erzeuge a_mysql.inc ($MysqlVersion)..."
    $mysql = Join-Path $BuildDir 'mysql'
    if (Test-Path $mysql) { Remove-Item -Recurse -Force $mysql }

    git clone --quiet --depth 1 $MysqlRepo $mysql
    (Get-Content (Join-Path $mysql 'a_mysql.inc.in') -Raw) `
        -replace '@MYSQL_PLUGIN_VERSION@', $MysqlVersion |
        Set-Content (Join-Path $IncludeDir 'a_mysql.inc') -Encoding UTF8
}

# ---------------------------------------------------------------------------
# Abschluss
# ---------------------------------------------------------------------------
Write-Host ''

if ($Missing.Count -gt 0) {
    Write-Host 'Nicht alles konnte geladen werden:' -ForegroundColor Yellow
    Write-Host ''

    foreach ($plugin in $Missing) {
        $label = if ($plugin.Required) { 'PFLICHT ' } else { 'optional' }
        Write-Host ("  [$label] $($plugin.Dll)")
        Write-Host ("             $($plugin.Note)")
        Write-Host ("             $($plugin.Page)")
        Write-Host ("             .dll nach plugins\ , mitgelieferte .inc nach pawno\include\")
        Write-Host ''
    }

    $optional = @($Missing | Where-Object { -not $_.Required })
    if ($optional.Count -gt 0) {
        $names = ($optional | ForEach-Object { $_.Dll }) -join ', '
        Write-Host "  Alternativ diese aus der plugins-Zeile in server.cfg streichen: $names" -ForegroundColor Yellow
        Write-Host '  Sonst verweigert der Server den Start.' -ForegroundColor Yellow
        Write-Host ''
    }
}

Write-Host 'Einrichtung abgeschlossen.' -ForegroundColor Green
Write-Host @'
  .\compile.bat       Gamemode und Filterscripts uebersetzen
  .\run.bat           Server starten

Noch zu erledigen:
  1. Serverpaket besorgen (samp-server.exe bzw. omp-server.exe) - siehe README.
  2. rcon_password in server.cfg aendern.
  3. MySQL nur, wenn gewuenscht: scriptfiles\mysql.ini auf Enabled=1 setzen
     und mysql.dll nach plugins\ legen. Ohne das laeuft der Server auf Dateien.
'@
