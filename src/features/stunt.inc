/*
 * UIF Freeroam - Stunts
 *
 * Bewertet Spruenge mit dem Fahrzeug und vergibt Stuntpunkte. SA-MP meldet
 * nicht, ob ein Fahrzeug den Boden beruehrt, deshalb wird die Flugphase aus
 * der vertikalen Geschwindigkeit abgeleitet: ein deutlicher Aufwaertsimpuls
 * startet die Messung, ein abrupter Stillstand der Vertikalbewegung beendet
 * sie. Das ist eine Naeherung, aber fuer eine Punktewertung stabil genug.
 */

#if defined _uif_stunt_included
    #endinput
#endif
#define _uif_stunt_included

#define STUNT_LIFTOFF_VELOCITY  (0.12)   // Vertikalgeschwindigkeit fuer Absprung
#define STUNT_LANDING_VELOCITY  (0.02)   // darunter gilt der Sprung als beendet
#define STUNT_MIN_AIRTIME       (900)    // ms, kuerzere Spruenge zaehlen nicht
#define STUNT_MAX_AIRTIME       (30000)  // ms, danach Messung verwerfen (Flugzeug)
#define STUNT_RAMP_OBJECT       (1655)   // Sprungrampe

// Stuntziele fuer das /stunt-Menue
enum E_STUNT_ZONE
{
    szName[40],
    Float:szX,
    Float:szY,
    Float:szZ,
    Float:szAngle
};

static const gStuntZones[][E_STUNT_ZONE] =
{
    {"Mount Chiliad - Gipfelstart",  -2340.0000, -1600.0000, 483.7000, 180.0000},
    {"Chiliad - Steilhang",          -2263.0000, -1740.0000, 480.3000, 220.0000},
    {"Sherman-Staudamm",              -969.0000,  1966.0000,  60.4000,  90.0000},
    {"Steinbruch",                     595.0000,   852.0000, -42.5000,   0.0000},
    {"Verlassener Flughafen",          400.0000,  2536.0000,  16.5000, 270.0000},
    {"LS - Hochhausdach",             1544.0000, -1353.0000, 329.4700,  90.0000},
    {"LV - Pyramidenspitze",          2001.0000,  1544.0000,  50.6000,   0.0000},
    {"Big Ear Radioteleskop",         -394.0000,  1493.0000,  65.6000, 180.0000},
    {"Staudamm-Krone Nord",           -1000.0000, 2000.0000,  60.0000,   0.0000}
};

stock Stunt_OnGameModeInit()
{
    new msg[64];
    format(msg, sizeof msg, "%d Stuntzonen geladen.", sizeof gStuntZones);
    Log(msg);
    return 1;
}

/*
 * Vergibt Punkte anhand von Flugdauer und zurueckgelegter Strecke.
 */
static Stunt_Award(playerid, airtime, Float:distance, Float:height)
{
    new points = (airtime / 250) + floatround(distance / 4.0) + floatround(height / 2.0);
    if (points < 1) return 0;

    // VIPs bekommen einen Bonus von 25 Prozent
    if (IsVip(playerid)) points = (points * 125) / 100;

    PlayerData[playerid][pStuntPoints] += points;
    GivePlayerMoney(playerid, points * 10);

    new msg[144];
    format(msg, sizeof msg, "~y~Stunt!~n~~w~%d Punkte~n~~b~%.0fm / %.1fs",
        points, distance, float(airtime) / 1000.0);
    GameTextForPlayer(playerid, msg, 3000, 4);

    format(msg, sizeof msg, "Stunt gelandet: %s%d Punkte%s (Strecke %.0fm, Hoehe %.0fm, %.1fs Flugzeit)",
        EC_WHITE, points, EC_CYAN, distance, height, float(airtime) / 1000.0);
    SendInfo(playerid, msg);
    return points;
}

stock Stunt_OnPlayerUpdate(playerid)
{
    if (!IsPlayerInAnyVehicleAsDriver(playerid))
    {
        PlayerData[playerid][pStuntInAir] = false;
        return 0;
    }

    new vehicleid = GetPlayerVehicleID(playerid);

    // Flugzeuge und Helikopter fliegen ohnehin - keine Stuntwertung
    switch (GetVehicleModel(vehicleid))
    {
        case 417, 425, 447, 460, 469, 476, 487, 488, 497, 511, 512, 513,
             519, 520, 548, 553, 563, 577, 592, 593:
        {
            PlayerData[playerid][pStuntInAir] = false;
            return 0;
        }
    }

    new Float:vx, Float:vy, Float:vz;
    GetVehicleVelocity(vehicleid, vx, vy, vz);

    new Float:x, Float:y, Float:z;
    GetVehiclePos(vehicleid, x, y, z);

    if (!PlayerData[playerid][pStuntInAir])
    {
        if (vz >= STUNT_LIFTOFF_VELOCITY)
        {
            PlayerData[playerid][pStuntInAir]      = true;
            PlayerData[playerid][pStuntStartTick]  = GetTickCount();
            PlayerData[playerid][pStuntStartPos][0] = x;
            PlayerData[playerid][pStuntStartPos][1] = y;
            PlayerData[playerid][pStuntStartPos][2] = z;
        }
        return 0;
    }

    new airtime = GetTickCount() - PlayerData[playerid][pStuntStartTick];

    if (airtime > STUNT_MAX_AIRTIME)
    {
        PlayerData[playerid][pStuntInAir] = false;
        return 0;
    }

    // Vertikalbewegung fast zum Stillstand gekommen -> gelandet
    if (airtime >= 300 && vz > -STUNT_LANDING_VELOCITY && vz < STUNT_LANDING_VELOCITY)
    {
        PlayerData[playerid][pStuntInAir] = false;

        if (airtime < STUNT_MIN_AIRTIME) return 0;

        new Float:dx = x - PlayerData[playerid][pStuntStartPos][0];
        new Float:dy = y - PlayerData[playerid][pStuntStartPos][1];
        new Float:distance = VectorSize(dx, dy, 0.0);
        new Float:height   = z - PlayerData[playerid][pStuntStartPos][2];
        if (height < 0.0) height = 0.0;

        Stunt_Award(playerid, airtime, distance, height);
    }
    return 1;
}

stock Stunt_OnPlayerDisconnect(playerid)
{
    if (PlayerData[playerid][pStuntObject] != INVALID_OBJECT_ID)
    {
        DestroyObject(PlayerData[playerid][pStuntObject]);
        PlayerData[playerid][pStuntObject] = INVALID_OBJECT_ID;
    }
    return 1;
}

stock Stunt_ShowZones(playerid)
{
    new list[768], entry[64];

    for (new i = 0; i < sizeof gStuntZones; i++)
    {
        format(entry, sizeof entry, "%s%s\n", EC_WHITE, gStuntZones[i][szName]);
        strcat(list, entry);
    }

    return ShowPlayerDialog(playerid, DIALOG_STUNT_LIST, DIALOG_STYLE_LIST,
        "Stuntzonen", list, "Hin", "Abbrechen");
}

stock Stunt_OnDialogResponse(playerid, dialogid, response, listitem)
{
    if (dialogid != DIALOG_STUNT_LIST) return 0;
    if (!response) return 1;
    if (listitem < 0 || listitem >= sizeof gStuntZones) return 1;
    if (!Teleport_CanUse(playerid)) return 1;

    Teleport_Player(playerid,
        gStuntZones[listitem][szX], gStuntZones[listitem][szY],
        gStuntZones[listitem][szZ], gStuntZones[listitem][szAngle]);

    new msg[96];
    format(msg, sizeof msg, "Stuntzone: %s%s", EC_WHITE, gStuntZones[listitem][szName]);
    SendSuccess(playerid, msg);
    return 1;
}

// ---------------------------------------------------------------------------
// Kommandos
// ---------------------------------------------------------------------------
CMD:stunt(playerid, params[])
{
    #pragma unused params
    return Stunt_ShowZones(playerid);
}

/*
 * Setzt eine Sprungrampe vor den Spieler. Jeder Spieler hat hoechstens eine;
 * eine neue ersetzt die alte.
 */
CMD:ramp(playerid, params[])
{
    #pragma unused params

    new Float:x, Float:y, Float:z, Float:angle;
    GetPlayerPos(playerid, x, y, z);
    GetPlayerFacingAngle(playerid, angle);

    if (IsPlayerInAnyVehicle(playerid))
    {
        GetVehiclePos(GetPlayerVehicleID(playerid), x, y, z);
        GetVehicleZAngle(GetPlayerVehicleID(playerid), angle);
    }

    x += 12.0 * floatsin(-angle, degrees);
    y += 12.0 * floatcos(-angle, degrees);

    if (PlayerData[playerid][pStuntObject] != INVALID_OBJECT_ID)
    {
        DestroyObject(PlayerData[playerid][pStuntObject]);
    }

    PlayerData[playerid][pStuntObject] =
        CreateObject(STUNT_RAMP_OBJECT, x, y, z - 0.8, 0.0, 0.0, angle);

    SendSuccess(playerid, "Rampe gesetzt. " EC_WHITE "/delramp" EC_GREEN " entfernt sie wieder.");
    return 1;
}

CMD:delramp(playerid, params[])
{
    #pragma unused params
    require(PlayerData[playerid][pStuntObject] != INVALID_OBJECT_ID, "Du hast keine Rampe gesetzt.");

    DestroyObject(PlayerData[playerid][pStuntObject]);
    PlayerData[playerid][pStuntObject] = INVALID_OBJECT_ID;
    SendSuccess(playerid, "Rampe entfernt.");
    return 1;
}
