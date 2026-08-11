/*
 * Jebiga-Gaming - Rennen
 *
 * Immer hoechstens ein Rennen gleichzeitig. Ein Spieler eroeffnet mit /race
 * eine Lobby, andere steigen mit /join ein; nach dem Countdown zaehlen nur
 * noch die Checkpoints. Wer als Erster durchs Ziel faehrt, gewinnt.
 */

#if defined _jebiga_race_included
    #endinput
#endif
#define _jebiga_race_included

#define RACE_MAX_CHECKPOINTS    (10)
#define RACE_LOBBY_TIME         (30)     // Sekunden Wartezeit vor dem Start
#define RACE_COUNTDOWN          (5)      // Sekunden Startcountdown
#define RACE_TIME_LIMIT         (600)    // Sekunden, danach Abbruch
#define RACE_CHECKPOINT_SIZE    (12.0)
#define RACE_MIN_PLAYERS        (2)
#define RACE_PRIZE_PER_PLAYER   (5000)

#define RACE_IDLE               (0)
#define RACE_LOBBY              (1)
#define RACE_COUNTING           (2)
#define RACE_RUNNING            (3)

enum E_RACE
{
    raName[32],
    raVehicle,
    Float:raStartX,
    Float:raStartY,
    Float:raStartZ,
    Float:raStartAngle,
    raCheckpointCount,
    // Checkpoints flach als x,y,z hintereinander - Pawn-Enums koennen keine
    // mehrdimensionalen Felder.
    Float:raPoints[RACE_MAX_CHECKPOINTS * 3]
};

static const gRaces[][E_RACE] =
{
    {
        "Los Santos Stadtrundfahrt", 411, 1481.0000, -1770.0000, 18.7900, 90.0000, 6,
        {
            1380.0000, -1730.0000, 13.3000,
            1180.0000, -1330.0000, 13.5000,
             820.0000, -1080.0000, 24.3000,
            1160.0000,  -930.0000, 42.9000,
            1480.0000, -1180.0000, 23.8000,
            1481.0000, -1770.0000, 18.7900,
            0.0, 0.0, 0.0,
            0.0, 0.0, 0.0,
            0.0, 0.0, 0.0,
            0.0, 0.0, 0.0
        }
    },
    {
        "Wuestenrallye", 495, 400.0000, 2536.0000, 16.5000, 270.0000, 6,
        {
             160.0000,  2460.0000, 16.5000,
            -180.0000,  2300.0000, 30.0000,
            -400.0000,  2200.0000, 42.0000,
            -180.0000,  2000.0000, 60.0000,
             150.0000,  2100.0000, 20.0000,
             400.0000,  2536.0000, 16.5000,
            0.0, 0.0, 0.0,
            0.0, 0.0, 0.0,
            0.0, 0.0, 0.0,
            0.0, 0.0, 0.0
        }
    },
    {
        "Chiliad-Abfahrt", 468, -2340.0000, -1600.0000, 483.7000, 180.0000, 5,
        {
            -2280.0000, -1750.0000, 440.0000,
            -2200.0000, -1950.0000, 340.0000,
            -2130.0000, -2200.0000, 180.0000,
            -2100.0000, -2400.0000,  50.0000,
            -2160.0000, -2400.0000,  30.6200,
            0.0, 0.0, 0.0,
            0.0, 0.0, 0.0,
            0.0, 0.0, 0.0,
            0.0, 0.0, 0.0,
            0.0, 0.0, 0.0
        }
    },
    {
        "Las Venturas Strip", 429, 2036.0000, 1546.0000, 10.8200, 90.0000, 6,
        {
            2200.0000, 1600.0000, 10.8200,
            2380.0000, 1900.0000, 10.8200,
            2100.0000, 2200.0000, 10.8200,
            1700.0000, 2000.0000, 10.8200,
            1750.0000, 1600.0000, 10.8200,
            2036.0000, 1546.0000, 10.8200,
            0.0, 0.0, 0.0,
            0.0, 0.0, 0.0,
            0.0, 0.0, 0.0,
            0.0, 0.0, 0.0
        }
    }
};

static gRaceState   = RACE_IDLE;
static gRaceTrack   = -1;
static gRaceTimer   = 0;        // Restsekunden der aktuellen Phase
static gRaceElapsed = 0;        // Sekunden seit dem Start
static gRaceHost    = INVALID_PLAYER_ID;

stock Race_OnGameModeInit()
{
    gRaceState = RACE_IDLE;
    gRaceTrack = -1;

    new msg[64];
    format(msg, sizeof msg, "%d Rennstrecken geladen.", sizeof gRaces);
    Log(msg);
    return 1;
}

// ---------------------------------------------------------------------------
// Hilfsfunktionen
// ---------------------------------------------------------------------------
static Race_GetCheckpoint(track, index, &Float:x, &Float:y, &Float:z)
{
    new base = index * 3;
    x = gRaces[track][raPoints][base];
    y = gRaces[track][raPoints][base + 1];
    z = gRaces[track][raPoints][base + 2];
    return 1;
}

stock Race_CountParticipants()
{
    new count = 0;
    foreach_player(i)
    {
        if (PlayerData[i][pMinigame] == MINIGAME_RACE) count++;
    }
    return count;
}

static Race_SetNextCheckpoint(playerid)
{
    new index = PlayerData[playerid][pRaceCheckpoint];

    new Float:x, Float:y, Float:z;
    Race_GetCheckpoint(gRaceTrack, index, x, y, z);

    // Letzter Punkt = Zielcheckpoint (Typ 1), sonst normaler Durchfahrtspunkt
    new type = (index == gRaces[gRaceTrack][raCheckpointCount] - 1) ? 1 : 0;

    if (type == 0)
    {
        new Float:nx, Float:ny, Float:nz;
        Race_GetCheckpoint(gRaceTrack, index + 1, nx, ny, nz);
        SetPlayerRaceCheckpoint(playerid, 0, x, y, z, nx, ny, nz, RACE_CHECKPOINT_SIZE);
    }
    else
    {
        SetPlayerRaceCheckpoint(playerid, 1, x, y, z, 0.0, 0.0, 0.0, RACE_CHECKPOINT_SIZE);
    }
    return 1;
}

/*
 * Nimmt einen Spieler aus dem Rennen und raeumt sein Fahrzeug weg.
 */
stock Race_RemovePlayer(playerid, bool:teleportOut = true)
{
    if (PlayerData[playerid][pMinigame] != MINIGAME_RACE) return 0;

    PlayerData[playerid][pMinigame]       = MINIGAME_NONE;
    PlayerData[playerid][pRaceId]         = -1;
    PlayerData[playerid][pRaceCheckpoint] = 0;

    DisablePlayerRaceCheckpoint(playerid);

    if (PlayerData[playerid][pMinigameVehicle] != INVALID_VEHICLE_ID)
    {
        DestroyVehicle(PlayerData[playerid][pMinigameVehicle]);
        PlayerData[playerid][pMinigameVehicle] = INVALID_VEHICLE_ID;
    }

    if (teleportOut && IsPlayerConnected(playerid))
    {
        Teleport_SpawnPlayer(playerid);
        GivePlayerSpawnProtection(playerid);
    }
    return 1;
}

stock Race_End(const reason[])
{
    foreach_player(i)
    {
        if (PlayerData[i][pMinigame] == MINIGAME_RACE) Race_RemovePlayer(i);
    }

    gRaceState   = RACE_IDLE;
    gRaceTrack   = -1;
    gRaceTimer   = 0;
    gRaceElapsed = 0;
    gRaceHost    = INVALID_PLAYER_ID;

    if (reason[0] != EOS)
    {
        new msg[144];
        format(msg, sizeof msg, "Rennen beendet: %s", reason);
        SendAnnounce(msg);
    }
    return 1;
}

/*
 * Setzt einen Teilnehmer an die Startlinie. Die Startplaetze werden seitlich
 * versetzt, damit sich niemand ins Fahrzeug des anderen setzt.
 */
static Race_PlaceAtStart(playerid, slot)
{
    new Float:angle = gRaces[gRaceTrack][raStartAngle];
    new Float:offset = float(slot) * 4.0;

    // Seitlich zur Fahrtrichtung versetzen
    new Float:x = gRaces[gRaceTrack][raStartX] + offset * floatcos(-angle, degrees);
    new Float:y = gRaces[gRaceTrack][raStartY] - offset * floatsin(-angle, degrees);
    new Float:z = gRaces[gRaceTrack][raStartZ];

    if (PlayerData[playerid][pMinigameVehicle] != INVALID_VEHICLE_ID)
    {
        DestroyVehicle(PlayerData[playerid][pMinigameVehicle]);
    }

    new vehicleid = CreateVehicle(gRaces[gRaceTrack][raVehicle], x, y, z + 0.5, angle,
        random(256), random(256), -1);

    if (vehicleid == INVALID_VEHICLE_ID)
    {
        SendError(playerid, "Es konnte kein Rennfahrzeug erstellt werden.");
        Race_RemovePlayer(playerid);
        return 0;
    }

    PlayerData[playerid][pMinigameVehicle] = vehicleid;
    SetPlayerPos(playerid, x, y, z + 0.5);
    PutPlayerInVehicle(playerid, vehicleid, 0);
    TogglePlayerControllable(playerid, false);
    return 1;
}

stock Race_Start()
{
    new slot = 0;

    foreach_player(i)
    {
        if (PlayerData[i][pMinigame] != MINIGAME_RACE) continue;

        PlayerData[i][pRaceCheckpoint] = 0;
        PlayerData[i][pRaceStartTime]  = GetTickCount();
        Race_PlaceAtStart(i, slot++);
    }

    gRaceState = RACE_COUNTING;
    gRaceTimer = RACE_COUNTDOWN;
    return 1;
}

// ---------------------------------------------------------------------------
// Takt
// ---------------------------------------------------------------------------
stock Race_Tick()
{
    switch (gRaceState)
    {
        case RACE_LOBBY:
        {
            gRaceTimer--;

            if (gRaceTimer <= 0)
            {
                if (Race_CountParticipants() < RACE_MIN_PLAYERS)
                {
                    Race_End("zu wenige Teilnehmer.");
                    return 1;
                }
                Race_Start();
                return 1;
            }

            // In den letzten zehn Sekunden im Sekundentakt erinnern
            if (gRaceTimer <= 10 || gRaceTimer % 10 == 0)
            {
                new msg[128];
                format(msg, sizeof msg, "%sRennen '%s' startet in %d Sekunden - %s/join",
                    EC_CYAN, gRaces[gRaceTrack][raName], gRaceTimer, EC_WHITE);
                SendClientMessageToAll(COL_WHITE, msg);
            }
        }
        case RACE_COUNTING:
        {
            new text[32];

            if (gRaceTimer > 0) format(text, sizeof text, "~y~%d", gRaceTimer);
            else                format(text, sizeof text, "~g~LOS!");

            foreach_player(i)
            {
                if (PlayerData[i][pMinigame] == MINIGAME_RACE)
                {
                    GameTextForPlayer(i, text, 1000, 3);
                }
            }

            if (gRaceTimer <= 0)
            {
                gRaceState   = RACE_RUNNING;
                gRaceElapsed = 0;

                foreach_player(i)
                {
                    if (PlayerData[i][pMinigame] != MINIGAME_RACE) continue;

                    TogglePlayerControllable(i, true);
                    PlayerData[i][pRaceStartTime] = GetTickCount();
                    Race_SetNextCheckpoint(i);
                }
                return 1;
            }
            gRaceTimer--;
        }
        case RACE_RUNNING:
        {
            gRaceElapsed++;

            if (gRaceElapsed >= RACE_TIME_LIMIT)
            {
                Race_End("Zeitlimit erreicht.");
                return 1;
            }
            if (Race_CountParticipants() == 0)
            {
                Race_End("");
                return 1;
            }
        }
    }
    return 1;
}

// ---------------------------------------------------------------------------
// Callbacks
// ---------------------------------------------------------------------------
stock Race_OnEnterRaceCheckpoint(playerid)
{
    if (PlayerData[playerid][pMinigame] != MINIGAME_RACE) return 0;
    if (gRaceState != RACE_RUNNING) return 0;

    PlayerData[playerid][pRaceCheckpoint]++;

    // Ziel erreicht
    if (PlayerData[playerid][pRaceCheckpoint] >= gRaces[gRaceTrack][raCheckpointCount])
    {
        new seconds = (GetTickCount() - PlayerData[playerid][pRaceStartTime]) / 1000;
        new prize   = RACE_PRIZE_PER_PLAYER * Race_CountParticipants();

        PlayerData[playerid][pRaceWins]++;
        PlayerData[playerid][pScore] += 5;
        SetPlayerScore(playerid, PlayerData[playerid][pScore]);
        GivePlayerMoney(playerid, prize);

        new duration[32], money[24], msg[192];
        FormatDuration(seconds, duration, sizeof duration);
        FormatNumber(prize, money, sizeof money);

        format(msg, sizeof msg, "%s gewinnt '%s' in %s und erhaelt $%s!",
            GetName(playerid), gRaces[gRaceTrack][raName], duration, money);
        SendAnnounce(msg);

        Race_End("");
        return 1;
    }

    Race_SetNextCheckpoint(playerid);

    new msg[96];
    format(msg, sizeof msg, "Checkpoint %d von %d",
        PlayerData[playerid][pRaceCheckpoint], gRaces[gRaceTrack][raCheckpointCount]);
    GameTextForPlayer(playerid, msg, 1500, 6);
    return 1;
}

stock Race_OnPlayerDeath(playerid)
{
    if (PlayerData[playerid][pMinigame] != MINIGAME_RACE) return 0;

    SendError(playerid, "Du bist im Rennen gestorben und ausgeschieden.");
    Race_RemovePlayer(playerid, false);
    return 1;
}

stock Race_OnPlayerDisconnect(playerid)
{
    if (PlayerData[playerid][pMinigame] != MINIGAME_RACE) return 0;

    Race_RemovePlayer(playerid, false);
    if (gRaceHost == playerid) gRaceHost = INVALID_PLAYER_ID;
    return 1;
}

stock Race_ShowList(playerid)
{
    new list[512], entry[96];

    for (new i = 0; i < sizeof gRaces; i++)
    {
        format(entry, sizeof entry, "%s%s%s - %d Checkpoints, %s\n",
            EC_WHITE, gRaces[i][raName], EC_GREY,
            gRaces[i][raCheckpointCount], Vehicle_GetModelName(gRaces[i][raVehicle]));
        strcat(list, entry);
    }

    return ShowPlayerDialog(playerid, DIALOG_RACE_LIST, DIALOG_STYLE_LIST,
        "Rennen starten", list, "Eroeffnen", "Abbrechen");
}

stock Race_Open(playerid, track)
{
    if (gRaceState != RACE_IDLE)
    {
        SendError(playerid, "Es laeuft bereits ein Rennen. Mit /join steigst du ein.");
        return 0;
    }

    gRaceState = RACE_LOBBY;
    gRaceTrack = track;
    gRaceTimer = RACE_LOBBY_TIME;
    gRaceHost  = playerid;

    PlayerData[playerid][pMinigame]       = MINIGAME_RACE;
    PlayerData[playerid][pRaceId]         = track;
    PlayerData[playerid][pRaceCheckpoint] = 0;

    new msg[192];
    format(msg, sizeof msg, "%s hat das Rennen '%s' eroeffnet! Mit /join bist du dabei (%d Sekunden).",
        GetName(playerid), gRaces[track][raName], RACE_LOBBY_TIME);
    SendAnnounce(msg);
    return 1;
}

stock Race_OnDialogResponse(playerid, dialogid, response, listitem)
{
    if (dialogid != DIALOG_RACE_LIST) return 0;
    if (!response) return 1;
    if (listitem < 0 || listitem >= sizeof gRaces) return 1;

    Race_Open(playerid, listitem);
    return 1;
}

// ---------------------------------------------------------------------------
// Kommandos
// ---------------------------------------------------------------------------
CMD:race(playerid, params[])
{
    #pragma unused params
    require(PlayerData[playerid][pMinigame] == MINIGAME_NONE, "Du bist bereits in einem Minispiel.");

    if (gRaceState != RACE_IDLE)
    {
        new msg[144];
        format(msg, sizeof msg, "Es laeuft bereits das Rennen '%s'. Steig mit /join ein.",
            gRaces[gRaceTrack][raName]);
        SendInfo(playerid, msg);
        return 1;
    }
    return Race_ShowList(playerid);
}

CMD:join(playerid, params[])
{
    #pragma unused params
    require(PlayerData[playerid][pMinigame] == MINIGAME_NONE, "Du bist bereits in einem Minispiel.");
    require(gRaceState == RACE_LOBBY, "Gerade kann man keinem Rennen beitreten. /race eroeffnet eins.");

    PlayerData[playerid][pMinigame]       = MINIGAME_RACE;
    PlayerData[playerid][pRaceId]         = gRaceTrack;
    PlayerData[playerid][pRaceCheckpoint] = 0;

    new msg[144];
    format(msg, sizeof msg, "%s nimmt am Rennen teil (%d Teilnehmer).",
        GetName(playerid), Race_CountParticipants());
    SendClientMessageToAll(COL_CYAN, msg);

    format(msg, sizeof msg, "Du bist beim Rennen '%s' dabei. Start in %d Sekunden.",
        gRaces[gRaceTrack][raName], gRaceTimer);
    SendSuccess(playerid, msg);
    return 1;
}
