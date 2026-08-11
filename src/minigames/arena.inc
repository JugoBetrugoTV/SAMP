/*
 * Jebiga-Gaming - Deathmatch-Arenen und Duelle
 *
 * Zwei verwandte Spielarten in einem Modul:
 *
 *  - Arenen: dauerhaft offene Deathmatch-Gebiete. Jede liegt in einer eigenen
 *    virtuellen Welt, damit dort niemand mit dem normalen Freeroam kollidiert.
 *    Beigetreten wird jederzeit, gestorben wird beliebig oft.
 *
 *  - Duelle: 1 gegen 1 auf Einladung. Beide Kaempfer landen auf einem eigenen
 *    Duellplatz mit eigener Welt; wer zuerst stirbt, verliert.
 *
 * Die Schauplaetze sind bewusst bestehende, flache Orte der Karte - so braucht
 * es keine zusaetzlichen Objekte und keine Annahmen ueber Bodenhoehen, die
 * sich nicht pruefen lassen.
 */

#if defined _jebiga_arena_included
    #endinput
#endif
#define _jebiga_arena_included

#define ARENA_WORLD_BASE        (800)   // 800 + Arenaindex
#define ARENA_WEAPONS           (3)
#define ARENA_SPAWN_RADIUS      (25.0)

#define MAX_DUELS               (24)
#define DUEL_INVITE_TIMEOUT     (45)    // Sekunden
#define DUEL_PRIZE              (10000)

enum E_ARENA
{
    arName[32],
    Float:arX,
    Float:arY,
    Float:arZ,
    arWeapons[ARENA_WEAPONS],
    arAmmo
};

static const gArenas[][E_ARENA] =
{
    //  Name                  X           Y          Z        Waffen              Munition
    {"Steinbruch - Sturmgewehre", 595.0000,  852.0000, -42.5000, {31, 24, 25},  500},
    {"Area 51 - Schrotflinten",   135.0000, 1943.0000,  19.9000, {27, 26, 24},  300},
    {"Chiliad - Scharfschuetzen",-2340.0000,-1600.0000, 483.7000, {34, 33, 23}, 150},
    {"LS Stadion - Raketen",     2769.0000,-1618.0000,  12.1000, {35, 16, 24},   40},
    {"Bayside - Nahkampf",      -2394.0000, 2321.0000,   4.9800, { 4,  0,  0},    1}
};

enum E_DUEL
{
    duActive,
    duPlayerA,
    duPlayerB,
    duWeapon,
    duStart
};

static gDuels[MAX_DUELS][E_DUEL];

// Duellplaetze - dieselben Orte wie die Arenen, aber je Duell eigene Welt
static const Float:gDuelSpots[][3] =
{
    { 595.0000,  852.0000, -42.5000},
    { 135.0000, 1943.0000,  19.9000},
    { 400.0000, 2536.0000,  16.5000},
    {2769.0000,-1618.0000,  12.1000}
};

stock Arena_OnGameModeInit()
{
    for (new i = 0; i < MAX_DUELS; i++) gDuels[i][duActive] = 0;

    new msg[64];
    format(msg, sizeof msg, "%d DM-Arenen und %d Duellplaetze bereit.", sizeof gArenas, MAX_DUELS);
    Log(msg);
    return 1;
}

// ---------------------------------------------------------------------------
// Deathmatch-Arenen
// ---------------------------------------------------------------------------
stock Arena_CountPlayers(index)
{
    new count = 0;
    foreach_player(i)
    {
        if (PlayerData[i][pArena] == index) count++;
    }
    return count;
}

/*
 * Setzt einen Spieler an eine zufaellige Stelle im Umkreis des Arenamittelpunkts.
 */
static Arena_PlacePlayer(playerid, index)
{
    new Float:angle = float(random(360));
    new Float:distance = float(random(floatround(ARENA_SPAWN_RADIUS)));

    new Float:x = gArenas[index][arX] + distance * floatsin(-angle, degrees);
    new Float:y = gArenas[index][arY] + distance * floatcos(-angle, degrees);

    SetPlayerPos(playerid, x, y, gArenas[index][arZ] + 1.0);
    SetPlayerFacingAngle(playerid, angle + 180.0);
    SetPlayerInterior(playerid, 0);
    SetPlayerVirtualWorld(playerid, ARENA_WORLD_BASE + index);
    SetCameraBehindPlayer(playerid);

    SetPlayerHealth(playerid, 100.0);
    SetPlayerArmour(playerid, 0.0);
    ResetPlayerWeapons(playerid);

    for (new w = 0; w < ARENA_WEAPONS; w++)
    {
        if (gArenas[index][arWeapons][w] == 0) continue;
        GivePlayerWeapon(playerid, gArenas[index][arWeapons][w], gArenas[index][arAmmo]);
    }
    GivePlayerSpawnProtection(playerid, 2000);
    return 1;
}

stock Arena_Join(playerid, index)
{
    if (index < 0 || index >= sizeof gArenas) return 0;

    PlayerData[playerid][pMinigame]   = MINIGAME_DM;
    PlayerData[playerid][pArena]      = index;
    PlayerData[playerid][pArenaKills] = 0;

    Arena_PlacePlayer(playerid, index);

    new msg[144];
    format(msg, sizeof msg, "Arena %s%s%s betreten - %d Spieler drin. Raus mit /leave.",
        EC_WHITE, gArenas[index][arName], EC_GREEN, Arena_CountPlayers(index));
    SendSuccess(playerid, msg);
    return 1;
}

stock Arena_Leave(playerid, bool:teleportOut = true)
{
    if (PlayerData[playerid][pArena] == -1) return 0;

    new msg[128];
    format(msg, sizeof msg, "Arena verlassen. Kills in dieser Runde: %d",
        PlayerData[playerid][pArenaKills]);

    PlayerData[playerid][pArena]     = -1;
    PlayerData[playerid][pMinigame]  = MINIGAME_NONE;
    PlayerData[playerid][pArenaKills] = 0;

    if (IsPlayerConnected(playerid))
    {
        ResetPlayerWeapons(playerid);
        SetPlayerVirtualWorld(playerid, WORLD_MAIN);

        if (teleportOut)
        {
            Teleport_SpawnPlayer(playerid);
            GivePlayerSpawnProtection(playerid);
        }
        SendInfo(playerid, msg);
    }
    return 1;
}

stock Arena_ShowList(playerid)
{
    new list[640], entry[128];

    for (new i = 0; i < sizeof gArenas; i++)
    {
        format(entry, sizeof entry, "%s%s  %s%d Spieler\n",
            EC_WHITE, gArenas[i][arName], EC_GREY, Arena_CountPlayers(i));
        strcat(list, entry);
    }

    return ShowPlayerDialog(playerid, DIALOG_DM_LIST, DIALOG_STYLE_LIST,
        "Deathmatch-Arenen", list, "Betreten", "Abbrechen");
}

// ---------------------------------------------------------------------------
// Duelle
// ---------------------------------------------------------------------------
static Duel_FindFreeSlot()
{
    for (new i = 0; i < MAX_DUELS; i++)
    {
        if (!gDuels[i][duActive]) return i;
    }
    return -1;
}

stock Duel_End(slot, winner, const reason[])
{
    if (!gDuels[slot][duActive]) return 0;

    new a = gDuels[slot][duPlayerA];
    new b = gDuels[slot][duPlayerB];

    gDuels[slot][duActive] = 0;

    for (new i = 0; i < 2; i++)
    {
        new id = (i == 0) ? a : b;
        if (id == INVALID_PLAYER_ID || !IsPlayerConnected(id)) continue;

        PlayerData[id][pMinigame] = MINIGAME_NONE;
        PlayerData[id][pDuelSlot] = -1;

        ResetPlayerWeapons(id);
        SetPlayerVirtualWorld(id, WORLD_MAIN);
        Teleport_SpawnPlayer(id);
        GivePlayerSpawnProtection(id);
    }

    if (winner != INVALID_PLAYER_ID && IsPlayerConnected(winner))
    {
        GivePlayerMoney(winner, DUEL_PRIZE);
        PlayerData[winner][pScore] += 2;
        SetPlayerScore(winner, PlayerData[winner][pScore]);

        new money[24], msg[176];
        FormatNumber(DUEL_PRIZE, money, sizeof money);
        format(msg, sizeof msg, "%s gewinnt das Duell (%s) und erhaelt $%s!",
            GetName(winner), reason, money);
        SendClientMessageToAll(COL_ORANGE, msg);
    }
    return 1;
}

stock Duel_Start(playerid, targetid, weapon)
{
    new slot = Duel_FindFreeSlot();
    if (slot == -1)
    {
        SendError(playerid, "Gerade sind alle Duellplaetze belegt.");
        return 0;
    }

    new spot = slot % sizeof gDuelSpots;
    new world = WORLD_DUEL_BASE + slot;

    gDuels[slot][duActive]  = 1;
    gDuels[slot][duPlayerA] = playerid;
    gDuels[slot][duPlayerB] = targetid;
    gDuels[slot][duWeapon]  = weapon;
    gDuels[slot][duStart]   = gettime();

    // Beide Kaempfer 12 Meter auseinander, mit Blick zueinander
    for (new i = 0; i < 2; i++)
    {
        new id = (i == 0) ? playerid : targetid;
        new Float:offset = (i == 0) ? -6.0 : 6.0;

        PlayerData[id][pMinigame] = MINIGAME_DUEL;
        PlayerData[id][pDuelSlot] = slot;

        SetPlayerPos(id, gDuelSpots[spot][0] + offset, gDuelSpots[spot][1], gDuelSpots[spot][2] + 1.0);
        SetPlayerFacingAngle(id, (i == 0) ? 90.0 : 270.0);
        SetPlayerInterior(id, 0);
        SetPlayerVirtualWorld(id, world);
        SetCameraBehindPlayer(id);

        SetPlayerHealth(id, 100.0);
        SetPlayerArmour(id, 0.0);
        ResetPlayerWeapons(id);
        if (weapon > 0) GivePlayerWeapon(id, weapon, 1000);

        GivePlayerSpawnProtection(id, 3000);
    }

    new msg[160];
    format(msg, sizeof msg, "Duell: %s gegen %s. Viel Glueck!", GetName(playerid), GetName(targetid));
    SendClientMessage(playerid, COL_ORANGE, msg);
    SendClientMessage(targetid, COL_ORANGE, msg);
    return 1;
}

// ---------------------------------------------------------------------------
// Callbacks
// ---------------------------------------------------------------------------
stock Arena_OnPlayerDeath(playerid, killerid)
{
    // Duell: der Tod entscheidet
    if (PlayerData[playerid][pDuelSlot] != -1)
    {
        new slot = PlayerData[playerid][pDuelSlot];
        new winner = (gDuels[slot][duPlayerA] == playerid)
            ? gDuels[slot][duPlayerB] : gDuels[slot][duPlayerA];

        Duel_End(slot, winner, "Gegner besiegt");
        return 1;
    }

    // Arena: einfach wieder einsetzen
    if (PlayerData[playerid][pArena] != -1)
    {
        if (killerid != INVALID_PLAYER_ID && PlayerData[killerid][pArena] == PlayerData[playerid][pArena])
        {
            PlayerData[killerid][pArenaKills]++;
        }
        return 1;
    }
    return 0;
}

/*
 * Nach dem Tod in einer Arena spawnt SA-MP den Spieler normal - hier wird er
 * stattdessen zurueck in die Arena gesetzt.
 */
stock Arena_OnPlayerSpawn(playerid)
{
    if (PlayerData[playerid][pArena] == -1) return 0;

    Arena_PlacePlayer(playerid, PlayerData[playerid][pArena]);
    return 1;
}

stock Arena_OnPlayerDisconnect(playerid)
{
    if (PlayerData[playerid][pDuelSlot] != -1)
    {
        new slot = PlayerData[playerid][pDuelSlot];
        new winner = (gDuels[slot][duPlayerA] == playerid)
            ? gDuels[slot][duPlayerB] : gDuels[slot][duPlayerA];

        PlayerData[playerid][pDuelSlot] = -1;
        Duel_End(slot, winner, "Gegner hat den Server verlassen");
        return 1;
    }
    if (PlayerData[playerid][pArena] != -1) Arena_Leave(playerid, false);

    // Offene Einladungen dieses Spielers zuruecknehmen
    foreach_player(i)
    {
        if (PlayerData[i][pDuelInviteFrom] == playerid)
        {
            PlayerData[i][pDuelInviteFrom] = INVALID_PLAYER_ID;
        }
    }
    return 0;
}

stock Arena_OnDialogResponse(playerid, dialogid, response, listitem)
{
    if (dialogid != DIALOG_DM_LIST) return 0;
    if (!response) return 1;
    if (listitem < 0 || listitem >= sizeof gArenas) return 1;

    if (PlayerData[playerid][pMinigame] != MINIGAME_NONE &&
        PlayerData[playerid][pMinigame] != MINIGAME_DM)
    {
        SendError(playerid, "Du bist bereits in einem anderen Minispiel.");
        return 1;
    }
    if (PlayerData[playerid][pArena] != -1) Arena_Leave(playerid, false);

    Arena_Join(playerid, listitem);
    return 1;
}

// ---------------------------------------------------------------------------
// Kommandos
// ---------------------------------------------------------------------------
CMD:dm(playerid, params[])
{
    #pragma unused params
    require(PlayerData[playerid][pMinigame] == MINIGAME_NONE || PlayerData[playerid][pArena] != -1, "Du bist bereits in einem anderen Minispiel.");

    return Arena_ShowList(playerid);
}
ALIAS:arena(dm)

CMD:duel(playerid, params[])
{
    require(Params_Count(params) >= 1, "Benutzung: /duel [Spieler] [Waffen-ID, optional]");
    require(PlayerData[playerid][pMinigame] == MINIGAME_NONE, "Du bist bereits in einem Minispiel.");

    new arg[MAX_PLAYER_NAME + 1];
    Params_Get(params, 0, arg, sizeof arg);

    new target = ResolveTarget(playerid, arg);
    if (target == INVALID_PLAYER_ID) return 1;

    require(target != playerid, "Gegen dich selbst zu duellieren waere kurz.");
    require(PlayerData[target][pMinigame] == MINIGAME_NONE, "Dieser Spieler ist gerade in einem Minispiel.");

    new weapon = (Params_Count(params) >= 2) ? Params_Int(params, 1) : 24;   // Standard: Deagle
    require(weapon >= 1 && weapon <= 46, "Ungueltige Waffen-ID (erlaubt: 1-46).");

    PlayerData[target][pDuelInviteFrom]   = playerid;
    PlayerData[target][pDuelInviteWeapon] = weapon;

    new msg[176];
    format(msg, sizeof msg, "%s fordert dich zum Duell (Waffe %d). Annehmen mit %s/accept",
        GetName(playerid), weapon, EC_WHITE);
    SendInfo(target, msg);

    format(msg, sizeof msg, "Duellanfrage an %s gesendet.", GetName(target));
    SendSuccess(playerid, msg);
    return 1;
}

/*
 * Verlaesst Arena oder Duell. Eigener Name, weil /leave in derby.inc liegt
 * und dort die spaeter eingebundenen Module noch nicht bekannt sind.
 */
CMD:leavedm(playerid, params[])
{
    #pragma unused params

    if (PlayerData[playerid][pDuelSlot] != -1)
    {
        new slot = PlayerData[playerid][pDuelSlot];
        new winner = (gDuels[slot][duPlayerA] == playerid)
            ? gDuels[slot][duPlayerB] : gDuels[slot][duPlayerA];

        PlayerData[playerid][pDuelSlot] = -1;
        Duel_End(slot, winner, "Gegner hat aufgegeben");
        return 1;
    }

    require(PlayerData[playerid][pArena] != -1, "Du bist in keiner Arena und in keinem Duell.");

    Arena_Leave(playerid);
    return 1;
}
ALIAS:leavearena(leavedm)

CMD:accept(playerid, params[])
{
    #pragma unused params

    new from = PlayerData[playerid][pDuelInviteFrom];
    require(from != INVALID_PLAYER_ID && IsPlayerConnected(from), "Du hast keine offene Duellanfrage.");
    require(PlayerData[playerid][pMinigame] == MINIGAME_NONE, "Du bist bereits in einem Minispiel.");
    require(PlayerData[from][pMinigame] == MINIGAME_NONE, "Dein Herausforderer ist inzwischen anderweitig beschaeftigt.");

    new weapon = PlayerData[playerid][pDuelInviteWeapon];
    PlayerData[playerid][pDuelInviteFrom] = INVALID_PLAYER_ID;

    Duel_Start(from, playerid, weapon);
    return 1;
}
