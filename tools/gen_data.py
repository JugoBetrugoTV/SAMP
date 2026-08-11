#!/usr/bin/env python3
"""
Jebiga-Gaming - Generator fuer die Datenmodule unter src/data/

Die Tabellen in src/data/ sind nicht von Hand geschrieben, sondern aus der
Referenzdokumentation von open.mp erzeugt. Damit sind Skin-, Waffen-,
Bauteil-, Interior- und Animationsdaten belegt statt aus dem Gedaechtnis
zusammengetragen - bei mehreren tausend Eintraegen ist das der einzige Weg,
der nicht zu stillen Fehlern fuehrt.

Aufruf:

    git clone --depth 1 https://github.com/openmultiplayer/wiki /tmp/ompwiki
    python3 tools/gen_data.py /tmp/ompwiki

Die erzeugten .inc-Dateien werden eingecheckt; das Skript muss also nur laufen,
wenn die Referenzdaten aktualisiert werden sollen.
"""

import os
import re
import sys

HEADER = """/*
 * Jebiga-Gaming - {title}
 *
 * ERZEUGTE DATEI - NICHT VON HAND BEARBEITEN.
 * Erzeugt von tools/gen_data.py aus der Referenzdokumentation von open.mp
 * ({source}). Aenderungen gehoeren in den Generator.
 *
 * {note}
 */

#if defined _jg_{guard}_inc
    #endinput
#endif
#define _jg_{guard}_inc
"""


def clean(text: str) -> str:
    """Macht einen Tabellenwert fuer ein Pawn-Stringliteral brauchbar."""
    text = re.sub(r"!\[[^\]]*\]\([^)]*\)", " ", text)   # Bilder entfernen
    text = re.sub(r"<br\s*/?>", " ", text)              # Zeilenumbrueche
    text = re.sub(r"\[([^\]]*)\]\([^)]*\)", r"\1", text)  # Links entflachen
    text = text.replace("\\$", "$").replace("`", "")
    text = text.replace('"', "'")                       # Pawn-Strings schuetzen
    text = "".join(ch if 32 <= ord(ch) < 127 else " " for ch in text)
    return re.sub(r"\s+", " ", text).strip()


def rows(path: str):
    """Liefert die Datenzeilen einer Markdown-Tabelle als Spaltenlisten."""
    with open(path, encoding="utf-8") as handle:
        for line in handle:
            line = line.rstrip("\n")
            if not line.startswith("|"):
                continue
            if re.match(r"^\|[\s:|-]+\|?$", line):      # Trennzeile
                continue
            cols = [clean(c) for c in line.split("|")[1:-1]]
            if cols:
                yield cols


def rows_with_sections(path: str):
    """Wie rows(), liefert zusaetzlich die zuletzt gesehene ##-Ueberschrift."""
    section = ""
    with open(path, encoding="utf-8") as handle:
        for line in handle:
            line = line.rstrip("\n")
            if line.startswith("## "):
                section = clean(line[3:])
                continue
            if not line.startswith("|"):
                continue
            if re.match(r"^\|[\s:|-]+\|?$", line):
                continue
            cols = [clean(c) for c in line.split("|")[1:-1]]
            if cols:
                yield section, cols


def is_int(value: str) -> bool:
    return bool(re.fullmatch(r"-?\d+", value))


def write(path: str, text: str) -> None:
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, "w", encoding="utf-8") as handle:
        handle.write(text)
    print(f"  {path}: {text.count(chr(10)) + 1} Zeilen")


# ---------------------------------------------------------------------------
# Skins
# ---------------------------------------------------------------------------
def gen_skins(wiki: str, out: str) -> None:
    entries = {}
    for cols in rows(os.path.join(wiki, "skins.md")):
        if len(cols) < 6 or not is_int(cols[0]):
            continue
        skin = int(cols[0])
        name = cols[2] or f"Skin {skin}"
        female = cols[5].lower().startswith("female")
        entries[skin] = (name[:43], female)

    top = max(entries)
    missing = [i for i in range(top + 1) if i not in entries]
    if missing:
        raise SystemExit(f"Skinliste hat Luecken: {missing[:10]}")

    lines = [
        HEADER.format(
            title="Skindaten",
            source="docs/scripting/resources/skins.md",
            guard="skins",
            note=f"{len(entries)} Skins, luekenlos von 0 bis {top}.",
        ),
        "",
        f"#define SKIN_COUNT              ({len(entries)})",
        "",
        "enum E_SKIN_INFO",
        "{",
        "    skName[44],",
        "    bool:skFemale",
        "};",
        "",
        "new const gSkinInfo[SKIN_COUNT][E_SKIN_INFO] =",
        "{",
    ]
    for skin in range(top + 1):
        name, female = entries[skin]
        comma = "," if skin < top else ""
        lines.append(f'    {{"{name}", {"true" if female else "false"}}}{comma}'
                     f'   // {skin}')
    lines += [
        "};",
        "",
        "stock bool:IsValidSkin(skin)",
        "{",
        "    return (skin >= 0 && skin < SKIN_COUNT);",
        "}",
        "",
        "stock GetSkinName(skin)",
        "{",
        "    new name[44];",
        '    if (!IsValidSkin(skin)) name = "Unbekannt";',
        '    else format(name, sizeof name, "%s", gSkinInfo[skin][skName]);',
        "    return name;",
        "}",
        "",
        "stock bool:IsFemaleSkin(skin)",
        "{",
        "    if (!IsValidSkin(skin)) return false;",
        "    return gSkinInfo[skin][skFemale];",
        "}",
        "",
    ]
    write(os.path.join(out, "skins.inc"), "\n".join(lines))


# ---------------------------------------------------------------------------
# Waffen
# ---------------------------------------------------------------------------
def gen_weapons(wiki: str, out: str) -> None:
    entries = {}
    for cols in rows(os.path.join(wiki, "weaponids.md")):
        if len(cols) < 5 or not is_int(cols[3]):
            continue
        weapon = int(cols[3])
        if not 0 <= weapon <= 46:
            continue
        name = cols[0] or f"Waffe {weapon}"
        slot = int(cols[4]) if is_int(cols[4]) else 0
        entries[weapon] = (name[:27], slot)

    for weapon in range(47):
        entries.setdefault(weapon, (f"Waffe {weapon}", 0))

    lines = [
        HEADER.format(
            title="Waffendaten",
            source="docs/scripting/resources/weaponids.md",
            guard="weapons",
            note="47 Waffen (0 bis 46) mit Name und Inventarplatz.",
        ),
        "",
        "#define WEAPON_COUNT            (47)",
        "",
        "enum E_WEAPON_INFO",
        "{",
        "    wiName[28],",
        "    wiSlot",
        "};",
        "",
        "new const gWeaponInfo[WEAPON_COUNT][E_WEAPON_INFO] =",
        "{",
    ]
    for weapon in range(47):
        name, slot = entries[weapon]
        comma = "," if weapon < 46 else ""
        lines.append(f'    {{"{name}", {slot}}}{comma}   // {weapon}')
    lines += [
        "};",
        "",
        "stock bool:IsValidWeapon(weaponid)",
        "{",
        "    return (weaponid >= 0 && weaponid < WEAPON_COUNT);",
        "}",
        "",
        "stock GetWeaponNameEx(weaponid)",
        "{",
        "    new name[28];",
        '    if (!IsValidWeapon(weaponid)) name = "Unbekannt";',
        '    else format(name, sizeof name, "%s", gWeaponInfo[weaponid][wiName]);',
        "    return name;",
        "}",
        "",
        "stock GetWeaponSlot(weaponid)",
        "{",
        "    if (!IsValidWeapon(weaponid)) return -1;",
        "    return gWeaponInfo[weaponid][wiSlot];",
        "}",
        "",
        "/*",
        " * Sucht eine Waffe anhand einer ID oder eines Namensfragments.",
        " * Rueckgabe: Waffen-ID, -1 = nichts gefunden, -2 = mehrdeutig.",
        " */",
        "stock FindWeaponId(const input[])",
        "{",
        "    if (input[0] == EOS) return -1;",
        "",
        "    new bool:numeric = true;",
        "    for (new i = 0; input[i] != EOS; i++)",
        "    {",
        "        if (input[i] < '0' || input[i] > '9') { numeric = false; break; }",
        "    }",
        "    if (numeric)",
        "    {",
        "        new id = strval(input);",
        "        return IsValidWeapon(id) ? id : -1;",
        "    }",
        "",
        "    new found = -1, matches = 0;",
        "    for (new i = 0; i < WEAPON_COUNT; i++)",
        "    {",
        "        if (!strcmp(gWeaponInfo[i][wiName], input, true)) return i;",
        "        if (strfind(gWeaponInfo[i][wiName], input, true) != -1)",
        "        {",
        "            found = i;",
        "            matches++;",
        "        }",
        "    }",
        "    return (matches > 1) ? -2 : found;",
        "}",
        "",
    ]
    write(os.path.join(out, "weapons.inc"), "\n".join(lines))


# ---------------------------------------------------------------------------
# Tuningteile
# ---------------------------------------------------------------------------
def gen_components(wiki: str, out: str) -> None:
    entries = []
    for cols in rows(os.path.join(wiki, "carcomponentid.md")):
        if len(cols) < 6 or not is_int(cols[0]):
            continue
        comp = int(cols[0])
        if not 1000 <= comp <= 1193:
            continue
        part = cols[2] or "Sonstiges"
        kind = cols[3] or part
        price = re.sub(r"[^\d]", "", cols[5])
        entries.append((comp, part[:20], kind[:32], int(price) if price else 500))

    entries.sort()
    lines = [
        HEADER.format(
            title="Tuningbauteile",
            source="docs/scripting/resources/carcomponentid.md",
            guard="components",
            note=f"{len(entries)} Bauteile mit Kategorie, Bezeichnung und Originalpreis.",
        ),
        "",
        f"#define COMPONENT_COUNT         ({len(entries)})",
        "",
        "enum E_COMPONENT_INFO",
        "{",
        "    ciId,",
        "    ciPart[21],",
        "    ciType[33],",
        "    ciPrice",
        "};",
        "",
        "new const gComponentInfo[COMPONENT_COUNT][E_COMPONENT_INFO] =",
        "{",
    ]
    for index, (comp, part, kind, price) in enumerate(entries):
        comma = "," if index < len(entries) - 1 else ""
        lines.append(f'    {{{comp}, "{part}", "{kind}", {price}}}{comma}')
    lines += [
        "};",
        "",
        "stock Component_FindIndex(componentid)",
        "{",
        "    for (new i = 0; i < COMPONENT_COUNT; i++)",
        "    {",
        "        if (gComponentInfo[i][ciId] == componentid) return i;",
        "    }",
        "    return -1;",
        "}",
        "",
    ]
    write(os.path.join(out, "components.inc"), "\n".join(lines))


# ---------------------------------------------------------------------------
# Interiors
# ---------------------------------------------------------------------------
def gen_interiors(wiki: str, out: str) -> None:
    entries = []
    for section, cols in rows_with_sections(os.path.join(wiki, "interiorids.md")):
        if len(cols) < 5 or not is_int(cols[1]):
            continue
        try:
            x, y, z = float(cols[2]), float(cols[3]), float(cols[4])
        except ValueError:
            continue
        name = cols[0]
        if not name:
            continue
        entries.append((section[:24] or "Sonstige", name[:40], int(cols[1]), x, y, z))

    lines = [
        HEADER.format(
            title="Interiors",
            source="docs/scripting/resources/interiorids.md",
            guard="interiors",
            note=f"{len(entries)} Innenraeume mit Interior-ID und Koordinaten.",
        ),
        "",
        f"#define INTERIOR_COUNT          ({len(entries)})",
        "",
        "enum E_INTERIOR_INFO",
        "{",
        "    itCategory[25],",
        "    itName[41],",
        "    itInterior,",
        "    Float:itX,",
        "    Float:itY,",
        "    Float:itZ",
        "};",
        "",
        "new const gInteriorInfo[INTERIOR_COUNT][E_INTERIOR_INFO] =",
        "{",
    ]
    for index, (section, name, interior, x, y, z) in enumerate(entries):
        comma = "," if index < len(entries) - 1 else ""
        lines.append(f'    {{"{section}", "{name}", {interior}, '
                     f'{x:.4f}, {y:.4f}, {z:.4f}}}{comma}')
    lines += ["};", ""]
    write(os.path.join(out, "interiors.inc"), "\n".join(lines))


# ---------------------------------------------------------------------------
# Animationen
# ---------------------------------------------------------------------------
def gen_animations(wiki: str, out: str) -> None:
    entries = []
    seen = set()
    for cols in rows(os.path.join(wiki, "animations.md")):
        if len(cols) < 3 or not is_int(cols[0]):
            continue
        library, animation = cols[1], cols[2]
        if not library or not animation:
            continue
        if not re.fullmatch(r"[A-Za-z0-9_]+", library):
            continue
        if not re.fullmatch(r"[A-Za-z0-9_]+", animation):
            continue
        key = (library.upper(), animation)
        if key in seen:
            continue
        seen.add(key)
        entries.append((library[:20], animation[:32]))

    entries.sort(key=lambda e: (e[0].upper(), e[1].upper()))
    libraries = sorted({e[0].upper() for e in entries})

    lines = [
        HEADER.format(
            title="Animationen",
            source="docs/scripting/resources/animations.md",
            guard="anims",
            note=(f"{len(entries)} Animationen aus {len(libraries)} Bibliotheken, "
                  "nach Bibliothek sortiert."),
        ),
        "",
        f"#define ANIMATION_COUNT         ({len(entries)})",
        f"#define ANIMATION_LIBRARIES     ({len(libraries)})",
        "",
        "enum E_ANIMATION_INFO",
        "{",
        "    aiLibrary[21],",
        "    aiName[33]",
        "};",
        "",
        "// Nach Bibliothek sortiert, damit ein Bereich am Stueck ausgegeben",
        "// werden kann, ohne die gesamte Tabelle zu durchlaufen.",
        "new const gAnimationInfo[ANIMATION_COUNT][E_ANIMATION_INFO] =",
        "{",
    ]
    for index, (library, animation) in enumerate(entries):
        comma = "," if index < len(entries) - 1 else ""
        lines.append(f'    {{"{library}", "{animation}"}}{comma}')
    lines += [
        "};",
        "",
        "new const gAnimationLibraries[ANIMATION_LIBRARIES][21] =",
        "{",
    ]
    for index, library in enumerate(libraries):
        comma = "," if index < len(libraries) - 1 else ""
        lines.append(f'    "{library}"{comma}')
    lines += ["};", ""]
    write(os.path.join(out, "animations.inc"), "\n".join(lines))


def main() -> None:
    if len(sys.argv) < 2:
        raise SystemExit("Aufruf: gen_data.py <pfad-zum-openmultiplayer-wiki>")

    wiki = os.path.join(sys.argv[1], "docs", "scripting", "resources")
    if not os.path.isdir(wiki):
        raise SystemExit(f"Referenzdaten nicht gefunden: {wiki}")

    out = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))),
                       "src", "data")

    print("Erzeuge Datenmodule:")
    gen_skins(wiki, out)
    gen_weapons(wiki, out)
    gen_components(wiki, out)
    gen_interiors(wiki, out)
    gen_animations(wiki, out)
    print("Fertig.")


if __name__ == "__main__":
    main()
