/*
 * UIF Freeroam - Teleportsystem
 *
 * Kern eines Freeroam-Servers: schnelle Wege zu allen interessanten Orten der
 * Karte. Die Ziele sind in Kategorien gruppiert und teilweise VIPs vorbehalten.
 */

#if defined _uif_teleport_included
    #endinput
#endif
#define _uif_teleport_included

#define TP_CAT_CITY             (0)
#define TP_CAT_STUNT            (1)
#define TP_CAT_FUN              (2)
#define TP_CAT_MILITARY         (3)
#define TP_CAT_VIP              (4)
#define TP_CATEGORY_COUNT       (5)

enum E_TELEPORT
{
    tName[32],
    Float:tX,
    Float:tY,
    Float:tZ,
    Float:tAngle,
    tInterior,
    tCategory,
    tMinVip
};

static const gTeleportCategories[TP_CATEGORY_COUNT][] =
{
    "Staedte & Orte",
    "Stuntzonen",
    "Fun & Party",
    "Militaer & Sperrgebiet",
    "VIP-Bereiche"
};

static const gTeleports[][E_TELEPORT] =
{
    // --- Staedte & Orte ---
    {"Los Santos - Grove Street",   2495.0000, -1687.0000,   13.5100,   0.0000, 0, TP_CAT_CITY,     VIP_NONE},
    {"Los Santos - Innenstadt",     1481.0000, -1770.0000,   18.7900,  90.0000, 0, TP_CAT_CITY,     VIP_NONE},
    {"Los Santos - Strand",          364.0000, -1877.0000,    4.5600, 180.0000, 0, TP_CAT_CITY,     VIP_NONE},
    {"Los Santos - Flughafen",      1685.0000, -2334.0000,   13.5500,   0.0000, 0, TP_CAT_CITY,     VIP_NONE},
    {"Los Santos - Stadion",        2769.0000, -1618.0000,   12.1200,  90.0000, 0, TP_CAT_CITY,     VIP_NONE},
    {"Los Santos - Glen Park",      1969.0000, -1092.0000,   25.9000, 270.0000, 0, TP_CAT_CITY,     VIP_NONE},
    {"San Fierro - Zentrum",       -1980.0000,   137.0000,   27.6900,  90.0000, 0, TP_CAT_CITY,     VIP_NONE},
    {"San Fierro - Flughafen",     -1400.0000,  -300.0000,   14.1500,   0.0000, 0, TP_CAT_CITY,     VIP_NONE},
    {"San Fierro - Bruecke",       -2707.0000,  1439.0000,    7.1000, 180.0000, 0, TP_CAT_CITY,     VIP_NONE},
    {"San Fierro - Werft",         -2438.0000,  2287.0000,    4.9800,   0.0000, 0, TP_CAT_CITY,     VIP_NONE},
    {"Las Venturas - Strip",        2036.0000,  1546.0000,   10.8200,  90.0000, 0, TP_CAT_CITY,     VIP_NONE},
    {"Las Venturas - Flughafen",    1685.0000,  1447.0000,   10.7700,   0.0000, 0, TP_CAT_CITY,     VIP_NONE},
    {"Las Venturas - Caligula's",   2189.0000,  1677.0000,   12.3700, 180.0000, 0, TP_CAT_CITY,     VIP_NONE},
    {"Bayside",                    -2394.0000,  2321.0000,    4.9800,  90.0000, 0, TP_CAT_CITY,     VIP_NONE},
    {"Angel Pine",                 -2160.0000, -2400.0000,   30.6200,   0.0000, 0, TP_CAT_CITY,     VIP_NONE},
    {"Palomino Creek",              2255.0000,   65.0000,   26.4800, 270.0000, 0, TP_CAT_CITY,     VIP_NONE},
    {"Fort Carson",                -215.0000,   980.0000,   19.3000,  90.0000, 0, TP_CAT_CITY,     VIP_NONE},
    {"El Quebrados",               -1471.0000,  2620.0000,   55.8300,   0.0000, 0, TP_CAT_CITY,     VIP_NONE},

    // --- Stuntzonen ---
    {"Mount Chiliad - Gipfel",     -2340.0000, -1600.0000,  483.7000, 180.0000, 0, TP_CAT_STUNT,    VIP_NONE},
    {"Chiliad - Abfahrtsrampe",    -2263.0000, -1740.0000,  480.3000, 220.0000, 0, TP_CAT_STUNT,    VIP_NONE},
    {"Steinbruch",                   595.0000,   852.0000,  -42.5000,   0.0000, 0, TP_CAT_STUNT,    VIP_NONE},
    {"Sherman-Staudamm",            -969.0000,  1966.0000,   60.4000,  90.0000, 0, TP_CAT_STUNT,    VIP_NONE},
    {"Verlassener Flughafen",        400.0000,  2536.0000,   16.5000, 270.0000, 0, TP_CAT_STUNT,    VIP_NONE},
    {"Big Ear Radioteleskop",       -394.0000,  1493.0000,   65.6000, 180.0000, 0, TP_CAT_STUNT,    VIP_NONE},
    {"LS - Hochhausdach",           1544.0000, -1353.0000,  329.4700,  90.0000, 0, TP_CAT_STUNT,    VIP_NONE},
    {"LV - Pyramidenspitze",        2001.0000,  1544.0000,   50.6000,   0.0000, 0, TP_CAT_STUNT,    VIP_NONE},
    {"Bergstrasse Norden",         -1600.0000,  1200.0000,   14.0000,   0.0000, 0, TP_CAT_STUNT,    VIP_NONE},

    {"UIF Stuntpark (eigene Map)",  -950.0000,  1930.0000,  601.0000,   0.0000, 0, TP_CAT_STUNT,    VIP_NONE},

    // --- Fun & Party ---
    {"ZomboTech Labor",             1256.0000, -1400.0000,   13.5500,   0.0000, 0, TP_CAT_FUN,      VIP_NONE},
    {"Vergnuegungspark LS",         1961.0000, -1450.0000,   13.5500,  90.0000, 0, TP_CAT_FUN,      VIP_NONE},
    {"Riesenrad Santa Maria",        368.0000, -2032.0000,    7.8300, 180.0000, 0, TP_CAT_FUN,      VIP_NONE},
    {"Partyinsel",                  -1300.0000,  -300.0000,   14.1500,   0.0000, 0, TP_CAT_FUN,     VIP_NONE},
    {"Jizzy's Club",               -2624.0000,  1412.0000,    7.0900, 270.0000, 0, TP_CAT_FUN,      VIP_NONE},
    {"Bootshafen LS",                -83.0000, -1594.0000,    2.4500,   0.0000, 0, TP_CAT_FUN,      VIP_NONE},

    // --- Militaer & Sperrgebiet ---
    {"Area 51",                      135.0000,  1943.0000,   19.9000,   0.0000, 0, TP_CAT_MILITARY, VIP_NONE},
    {"Area 51 - Hangar",             213.0000,  1909.0000,   17.6400,  90.0000, 0, TP_CAT_MILITARY, VIP_NONE},
    {"Easter Basin Marinestuetzpunkt", -1671.0000, 393.0000,  7.1800, 180.0000, 0, TP_CAT_MILITARY, VIP_NONE},
    {"Verona Beach Militaerhafen",   -1655.0000,  -200.0000,  14.1500,   0.0000, 0, TP_CAT_MILITARY, VIP_NONE},

    // --- VIP-Bereiche ---
    {"VIP-Lounge",                   2000.0000,  1500.0000,  501.0000,   0.0000, 0, TP_CAT_VIP,     VIP_BRONZE},
    {"Gold-Insel",                   -1000.0000, -3000.0000,  11.0000,   0.0000, 0, TP_CAT_VIP,     VIP_GOLD}
};

// Spawnpunkte, wenn ein Spieler regulaer spawnt
static const Float:gSpawnPoints[][4] =
{
    {1958.3783, 1343.1572,  15.3746, 270.1425},   // Las Venturas
    {2495.0000, -1687.0000, 13.5100,   0.0000},   // Grove Street
    {-1980.0000,  137.0000, 27.6900,  90.0000},   // San Fierro
    {1481.0000, -1770.0000, 18.7900,  90.0000},   // LS Innenstadt
    {2036.0000,  1546.0000, 10.8200,  90.0000}    // LV Strip
};

stock Teleport_OnGameModeInit()
{
    new msg[64];
    format(msg, sizeof msg, "%d Teleportziele geladen.", sizeof gTeleports);
    Log(msg);
    return 1;
}

/*
 * Setzt einen Spieler auf einen zufaelligen Spawnpunkt.
 */
stock Teleport_SpawnPlayer(playerid)
{
    new idx = random(sizeof gSpawnPoints);

    SetPlayerPos(playerid, gSpawnPoints[idx][0], gSpawnPoints[idx][1], gSpawnPoints[idx][2]);
    SetPlayerFacingAngle(playerid, gSpawnPoints[idx][3]);
    SetPlayerInterior(playerid, 0);
    SetPlayerVirtualWorld(playerid, 0);
    SetCameraBehindPlayer(playerid);
    return 1;
}

/*
 * Teleportiert einen Spieler und nimmt ein eventuelles Fahrzeug mit, damit
 * niemand mitten in der Fahrt aus dem Auto faellt.
 */
stock Teleport_Player(playerid, Float:x, Float:y, Float:z, Float:angle = 0.0, interior = 0, world = 0)
{
    if (IsPlayerInAnyVehicleAsDriver(playerid))
    {
        new vehicleid = GetPlayerVehicleID(playerid);
        SetVehiclePos(vehicleid, x, y, z);
        SetVehicleZAngle(vehicleid, angle);
        LinkVehicleToInterior(vehicleid, interior);
        SetVehicleVirtualWorld(vehicleid, world);
    }
    else
    {
        SetPlayerPos(playerid, x, y, z);
        SetPlayerFacingAngle(playerid, angle);
    }

    SetPlayerInterior(playerid, interior);
    SetPlayerVirtualWorld(playerid, world);
    SetCameraBehindPlayer(playerid);
    GivePlayerSpawnProtection(playerid, 3000);
    return 1;
}

/*
 * Gemeinsame Pruefung fuer alle Teleportkommandos.
 */
stock bool:Teleport_CanUse(playerid)
{
    if (PlayerData[playerid][pMinigame] != MINIGAME_NONE)
    {
        SendError(playerid, "Waehrend eines Minispiels kannst du dich nicht teleportieren. (/leave)");
        return false;
    }
    if (IsPlayerJailed(playerid))
    {
        SendError(playerid, "Aus dem Jail kommst du nicht so einfach raus.");
        return false;
    }
    return true;
}

stock Teleport_Execute(playerid, index)
{
    if (index < 0 || index >= sizeof gTeleports) return 0;
    if (!Teleport_CanUse(playerid)) return 0;

    if (VipLevel(playerid) < gTeleports[index][tMinVip] && !IsAdmin(playerid, ADMIN_ADMIN))
    {
        new msg[128];
        format(msg, sizeof msg, "Dieses Ziel ist %s vorbehalten. Infos: /vip",
            GetVipRankName(gTeleports[index][tMinVip]));
        SendError(playerid, msg);
        return 0;
    }

    Teleport_Player(playerid,
        gTeleports[index][tX], gTeleports[index][tY], gTeleports[index][tZ],
        gTeleports[index][tAngle], gTeleports[index][tInterior]);

    new msg[96];
    format(msg, sizeof msg, "Teleportiert nach: %s%s", EC_WHITE, gTeleports[index][tName]);
    SendSuccess(playerid, msg);
    return 1;
}

// ---------------------------------------------------------------------------
// Dialoge
// ---------------------------------------------------------------------------
stock Teleport_ShowCategories(playerid)
{
    new list[256], entry[64];

    for (new c = 0; c < TP_CATEGORY_COUNT; c++)
    {
        new count = 0;
        for (new t = 0; t < sizeof gTeleports; t++)
        {
            if (gTeleports[t][tCategory] == c) count++;
        }
        format(entry, sizeof entry, "%s%s %s(%d)\n", EC_WHITE, gTeleportCategories[c], EC_GREY, count);
        strcat(list, entry);
    }

    return ShowPlayerDialog(playerid, DIALOG_TELEPORT_CAT, DIALOG_STYLE_LIST,
        "Teleport - Kategorie waehlen", list, "Weiter", "Abbrechen");
}

stock Teleport_ShowList(playerid, category)
{
    PlayerData[playerid][pDialogContext] = category;

    new list[1024], entry[96];

    for (new t = 0; t < sizeof gTeleports; t++)
    {
        if (gTeleports[t][tCategory] != category) continue;

        if (gTeleports[t][tMinVip] > VIP_NONE)
        {
            format(entry, sizeof entry, "%s%s %s[%s]\n",
                EC_WHITE, gTeleports[t][tName], EC_GOLD, GetVipRankName(gTeleports[t][tMinVip]));
        }
        else
        {
            format(entry, sizeof entry, "%s%s\n", EC_WHITE, gTeleports[t][tName]);
        }
        strcat(list, entry);
    }

    new caption[64];
    format(caption, sizeof caption, "Teleport - %s", gTeleportCategories[category]);

    return ShowPlayerDialog(playerid, DIALOG_TELEPORT_LIST, DIALOG_STYLE_LIST,
        caption, list, "Teleport", "Zurueck");
}

stock Teleport_OnDialogResponse(playerid, dialogid, response, listitem)
{
    switch (dialogid)
    {
        case DIALOG_TELEPORT_CAT:
        {
            if (!response) return 1;
            Teleport_ShowList(playerid, listitem);
            return 1;
        }
        case DIALOG_TELEPORT_LIST:
        {
            if (!response) { Teleport_ShowCategories(playerid); return 1; }

            // listitem zaehlt nur innerhalb der Kategorie - passenden Eintrag suchen
            new category = PlayerData[playerid][pDialogContext], seen = 0;

            for (new t = 0; t < sizeof gTeleports; t++)
            {
                if (gTeleports[t][tCategory] != category) continue;
                if (seen++ == listitem) { Teleport_Execute(playerid, t); break; }
            }
            return 1;
        }
    }
    return 0;
}

// ---------------------------------------------------------------------------
// Kommandos
// ---------------------------------------------------------------------------
CMD:teleport(playerid, params[])
{
    // Ohne Argument das Menue, mit Argument die Namenssuche
    if (params[0] == EOS) return Teleport_ShowCategories(playerid);

    new best = -1, matches = 0;
    for (new t = 0; t < sizeof gTeleports; t++)
    {
        if (strfind(gTeleports[t][tName], params, true) != -1)
        {
            best = t;
            matches++;
        }
    }

    if (matches == 0)
    {
        SendError(playerid, "Kein Teleportziel mit diesem Namen gefunden. /tp zeigt alle Ziele.");
        return 1;
    }
    if (matches > 1)
    {
        SendError(playerid, "Mehrere Ziele passen - bitte genauer angeben oder /tp benutzen.");
        return 1;
    }

    Teleport_Execute(playerid, best);
    return 1;
}
ALIAS:tp(teleport)
CMD:ls(playerid, params[])
{
    #pragma unused params
    if (!Teleport_CanUse(playerid)) return 1;
    Teleport_Player(playerid, 1481.0, -1770.0, 18.79, 90.0);
    SendSuccess(playerid, "Willkommen in Los Santos.");
    return 1;
}

CMD:sf(playerid, params[])
{
    #pragma unused params
    if (!Teleport_CanUse(playerid)) return 1;
    Teleport_Player(playerid, -1980.0, 137.0, 27.69, 90.0);
    SendSuccess(playerid, "Willkommen in San Fierro.");
    return 1;
}

CMD:lv(playerid, params[])
{
    #pragma unused params
    if (!Teleport_CanUse(playerid)) return 1;
    Teleport_Player(playerid, 2036.0, 1546.0, 10.82, 90.0);
    SendSuccess(playerid, "Willkommen in Las Venturas.");
    return 1;
}

CMD:zt(playerid, params[])
{
    #pragma unused params
    if (!Teleport_CanUse(playerid)) return 1;
    Teleport_Player(playerid, 1256.0, -1400.0, 13.55, 0.0);
    SendSuccess(playerid, "ZomboTech Labor - der Aufzug ist im Foyer.");
    return 1;
}

CMD:a51(playerid, params[])
{
    #pragma unused params
    if (!Teleport_CanUse(playerid)) return 1;
    Teleport_Player(playerid, 135.0, 1943.0, 19.9, 0.0);
    SendSuccess(playerid, "Area 51 - viel Glueck.");
    return 1;
}

CMD:chiliad(playerid, params[])
{
    #pragma unused params
    if (!Teleport_CanUse(playerid)) return 1;
    Teleport_Player(playerid, -2340.0, -1600.0, 483.7, 180.0);
    SendSuccess(playerid, "Mount Chiliad - der Weg nach unten ist der spannende Teil.");
    return 1;
}

/*
 * Zu einem anderen Spieler teleportieren. Auf Freeroam-Servern ueblich; das
 * Ziel wird informiert und der Ankommende bekommt kurzen Spawnschutz, damit
 * daraus kein Hinterhalt wird.
 */
CMD:tpto(playerid, params[])
{
    require(params[0] != EOS, "Benutzung: /tpto [Spieler]");
    if (!Teleport_CanUse(playerid)) return 1;

    new target = ResolveTarget(playerid, params);
    if (target == INVALID_PLAYER_ID) return 1;
    require(target != playerid, "Du bist bereits bei dir selbst.");

    new Float:x, Float:y, Float:z;
    GetPlayerPos(target, x, y, z);
    Teleport_Player(playerid, x + 1.5, y + 1.5, z,
        0.0, GetPlayerInterior(target), GetPlayerVirtualWorld(target));

    new msg[96];
    format(msg, sizeof msg, "Du wurdest zu %s teleportiert.", GetName(target));
    SendSuccess(playerid, msg);

    format(msg, sizeof msg, "%s hat sich zu dir teleportiert.", GetName(playerid));
    SendInfo(target, msg);
    return 1;
}
