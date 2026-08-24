#!/usr/bin/env bash
#
# Jebiga-Gaming - Gamemode und Filterscripts uebersetzen
#
# Aufruf:  ./compile.sh [weitere pawncc-Optionen]
#
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TOOLCHAIN="$ROOT/.toolchain"

if [ ! -x "$TOOLCHAIN/pawncc" ]; then
    echo "Der Pawn-Compiler fehlt. Bitte zuerst ./setup.sh ausfuehren." >&2
    exit 1
fi
if [ ! -f "$ROOT/pawno/include/a_samp.inc" ]; then
    echo "Die SA-MP-Includes fehlen. Bitte zuerst ./setup.sh ausfuehren." >&2
    exit 1
fi

# Zusaetzliche Optionen von der Kommandozeile durchreichen. Sie werden hier
# festgehalten, weil "$@" innerhalb der Funktion deren eigene Argumente meint.
EXTRA_ARGS=("$@")

# -d3  volle Debug-Informationen (Zeilennummern in Laufzeitfehlern)
# -;+  Semikolon am Anweisungsende verpflichtend
# -(+  Klammern um Kontrollstrukturen verpflichtend
compile_one() {
    local source="$1" output="$2"

    LD_LIBRARY_PATH="$TOOLCHAIN" "$TOOLCHAIN/pawncc" \
        "$source" \
        -i"$ROOT" \
        -i"$ROOT/pawno/include" \
        -i"$TOOLCHAIN/include" \
        -o"$output" \
        -d3 '-;+' '-(+' \
        ${EXTRA_ARGS[@]+"${EXTRA_ARGS[@]}"}
}

echo "== Gamemode =="
compile_one "$ROOT/gamemodes/jebiga.pwn" "$ROOT/gamemodes/jebiga.amx"

# Filterscripts sind eigenstaendige Skripte und werden einzeln uebersetzt.
shopt -s nullglob
scripts=("$ROOT"/filterscripts/*.pwn)
shopt -u nullglob

if [ ${#scripts[@]} -gt 0 ]; then
    echo
    echo "== Filterscripts =="
    for script in "${scripts[@]}"; do
        name="$(basename "$script" .pwn)"
        echo "-- $name"
        compile_one "$script" "$ROOT/filterscripts/$name.amx"
    done
fi

echo
echo "Fertig:"
printf '  %-34s %s Bytes\n' "gamemodes/jebiga.amx" "$(stat -c%s "$ROOT/gamemodes/jebiga.amx")"
for script in "$ROOT"/filterscripts/*.amx; do
    [ -e "$script" ] || continue
    printf '  %-34s %s Bytes\n' "filterscripts/$(basename "$script")" "$(stat -c%s "$script")"
done
