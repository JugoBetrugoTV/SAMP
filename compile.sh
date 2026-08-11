#!/usr/bin/env bash
#
# UIF Freeroam - Gamemode uebersetzen
#
# Aufruf:  ./compile.sh [weitere pawncc-Optionen]
#
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TOOLCHAIN="$ROOT/.toolchain"
SOURCE="$ROOT/gamemodes/uif.pwn"
OUTPUT="$ROOT/gamemodes/uif.amx"

if [ ! -x "$TOOLCHAIN/pawncc" ]; then
    echo "Der Pawn-Compiler fehlt. Bitte zuerst ./setup.sh ausfuehren." >&2
    exit 1
fi
if [ ! -f "$ROOT/pawno/include/a_samp.inc" ]; then
    echo "Die SA-MP-Includes fehlen. Bitte zuerst ./setup.sh ausfuehren." >&2
    exit 1
fi

# -d3  volle Debug-Informationen (Zeilennummern in Laufzeitfehlern)
# -;+  Semikolon am Anweisungsende verpflichtend
# -(+  Klammern um Kontrollstrukturen verpflichtend
LD_LIBRARY_PATH="$TOOLCHAIN" "$TOOLCHAIN/pawncc" \
    "$SOURCE" \
    -i"$ROOT" \
    -i"$ROOT/pawno/include" \
    -i"$TOOLCHAIN/include" \
    -o"$OUTPUT" \
    -d3 '-;+' '-(+' \
    "$@"

echo
echo "Fertig: ${OUTPUT#$ROOT/} ($(stat -c%s "$OUTPUT") Bytes)"
