/*
 * Jebiga-Gaming - Filterscript: Tore
 *
 * Automatische Schranken und Tore an mehreren Orten der Karte. Wer sich
 * naehert, oeffnet sie; wer sich entfernt, schliesst sie wieder.
 *
 * Bewusst als Filterscript und nicht im Gamemode: Tore lassen sich so im
 * laufenden Betrieb mit /rcon reloadfs neu laden, ohne dass Spieler
 * herausfliegen.
 *
 * Die Tore bewegen sich mit MoveObject zwischen zwei Positionen. Gearbeitet
 * wird mit gewoehnlichen Objekten statt mit dynamischen: es sind wenige,
 * ortsfeste Objekte, und so bleibt das Filterscript unabhaengig vom Streamer.
 */

#include <a_samp>

#define GATE_CHECK_INTERVAL     (700)    // ms zwischen zwei Naeherungspruefungen
#define GATE_SPEED              (2.5)

enum E_GATE
{
    gtName[32],
    gtModel,
    Float:gtRange,          // ab dieser Entfernung oeffnet das Tor
    Float:gtClosedX,        // geschlossene Position
    Float:gtClosedY,
    Float:gtClosedZ,
    Float:gtOpenX,          // offene Position
    Float:gtOpenY,
    Float:gtOpenZ,
    Float:gtRotX,
    Float:gtRotY,
    Float:gtRotZ
};

/*
 * Modell 980 ist die Schranke "airportgate" - ein breites Schiebetor, das sich
 * fuer Ein- und Ausfahrten eignet. Geoeffnet wird durch Absenken unter den
 * Boden; das ist unauffaellig und braucht keine zweite Drehung.
 */
static const gGates[][E_GATE] =
{
    {"Area 51 - Haupttor",      980,  12.0,   135.0000, 1941.0000,  19.0000,   135.0000, 1941.0000,  13.0000,  0.0, 0.0,  90.0},
    {"Area 51 - Hangar",        980,  12.0,   213.0000, 1907.0000,  17.0000,   213.0000, 1907.0000,  11.0000,  0.0, 0.0,   0.0},
    {"LS Flughafen - Vorfeld",  980,  14.0,  1685.0000, -2330.0000, 13.0000,  1685.0000, -2330.0000,  7.0000,  0.0, 0.0,   0.0},
    {"SF Werft - Zufahrt",      980,  12.0, -2438.0000,  2283.0000,  4.5000, -2438.0000,  2283.0000, -1.5000,  0.0, 0.0,  90.0},
    {"LV Flughafen - Tor",      980,  14.0,  1685.0000,  1443.0000, 10.3000,  1685.0000,  1443.0000,  4.3000,  0.0, 0.0,   0.0}
};

static gGateObject[sizeof gGates];
static bool:gGateOpen[sizeof gGates];

public OnFilterScriptInit()
{
    for (new i = 0; i < sizeof gGates; i++)
    {
        gGateObject[i] = CreateObject(gGates[i][gtModel],
            gGates[i][gtClosedX], gGates[i][gtClosedY], gGates[i][gtClosedZ],
            gGates[i][gtRotX], gGates[i][gtRotY], gGates[i][gtRotZ]);
        gGateOpen[i] = false;
    }

    SetTimer("Gates_Check", GATE_CHECK_INTERVAL, true);

    printf("[Tore] %d Tore erstellt.", sizeof gGates);
    return 1;
}

public OnFilterScriptExit()
{
    for (new i = 0; i < sizeof gGates; i++)
    {
        if (gGateObject[i]) DestroyObject(gGateObject[i]);
    }
    return 1;
}

/*
 * Prueft fuer jedes Tor, ob jemand in Reichweite ist. Die Schleife laeuft ueber
 * die Tore aussen und die Spieler innen - so bricht sie beim ersten Treffer ab,
 * statt fuer jeden Spieler alle Tore zu pruefen.
 */
forward Gates_Check();
public Gates_Check()
{
    new maxPlayers = GetPlayerPoolSize();

    for (new i = 0; i < sizeof gGates; i++)
    {
        new bool:someoneNear = false;

        for (new p = 0; p <= maxPlayers; p++)
        {
            if (!IsPlayerConnected(p) || IsPlayerNPC(p)) continue;

            if (IsPlayerInRangeOfPoint(p, gGates[i][gtRange],
                    gGates[i][gtClosedX], gGates[i][gtClosedY], gGates[i][gtClosedZ]))
            {
                someoneNear = true;
                break;
            }
        }

        if (someoneNear == gGateOpen[i]) continue;   // Zustand passt bereits

        if (someoneNear)
        {
            MoveObject(gGateObject[i], gGates[i][gtOpenX], gGates[i][gtOpenY], gGates[i][gtOpenZ], GATE_SPEED);
            gGateOpen[i] = true;
        }
        else
        {
            MoveObject(gGateObject[i], gGates[i][gtClosedX], gGates[i][gtClosedY], gGates[i][gtClosedZ], GATE_SPEED);
            gGateOpen[i] = false;
        }
    }
    return 1;
}

public OnPlayerCommandText(playerid, cmdtext[])
{
    if (strcmp(cmdtext, "/gates", true) != 0) return 0;

    new body[512], entry[96];
    for (new i = 0; i < sizeof gGates; i++)
    {
        format(entry, sizeof entry, "%s%s  %s\n",
            "{FFFFFF}", gGates[i][gtName], gGateOpen[i] ? "{2ECC71}offen" : "{AFAFAF}geschlossen");
        strcat(body, entry);
    }
    strcat(body, "\n{AFAFAF}Tore oeffnen sich von selbst, sobald jemand naeher kommt.");

    ShowPlayerDialog(playerid, 9100, DIALOG_STYLE_MSGBOX, "Tore", body, "Schliessen", "");
    return 1;
}
