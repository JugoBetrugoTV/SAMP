/*
 * UIF Freeroam - Derby
 *
 * Letzter Fahrer gewinnt. Die Teilnehmer werden im Kreis auf dem verlassenen
 * Flugfeld abgesetzt und in einer eigenen Virtual World gehalten, damit das
 * restliche Freeroam nicht dazwischenfunkt.
 */

#if defined _uif_derby_included
    #endinput
#endif
#define _uif_derby_included

#define DERBY_LOBBY_TIME        (40)
#define DERBY_COUNTDOWN         (5)
#define DERBY_TIME_LIMIT        (420)
#define DERBY_MIN_PLAYERS       (2)
#define DERBY_WORLD             (500)
#define DERBY_RADIUS            (35.0)
#define DERBY_PRIZE_PER_PLAYER  (7500)

#define DERBY_IDLE              (0)
#define DERBY_LOBBY             (1)
#define DERBY_COUNTING          (2)
#define DERBY_RUNNING           (3)

enum E_DERBY_ARENA
{
    daName[40],
    daVehicle,
    Float:daX,
    Float:daY,
    Float:daZ
};

static const gDerbyArenas[][E_DERBY_ARENA] =
{
    {"Verlassener Flughafen", 495,   400.0000, 2536.0000,  16.5000},
    {"Area 51 Rollfeld",      495,   135.0000, 1943.0000,  19.9000},
    {"Steinbruch",            568,   595.0000,  852.0000, -42.5000},
    {"LS Stadion",            504,  2769.0000, -1618.0000, 12.1000}
};

static gDerbyState = DERBY_IDLE;
static gDerbyArena = -1;
static gDerbyTimer = 0;
static gDerbyElapsed = 0;
static bool:gDerbyAlive[MAX_PLAYERS];

stock Derby_OnGameModeInit()
{
    gDerbyState = DERBY_IDLE;
    gDerbyArena = -1;

    new msg[64];
    format(msg, sizeof msg, "%d Derby-Arenen geladen.", sizeof gDerbyArenas);
    Log(msg);
    return 1;
}

stock bool:Derby_IsParticipant(playerid)
{
    return (PlayerData[playerid][pMinigame] == MINIGAME_DERBY);
}

stock Derby_CountParticipants()
{
    new count = 0;
    foreach_player(i)
    {
        if (Derby_IsParticipant(i)) count++;
    }
    return count;
}

stock Derby_CountAlive()
{
    new count = 0;
    foreach_player(i)
    {
        if (Derby_IsParticipant(i) && gDerbyAlive[i]) count++;
    }
    return count;
}

stock Derby_RemovePlayer(playerid, bool:teleportOut = true)
{
    if (!Derby_IsParticipant(playerid)) return 0;

    PlayerData[playerid][pMinigame] = MINIGAME_NONE;
    gDerbyAlive[playerid] = false;

    if (PlayerData[playerid][pMinigameVehicle] != INVALID_VEHICLE_ID)
    {
        DestroyVehicle(PlayerData[playerid][pMinigameVehicle]);
        PlayerData[playerid][pMinigameVehicle] = INVALID_VEHICLE_ID;
    }

    if (IsPlayerConnected(playerid))
    {
        SetPlayerVirtualWorld(playerid, 0);
        SetPlayerInterior(playerid, 0);

        if (teleportOut)
        {
            Teleport_SpawnPlayer(playerid);
            GivePlayerSpawnProtection(playerid);
        }
    }
    return 1;
}

stock Derby_End(const reason[])
{
    foreach_player(i)
    {
        if (Derby_IsParticipant(i)) Derby_RemovePlayer(i);
    }

    gDerbyState   = DERBY_IDLE;
    gDerbyArena   = -1;
    gDerbyTimer   = 0;
    gDerbyElapsed = 0;

    if (reason[0] != EOS)
    {
        new msg[144];
        format(msg, sizeof msg, "Derby beendet: %s", reason);
        SendAnnounce(msg);
    }
    return 1;
}

/*
 * Setzt einen Teilnehmer auf seinen Platz im Kreis der Arena.
 */
static Derby_Place(playerid, slot, total)
{
    if (total < 1) total = 1;

    new Float:angle = (360.0 / float(total)) * float(slot);
    new Float:x = gDerbyArenas[gDerbyArena][daX] + DERBY_RADIUS * floatsin(-angle, degrees);
    new Float:y = gDerbyArenas[gDerbyArena][daY] + DERBY_RADIUS * floatcos(-angle, degrees);
    new Float:z = gDerbyArenas[gDerbyArena][daZ];

    if (PlayerData[playerid][pMinigameVehicle] != INVALID_VEHICLE_ID)
    {
        DestroyVehicle(PlayerData[playerid][pMinigameVehicle]);
    }

    // Blick zur Arenamitte
    new Float:facing = angle + 180.0;
    if (facing >= 360.0) facing -= 360.0;

    new vehicleid = CreateVehicle(gDerbyArenas[gDerbyArena][daVehicle], x, y, z + 0.5, facing,
        random(256), random(256), -1);

    if (vehicleid == INVALID_VEHICLE_ID)
    {
        SendError(playerid, "Es konnte kein Derbyfahrzeug erstellt werden.");
        Derby_RemovePlayer(playerid);
        return 0;
    }

    PlayerData[playerid][pMinigameVehicle] = vehicleid;
    SetVehicleVirtualWorld(vehicleid, DERBY_WORLD);

    SetPlayerVirtualWorld(playerid, DERBY_WORLD);
    SetPlayerPos(playerid, x, y, z + 0.5);
    PutPlayerInVehicle(playerid, vehicleid, 0);
    TogglePlayerControllable(playerid, false);
    ResetPlayerWeapons(playerid);

    gDerbyAlive[playerid] = true;
    return 1;
}

/*
 * Nach einem Tod im Derby wird der Spieler nicht neu eingesetzt, sondern als
 * Zuschauer ueber der Arena gehalten.
 */
stock Derby_RespawnParticipant(playerid)
{
    SetPlayerVirtualWorld(playerid, DERBY_WORLD);
    SetPlayerPos(playerid,
        gDerbyArenas[gDerbyArena][daX],
        gDerbyArenas[gDerbyArena][daY],
        gDerbyArenas[gDerbyArena][daZ] + 25.0);
    SetPlayerHealth(playerid, 100.0);
    SendInfo(playerid, "Du bist ausgeschieden und siehst dem Rest zu. /leave verlaesst das Derby.");
    return 1;
}

static Derby_Eliminate(playerid, const cause[])
{
    if (!Derby_IsParticipant(playerid) || !gDerbyAlive[playerid]) return 0;

    gDerbyAlive[playerid] = false;

    new msg[144];
    format(msg, sizeof msg, "%s ist ausgeschieden (%s). Noch %d im Rennen.",
        GetName(playerid), cause, Derby_CountAlive());
    SendClientMessageToAll(COL_ORANGE, msg);

    if (PlayerData[playerid][pMinigameVehicle] != INVALID_VEHICLE_ID)
    {
        DestroyVehicle(PlayerData[playerid][pMinigameVehicle]);
        PlayerData[playerid][pMinigameVehicle] = INVALID_VEHICLE_ID;
    }
    Derby_RespawnParticipant(playerid);
    return 1;
}

static Derby_CheckForWinner()
{
    if (gDerbyState != DERBY_RUNNING) return 0;
    if (Derby_CountAlive() > 1) return 0;

    new winner = INVALID_PLAYER_ID;
    foreach_player(i)
    {
        if (Derby_IsParticipant(i) && gDerbyAlive[i]) { winner = i; break; }
    }

    if (winner == INVALID_PLAYER_ID)
    {
        Derby_End("kein Sieger uebrig.");
        return 1;
    }

    new prize = DERBY_PRIZE_PER_PLAYER * Derby_CountParticipants();

    PlayerData[winner][pDerbyWins]++;
    PlayerData[winner][pScore] += 5;
    SetPlayerScore(winner, PlayerData[winner][pScore]);
    GivePlayerMoney(winner, prize);

    new money[24], msg[192];
    FormatNumber(prize, money, sizeof money);
    format(msg, sizeof msg, "%s gewinnt das Derby in der Arena '%s' und erhaelt $%s!",
        GetName(winner), gDerbyArenas[gDerbyArena][daName], money);
    SendAnnounce(msg);

    Derby_End("");
    return 1;
}

stock Derby_Start()
{
    new total = Derby_CountParticipants(), slot = 0;

    foreach_player(i)
    {
        if (!Derby_IsParticipant(i)) continue;
        Derby_Place(i, slot++, total);
    }

    gDerbyState = DERBY_COUNTING;
    gDerbyTimer = DERBY_COUNTDOWN;
    return 1;
}

// ---------------------------------------------------------------------------
// Takt
// ---------------------------------------------------------------------------
stock Derby_Tick()
{
    switch (gDerbyState)
    {
        case DERBY_LOBBY:
        {
            gDerbyTimer--;

            if (gDerbyTimer <= 0)
            {
                if (Derby_CountParticipants() < DERBY_MIN_PLAYERS)
                {
                    Derby_End("zu wenige Teilnehmer.");
                    return 1;
                }
                Derby_Start();
                return 1;
            }

            if (gDerbyTimer <= 10 || gDerbyTimer % 10 == 0)
            {
                new msg[144];
                format(msg, sizeof msg, "%sDerby '%s' startet in %d Sekunden - %s/derby",
                    EC_ORANGE, gDerbyArenas[gDerbyArena][daName], gDerbyTimer, EC_WHITE);
                SendClientMessageToAll(COL_WHITE, msg);
            }
        }
        case DERBY_COUNTING:
        {
            new text[32];
            if (gDerbyTimer > 0) format(text, sizeof text, "~y~%d", gDerbyTimer);
            else                 format(text, sizeof text, "~r~ZERSTOEREN!");

            foreach_player(i)
            {
                if (Derby_IsParticipant(i)) GameTextForPlayer(i, text, 1000, 3);
            }

            if (gDerbyTimer <= 0)
            {
                gDerbyState   = DERBY_RUNNING;
                gDerbyElapsed = 0;

                foreach_player(i)
                {
                    if (Derby_IsParticipant(i)) TogglePlayerControllable(i, true);
                }
                return 1;
            }
            gDerbyTimer--;
        }
        case DERBY_RUNNING:
        {
            gDerbyElapsed++;

            if (gDerbyElapsed >= DERBY_TIME_LIMIT)
            {
                Derby_End("Zeitlimit erreicht.");
                return 1;
            }

            // Ausgestiegene Fahrer scheiden aus - zu Fuss gibt es kein Derby
            foreach_player(i)
            {
                if (!Derby_IsParticipant(i) || !gDerbyAlive[i]) continue;

                if (GetPlayerState(i) != PLAYER_STATE_DRIVER)
                {
                    Derby_Eliminate(i, "Fahrzeug verlassen");
                }
            }
            Derby_CheckForWinner();
        }
    }
    return 1;
}

// ---------------------------------------------------------------------------
// Callbacks
// ---------------------------------------------------------------------------
stock Derby_OnVehicleDeath(vehicleid)
{
    if (gDerbyState != DERBY_RUNNING) return 0;

    foreach_player(i)
    {
        if (!Derby_IsParticipant(i) || !gDerbyAlive[i]) continue;
        if (PlayerData[i][pMinigameVehicle] != vehicleid) continue;

        PlayerData[i][pMinigameVehicle] = INVALID_VEHICLE_ID;
        Derby_Eliminate(i, "Fahrzeug zerstoert");
        Derby_CheckForWinner();
        return 1;
    }
    return 0;
}

stock Derby_OnPlayerDeath(playerid, killerid)
{
    #pragma unused killerid

    if (!Derby_IsParticipant(playerid)) return 0;

    Derby_Eliminate(playerid, "gestorben");
    Derby_CheckForWinner();
    return 1;
}

stock Derby_OnPlayerDisconnect(playerid)
{
    if (!Derby_IsParticipant(playerid)) return 0;

    Derby_RemovePlayer(playerid, false);
    Derby_CheckForWinner();
    return 1;
}

stock Derby_ShowList(playerid)
{
    new list[512], entry[96];

    for (new i = 0; i < sizeof gDerbyArenas; i++)
    {
        format(entry, sizeof entry, "%s%s%s - %s\n",
            EC_WHITE, gDerbyArenas[i][daName], EC_GREY,
            Vehicle_GetModelName(gDerbyArenas[i][daVehicle]));
        strcat(list, entry);
    }

    return ShowPlayerDialog(playerid, DIALOG_DERBY_LIST, DIALOG_STYLE_LIST,
        "Derby starten", list, "Eroeffnen", "Abbrechen");
}

stock Derby_Open(playerid, arena)
{
    if (gDerbyState != DERBY_IDLE)
    {
        SendError(playerid, "Es laeuft bereits ein Derby.");
        return 0;
    }

    gDerbyState = DERBY_LOBBY;
    gDerbyArena = arena;
    gDerbyTimer = DERBY_LOBBY_TIME;

    PlayerData[playerid][pMinigame] = MINIGAME_DERBY;
    gDerbyAlive[playerid] = false;

    new msg[192];
    format(msg, sizeof msg, "%s hat ein Derby in '%s' eroeffnet! Mit /derby bist du dabei (%d Sekunden).",
        GetName(playerid), gDerbyArenas[arena][daName], DERBY_LOBBY_TIME);
    SendAnnounce(msg);
    return 1;
}

stock Derby_OnDialogResponse(playerid, dialogid, response, listitem)
{
    if (dialogid != DIALOG_DERBY_LIST) return 0;
    if (!response) return 1;
    if (listitem < 0 || listitem >= sizeof gDerbyArenas) return 1;

    Derby_Open(playerid, listitem);
    return 1;
}

// ---------------------------------------------------------------------------
// Kommandos
// ---------------------------------------------------------------------------
CMD:derby(playerid, params[])
{
    #pragma unused params

    // Laeuft schon eine Lobby, tritt man mit demselben Kommando bei
    if (gDerbyState == DERBY_LOBBY)
    {
        require(PlayerData[playerid][pMinigame] == MINIGAME_NONE, "Du bist bereits in einem Minispiel.");

        PlayerData[playerid][pMinigame] = MINIGAME_DERBY;
        gDerbyAlive[playerid] = false;

        new msg[144];
        format(msg, sizeof msg, "%s nimmt am Derby teil (%d Teilnehmer).",
            GetName(playerid), Derby_CountParticipants());
        SendClientMessageToAll(COL_ORANGE, msg);

        format(msg, sizeof msg, "Du bist beim Derby dabei. Start in %d Sekunden.", gDerbyTimer);
        SendSuccess(playerid, msg);
        return 1;
    }

    require(gDerbyState == DERBY_IDLE, "Es laeuft bereits ein Derby.");
    require(PlayerData[playerid][pMinigame] == MINIGAME_NONE, "Du bist bereits in einem Minispiel.");

    return Derby_ShowList(playerid);
}

/*
 * Verlaesst das laufende Minispiel - egal ob Rennen oder Derby. Steht hier,
 * weil an dieser Stelle beide Module bekannt sind.
 */
CMD:leave(playerid, params[])
{
    #pragma unused params
    require(PlayerData[playerid][pMinigame] != MINIGAME_NONE, "Du bist in keinem Minispiel.");

    if (PlayerData[playerid][pMinigame] == MINIGAME_RACE)
    {
        Race_RemovePlayer(playerid);
        SendInfo(playerid, "Du hast das Rennen verlassen.");

        if (Race_CountParticipants() == 0) Race_End("");
        return 1;
    }

    Derby_RemovePlayer(playerid);
    SendInfo(playerid, "Du hast das Derby verlassen.");
    Derby_CheckForWinner();
    return 1;
}
