#!/usr/bin/env bash
#
# UIF Freeroam - Server starten
#
# Erwartet das SA-MP-Serverpaket (samp03svr) im Projektverzeichnis.
# Siehe README.md, Abschnitt "Serverpaket besorgen".
#
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$ROOT"

if [ ! -f gamemodes/uif.amx ]; then
    echo "gamemodes/uif.amx fehlt - bitte zuerst ./compile.sh ausfuehren." >&2
    exit 1
fi

SERVER=""
for candidate in ./samp03svr ./omp-server ./announce; do
    [ -x "$candidate" ] && [ "$candidate" != "./announce" ] && { SERVER="$candidate"; break; }
done

if [ -z "$SERVER" ]; then
    cat >&2 <<'EOF'
Es wurde keine Serveranwendung gefunden (erwartet: ./samp03svr oder ./omp-server).

Das Serverpaket ist proprietaer und darf hier nicht mitgeliefert werden.
README.md beschreibt unter "Serverpaket besorgen", woher es kommt und welche
Dateien anschliessend in diesem Verzeichnis liegen muessen.
EOF
    exit 1
fi

if grep -q "CHANGE_ME_BEFORE_FIRST_START" server.cfg; then
    echo "Warnung: das rcon_password in server.cfg ist noch der Platzhalter." >&2
    echo "         Bitte vor dem Betrieb im Internet aendern." >&2
fi

mkdir -p scriptfiles/accounts scriptfiles/logs
chmod +x "$SERVER"

echo "Starte $SERVER ..."
exec "$SERVER"
