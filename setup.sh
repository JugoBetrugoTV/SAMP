#!/usr/bin/env bash
#
# Jebiga-Gaming - Einrichtung der Build-Umgebung
#
# Holt die SA-MP-Standardincludes und baut den Pawn-Compiler aus den Quellen.
# Beides landet in Verzeichnissen, die von Git ignoriert werden - im Repository
# liegt ausschliesslich eigener Quellcode.
#
# Aufruf:  ./setup.sh
#
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TOOLCHAIN="$ROOT/.toolchain"
INCLUDE_DIR="$ROOT/pawno/include"
BUILD_DIR="$ROOT/.build"

SAMP_STDLIB_REPO="https://github.com/pawn-lang/samp-stdlib.git"
# Bewusst ein Commit vom master-Branch und nicht das Release 0.3.7-R2-2-1:
# das Release enthaelt die unveraenderten Original-Includes von SA-MP. Die
# deklarieren print/printf selbst, was mit der console.inc des Pawn-Compilers
# kollidiert, und fuehren SHA256_PassHash ohne const-Parameter - beides
# verhindert eine Uebersetzung. Der master-Zweig ist auf genau diesen Compiler
# abgestimmt; der feste Commit haelt den Build reproduzierbar.
SAMP_STDLIB_COMMIT="8ffb055624308b25521665b60e78b5e6e6b3717f"
COMPILER_REPO="https://github.com/pawn-lang/compiler.git"
COMPILER_TAG="v3.10.10"

# Streamer-Plugin (Incognito). Wird als 32-Bit-Shared-Object gebaut - SA-MP
# laedt ausschliesslich 32-Bit-Plugins, auch auf 64-Bit-Systemen.
STREAMER_REPO="https://github.com/samp-incognito/samp-streamer-plugin.git"
STREAMER_COMMIT="49d48e515b9a095f016fb6a208a2cd94c42acda5"

# sscanf2 (maddinat0r) und crashdetect (Fork von Y-Less). Beide brauchen
# subhook; das Originalrepository von Zeex ist nicht mehr erreichbar, deshalb
# wird der gepflegte Fork eingesetzt.
SSCANF_REPO="https://github.com/maddinat0r/sscanf.git"
SSCANF_COMMIT="27f0b725095bf40ed9c4d772acd7300b02be632b"
CRASHDETECT_REPO="https://github.com/Y-Less/samp-plugin-crashdetect.git"
CRASHDETECT_COMMIT="2bb17d36e098c32a3fad42046e2fa031d2ec6cda"
SUBHOOK_FORK="https://github.com/Y-Less/subhook.git"

# MySQL-Plugin: hier wird nur die Include-Datei erzeugt, damit das Gamemode
# uebersetzt. Das Plugin selbst haengt an einer Abhaengigkeitskette, die sich
# nicht zuverlaessig automatisch bauen laesst - siehe README.
MYSQL_REPO="https://github.com/pBlueG/SA-MP-MySQL.git"
MYSQL_VERSION="R41-4"

log() { printf '\033[1;36m==>\033[0m %s\n' "$1"; }
die() { printf '\033[1;31mFehler:\033[0m %s\n' "$1" >&2; exit 1; }

for tool in git cmake make gcc; do
    command -v "$tool" >/dev/null 2>&1 || die "'$tool' wird benoetigt, ist aber nicht installiert."
done

mkdir -p "$TOOLCHAIN/include" "$INCLUDE_DIR" "$BUILD_DIR" "$ROOT/plugins"

# --- SA-MP Standardincludes -------------------------------------------------
if [ -f "$INCLUDE_DIR/a_samp.inc" ]; then
    log "SA-MP-Includes bereits vorhanden - uebersprungen."
else
    log "Hole SA-MP-Includes (${SAMP_STDLIB_COMMIT:0:8})..."
    rm -rf "$BUILD_DIR/samp-stdlib"
    git clone "$SAMP_STDLIB_REPO" "$BUILD_DIR/samp-stdlib"
    git -C "$BUILD_DIR/samp-stdlib" checkout --quiet "$SAMP_STDLIB_COMMIT"
    cp "$BUILD_DIR/samp-stdlib"/*.inc "$INCLUDE_DIR/"
    log "$(ls -1 "$INCLUDE_DIR"/*.inc | wc -l) Includes installiert."
fi

# --- Pawn-Compiler ----------------------------------------------------------
if [ -x "$TOOLCHAIN/pawncc" ]; then
    log "Pawn-Compiler bereits vorhanden - uebersprungen."
else
    log "Baue Pawn-Compiler ($COMPILER_TAG) aus den Quellen..."
    rm -rf "$BUILD_DIR/compiler"
    git clone --depth 1 --branch "$COMPILER_TAG" "$COMPILER_REPO" "$BUILD_DIR/compiler"

    mkdir -p "$BUILD_DIR/compiler/source/compiler/build"
    (
        cd "$BUILD_DIR/compiler/source/compiler/build"
        cmake .. -DCMAKE_BUILD_TYPE=Release >/dev/null
        # Nur pawncc bauen: das Hilfsziel pawnruns laesst sich mit aktuellen
        # GCC-Versionen nicht uebersetzen und wird hier nicht gebraucht.
        make pawncc -j"$(nproc)" >/dev/null
    )

    cp "$BUILD_DIR/compiler/source/compiler/build/pawncc"     "$TOOLCHAIN/"
    cp "$BUILD_DIR/compiler/source/compiler/build/libpawnc.so" "$TOOLCHAIN/"
    cp "$BUILD_DIR/compiler/include"/*.inc "$TOOLCHAIN/include/"
    log "Compiler gebaut: $("$TOOLCHAIN/pawncc" 2>&1 | head -1)"
fi

# --- Streamer-Plugin --------------------------------------------------------
if [ -f "$ROOT/plugins/streamer.so" ] && [ -f "$INCLUDE_DIR/streamer.inc" ]; then
    log "Streamer-Plugin bereits vorhanden - uebersprungen."
else
    if ! echo 'int main(){return 0;}' | g++ -m32 -x c++ - -o /dev/null 2>/dev/null; then
        die "Fuer das Streamer-Plugin wird eine 32-Bit-C++-Toolchain benoetigt.
       Debian/Ubuntu: apt-get install g++-multilib lib32stdc++-14-dev
       Fedora:        dnf install glibc-devel.i686 libstdc++-devel.i686"
    fi

    log "Baue Streamer-Plugin (${STREAMER_COMMIT:0:8})..."
    rm -rf "$BUILD_DIR/streamer"
    git clone "$STREAMER_REPO" "$BUILD_DIR/streamer"
    git -C "$BUILD_DIR/streamer" checkout --quiet "$STREAMER_COMMIT"
    git -C "$BUILD_DIR/streamer" submodule update --init --depth 1 --recursive

    mkdir -p "$BUILD_DIR/streamer/build"
    (
        cd "$BUILD_DIR/streamer/build"
        cmake .. -DCMAKE_BUILD_TYPE=Release >/dev/null
        make -j"$(nproc)" >/dev/null
    )

    cp "$BUILD_DIR/streamer/build/bin/streamer.so" "$ROOT/plugins/"
    cp "$BUILD_DIR/streamer/streamer.inc"          "$INCLUDE_DIR/"
    log "Streamer gebaut: $(file -b "$ROOT/plugins/streamer.so" | cut -d, -f1-2)"
fi

# --- Optionale Plugins ------------------------------------------------------
#
# streamer ist Pflicht - ohne ihn fehlt die Custom Map. sscanf2 und crashdetect
# sind Zugaben: schlaegt ihr Build fehl, laeuft die Einrichtung weiter, damit
# wenigstens a_mysql.inc entsteht und das Gamemode uebersetzt. Der Hinweis am
# Ende sagt, was dann aus server.cfg zu streichen ist.
OPTIONAL_FAILED=()

build_sscanf() {
    log "Baue sscanf2 (${SSCANF_COMMIT:0:8})..."
    rm -rf "$BUILD_DIR/sscanf"
    git clone "$SSCANF_REPO" "$BUILD_DIR/sscanf"
    git -C "$BUILD_DIR/sscanf" checkout --quiet "$SSCANF_COMMIT"

    # sscanf2 zieht subhook als Submodul; das Original von Zeex ist nicht mehr
    # erreichbar, deshalb auf den gepflegten Fork umbiegen.
    if [ -f "$BUILD_DIR/sscanf/.gitmodules" ]; then
        sed -i "s#url = .*subhook.*#url = $SUBHOOK_FORK#" "$BUILD_DIR/sscanf/.gitmodules"
        git -C "$BUILD_DIR/sscanf" submodule sync --quiet
    fi
    git -C "$BUILD_DIR/sscanf" submodule update --init --depth 1 --recursive

    mkdir -p "$BUILD_DIR/sscanf/build"
    (
        cd "$BUILD_DIR/sscanf/build"
        cmake .. -DCMAKE_BUILD_TYPE=Release >/dev/null
        make -j"$(nproc)" >/dev/null
    )
    cp "$BUILD_DIR/sscanf/build/libsscanf.so" "$ROOT/plugins/sscanf.so"
    find "$BUILD_DIR/sscanf" -maxdepth 1 -name "*.inc" -exec cp {} "$INCLUDE_DIR/" \;
}

build_crashdetect() {
    log "Baue crashdetect (${CRASHDETECT_COMMIT:0:8})..."
    rm -rf "$BUILD_DIR/crashdetect"
    git clone "$CRASHDETECT_REPO" "$BUILD_DIR/crashdetect"
    git -C "$BUILD_DIR/crashdetect" checkout --quiet "$CRASHDETECT_COMMIT"

    # crashdetect holt seine Abhaengigkeiten ueber CMake und hat deshalb gar
    # keine .gitmodules - der Aufruf bleibt trotzdem stehen, falls sich das
    # aendert.
    if [ -f "$BUILD_DIR/crashdetect/.gitmodules" ]; then
        sed -i "s#url = .*Zeex/subhook.*#url = $SUBHOOK_FORK#" "$BUILD_DIR/crashdetect/.gitmodules"
        git -C "$BUILD_DIR/crashdetect" submodule sync --quiet
        git -C "$BUILD_DIR/crashdetect" submodule update --init --depth 1 --recursive
    fi

    # Der mitgelieferte AMX-Interpreter benutzt "labels as values". Seit GCC 12
    # verlangt der Compiler dort einen echten Zeigertyp, sonst bricht der Build
    # mit "computed goto must be pointer type" ab.
    sed -i 's|goto \*\*cip++;|goto *(void *)*cip++;|g' "$BUILD_DIR/crashdetect/src/amx/amx.c"

    mkdir -p "$BUILD_DIR/crashdetect/build"
    (
        cd "$BUILD_DIR/crashdetect/build"
        cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF >/dev/null
        make -j"$(nproc)" >/dev/null
    )
    cp "$BUILD_DIR/crashdetect/build/crashdetect.so" "$ROOT/plugins/"
    find "$BUILD_DIR/crashdetect" -maxdepth 2 -name "crashdetect.inc" \
        -exec cp {} "$INCLUDE_DIR/" \; 2>/dev/null || true
}

if [ -f "$ROOT/plugins/sscanf.so" ]; then
    log "sscanf2 bereits vorhanden - uebersprungen."
elif build_sscanf; then
    log "sscanf2 gebaut."
else
    log "sscanf2 konnte nicht gebaut werden - wird uebersprungen."
    OPTIONAL_FAILED+=("sscanf.so")
fi

if [ -f "$ROOT/plugins/crashdetect.so" ]; then
    log "crashdetect bereits vorhanden - uebersprungen."
elif build_crashdetect; then
    log "crashdetect gebaut."
else
    log "crashdetect konnte nicht gebaut werden - wird uebersprungen."
    OPTIONAL_FAILED+=("crashdetect.so")
fi

# --- MySQL-Include ----------------------------------------------------------
if [ -f "$INCLUDE_DIR/a_mysql.inc" ]; then
    log "a_mysql.inc bereits vorhanden - uebersprungen."
else
    log "Erzeuge a_mysql.inc ($MYSQL_VERSION)..."
    rm -rf "$BUILD_DIR/mysql"
    git clone --depth 1 "$MYSQL_REPO" "$BUILD_DIR/mysql"
    sed "s/@MYSQL_PLUGIN_VERSION@/$MYSQL_VERSION/g" \
        "$BUILD_DIR/mysql/a_mysql.inc.in" > "$INCLUDE_DIR/a_mysql.inc"
    log "a_mysql.inc erzeugt. Das Plugin selbst muss separat bereitgestellt werden - siehe README."
fi

# --- Laufzeitverzeichnisse --------------------------------------------------
mkdir -p "$ROOT/scriptfiles/accounts" "$ROOT/scriptfiles/logs"

if [ ${#OPTIONAL_FAILED[@]} -gt 0 ]; then
    echo
    echo "Achtung: diese optionalen Plugins fehlen: ${OPTIONAL_FAILED[*]}"
    echo "         Bitte aus der plugins-Zeile in server.cfg streichen,"
    echo "         sonst verweigert der Server den Start."
fi

cat <<'EOF'

Einrichtung abgeschlossen.

  ./compile.sh        Gamemode und Filterscripts uebersetzen
  ./run.sh            Server starten        (benoetigt das SA-MP-Serverpaket)

Das SA-MP-Serverpaket (samp03svr) ist nicht Teil dieses Repositories und wird
von setup.sh auch nicht heruntergeladen - siehe README.md, Abschnitt
"Serverpaket besorgen".
EOF
