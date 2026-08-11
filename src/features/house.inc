/*
 * Jebiga-Gaming - Haeuser
 *
 * Kaufbare Haeuser mit begehbaren Innenraeumen. Die Standorte stehen fest im
 * Skript, nur der Besitzer wird in scriptfiles/houses.ini gespeichert. Einige
 * Objekte sind VIPs vorbehalten.
 */

#if defined _jebiga_house_included
    #endinput
#endif
#define _jebiga_house_included

#define HOUSE_ENTER_RANGE       (2.5)

enum E_HOUSE
{
    hName[40],
    Float:hX,               // Eingang aussen
    Float:hY,
    Float:hZ,
    Float:hIntX,            // Position im Innenraum
    Float:hIntY,
    Float:hIntZ,
    hInterior,
    hPrice,
    hMinVip
};

static const gHouses[][E_HOUSE] =
{
    {"Grove Street 4",        2496.0500, -1692.5300, 13.5100,  2496.0500, -1695.2300, 1014.7400,  3,   150000, VIP_NONE},
    {"Idlewood Apartment",    2201.3600, -1153.1000, 25.7300,  2233.8000, -1115.3000, 1050.8800,  5,   120000, VIP_NONE},
    {"Jefferson Motel",       2226.0000, -1153.0000, 25.8900,  2215.5000, -1076.2000, 1050.4800, 15,    95000, VIP_NONE},
    {"Santa Maria Beach Haus", 364.0000, -1870.0000,  4.5600,   225.6000,  1240.0000, 1082.1400,  1,   220000, VIP_NONE},
    {"Mulholland Villa",      1299.1000, -797.0000,  84.1400,  2259.4000, -1135.9000, 1050.6300,  6,   480000, VIP_NONE},
    {"Vinewood Loft",         1082.0000, -344.0000,  73.9800,   235.5000,  1189.2000, 1080.3400,  9,   310000, VIP_NONE},
    {"Rodeo Penthouse",        615.0000, -1230.0000, 18.2600,   295.1000,  1474.5000, 1080.2600, 15,   650000, VIP_BRONZE},
    {"Palomino Ranch",        2255.0000,    62.0000, 26.4800,   223.0000,  1289.0000, 1082.2000,  1,   275000, VIP_NONE},
    {"Whetstone Huette",     -2160.0000, -2395.0000, 30.6200,   266.3000,  1235.0000, 1084.2600,  9,    85000, VIP_NONE},
    {"San Fierro Stadthaus", -2670.0000,   258.0000, 4.6300,   2308.8000, -1212.9000, 1049.0200, 10,   340000, VIP_NONE},
    {"Bayside Marina",       -2394.0000,  2316.0000,  4.9800,   225.6000,  1022.0000, 1084.0700,  7,   295000, VIP_NONE},
    {"Las Venturas Suite",    2036.0000,  1540.0000, 10.8200,  2196.8000, -1204.3000, 1049.0200,  6,   520000, VIP_NONE},
    {"Redsands Bungalow",     1800.0000,  2200.0000, 10.8200,   235.5000,  1189.2000, 1080.3400,  9,   180000, VIP_NONE},
    {"Prickle Pine Anwesen",  1266.0000,  2000.0000, 10.8200,  2259.4000, -1135.9000, 1050.6300,  6,   420000, VIP_SILVER},
    {"Chiliad Berghaus",     -2290.0000, -1650.0000, 483.7000,  266.3000,  1235.0000, 1084.2600,  9,   750000, VIP_GOLD}
};

// Besitzer je Haus ("" = frei), zur Laufzeit veraenderbar
static gHouseOwner[sizeof gHouses][MAX_PLAYER_NAME + 1];
static gHousePickup[sizeof gHouses];
static Text3D:gHouseLabel[sizeof gHouses];

// ---------------------------------------------------------------------------
// Anzeige
// ---------------------------------------------------------------------------
static House_UpdateLabel(index)
{
    new text[192];

    if (gHouseOwner[index][0] == EOS)
    {
        new price[24];
        FormatNumber(gHouses[index][hPrice], price, sizeof price);

        if (gHouses[index][hMinVip] > VIP_NONE)
        {
            format(text, sizeof text, "{2ECC71}%s\n{FFFFFF}Zu verkaufen: {F1C40F}$%s\n{FFD700}Nur %s\n{AFAFAF}/buyhouse",
                gHouses[index][hName], price, GetVipRankName(gHouses[index][hMinVip]));
        }
        else
        {
            format(text, sizeof text, "{2ECC71}%s\n{FFFFFF}Zu verkaufen: {F1C40F}$%s\n{AFAFAF}/buyhouse",
                gHouses[index][hName], price);
        }
    }
    else
    {
        format(text, sizeof text, "{E74C3C}%s\n{FFFFFF}Besitzer: {1ABC9C}%s\n{AFAFAF}/enter",
            gHouses[index][hName], gHouseOwner[index]);
    }

    if (gHouseLabel[index] != Text3D:INVALID_3DTEXT_ID) Delete3DTextLabel(gHouseLabel[index]);

    gHouseLabel[index] = Create3DTextLabel(text, COL_WHITE,
        gHouses[index][hX], gHouses[index][hY], gHouses[index][hZ] + 0.6, 20.0, 0, 1);
    return 1;
}

// ---------------------------------------------------------------------------
// Speichern / Laden
// ---------------------------------------------------------------------------
stock House_Load()
{
    for (new i = 0; i < sizeof gHouses; i++) gHouseOwner[i][0] = EOS;

    if (!fexist(PATH_HOUSES)) return 0;

    new key[16], loaded = 0;
    for (new i = 0; i < sizeof gHouses; i++)
    {
        format(key, sizeof key, "House%d", i);
        if (INI_GetString(PATH_HOUSES, key, gHouseOwner[i], MAX_PLAYER_NAME + 1) &&
            gHouseOwner[i][0] != EOS)
        {
            loaded++;
        }
    }
    return loaded;
}

stock House_SaveAll()
{
    if (!INI_Open(PATH_HOUSES)) return 0;

    new key[16];
    for (new i = 0; i < sizeof gHouses; i++)
    {
        format(key, sizeof key, "House%d", i);
        INI_WriteString(key, gHouseOwner[i]);
    }
    INI_Close();
    return 1;
}

stock House_OnGameModeInit()
{
    for (new i = 0; i < sizeof gHouses; i++)
    {
        gHouseLabel[i] = Text3D:INVALID_3DTEXT_ID;
        gHouseOwner[i][0] = EOS;
    }

    new owned = House_Load();

    for (new i = 0; i < sizeof gHouses; i++)
    {
        // Pickup-Typ 1 = sichtbar, loest kein Aufsammeln aus
        gHousePickup[i] = CreatePickup(1273, 1, gHouses[i][hX], gHouses[i][hY], gHouses[i][hZ], -1);
        House_UpdateLabel(i);
    }

    new msg[80];
    format(msg, sizeof msg, "%d Haeuser geladen (%d davon vergeben).", sizeof gHouses, owned);
    Log(msg);
    return 1;
}

// ---------------------------------------------------------------------------
// Hilfsfunktionen
// ---------------------------------------------------------------------------

// Haus, an dessen Eingang der Spieler steht (-1 = keins)
stock House_GetNearest(playerid)
{
    if (GetPlayerInterior(playerid) != 0) return -1;

    for (new i = 0; i < sizeof gHouses; i++)
    {
        if (IsPlayerInRangeOfPoint(playerid, HOUSE_ENTER_RANGE,
                gHouses[i][hX], gHouses[i][hY], gHouses[i][hZ]))
        {
            return i;
        }
    }
    return -1;
}

stock House_GetOwned(playerid)
{
    new name[MAX_PLAYER_NAME + 1];
    GetPlayerName(playerid, name, sizeof name);

    for (new i = 0; i < sizeof gHouses; i++)
    {
        if (!strcmp(gHouseOwner[i], name, true) && gHouseOwner[i][0] != EOS) return i;
    }
    return -1;
}

stock House_OnPlayerPickUpPickup(playerid, pickupid)
{
    for (new i = 0; i < sizeof gHouses; i++)
    {
        if (gHousePickup[i] != pickupid) continue;

        if (gHouseOwner[i][0] == EOS) SendInfo(playerid, "Dieses Haus steht zum Verkauf - " EC_WHITE "/buyhouse");
        else                          SendInfo(playerid, "Bewohntes Haus - " EC_WHITE "/enter");
        return 1;
    }
    return 0;
}

stock House_Buy(playerid, index)
{
    if (gHouseOwner[index][0] != EOS)
    {
        SendError(playerid, "Dieses Haus gehoert bereits jemandem.");
        return 0;
    }
    if (House_GetOwned(playerid) != -1)
    {
        SendError(playerid, "Du besitzt bereits ein Haus. Verkaufe es zuerst mit /sellhouse.");
        return 0;
    }
    if (VipLevel(playerid) < gHouses[index][hMinVip])
    {
        new msg[128];
        format(msg, sizeof msg, "Dieses Haus ist %s vorbehalten. Infos: /vip",
            GetVipRankName(gHouses[index][hMinVip]));
        SendError(playerid, msg);
        return 0;
    }
    if (GetPlayerMoney(playerid) < gHouses[index][hPrice])
    {
        new price[24], msg[128];
        FormatNumber(gHouses[index][hPrice], price, sizeof price);
        format(msg, sizeof msg, "Dir fehlt das Geld. Preis: $%s", price);
        SendError(playerid, msg);
        return 0;
    }

    GivePlayerMoney(playerid, -gHouses[index][hPrice]);
    GetPlayerName(playerid, gHouseOwner[index], MAX_PLAYER_NAME + 1);
    PlayerData[playerid][pHouse] = index;

    House_UpdateLabel(index);
    House_SaveAll();

    new price[24], msg[144];
    FormatNumber(gHouses[index][hPrice], price, sizeof price);
    format(msg, sizeof msg, "Glueckwunsch! Du hast %s fuer $%s gekauft.", gHouses[index][hName], price);
    SendSuccess(playerid, msg);

    format(msg, sizeof msg, "%s hat das Haus \"%s\" gekauft.", GetName(playerid), gHouses[index][hName]);
    SendClientMessageToAll(COL_GREY, msg);
    return 1;
}

stock House_OnDialogResponse(playerid, dialogid, response, listitem)
{
    if (dialogid != DIALOG_HOUSE_BUY) return 0;
    if (!response) return 1;

    #pragma unused listitem

    new index = House_GetNearest(playerid);
    if (index == -1)
    {
        SendError(playerid, "Du stehst nicht mehr am Eingang.");
        return 1;
    }
    House_Buy(playerid, index);
    return 1;
}

// ---------------------------------------------------------------------------
// Kommandos
// ---------------------------------------------------------------------------
CMD:buyhouse(playerid, params[])
{
    #pragma unused params

    new index = House_GetNearest(playerid);
    require(index != -1, "Du stehst an keinem Hauseingang. /houses zeigt alle Standorte.");
    require(gHouseOwner[index][0] == EOS, "Dieses Haus ist bereits verkauft.");

    new price[24], body[384];
    FormatNumber(gHouses[index][hPrice], price, sizeof price);

    format(body, sizeof body,
        "%sHaus: %s%s\n%sPreis: %s$%s\n%sDein Geld: %s$%d\n\n%sMoechtest du dieses Haus kaufen?",
        EC_GREY, EC_WHITE, gHouses[index][hName],
        EC_GREY, EC_YELLOW, price,
        EC_GREY, EC_WHITE, GetPlayerMoney(playerid),
        EC_WHITE);

    ShowPlayerDialog(playerid, DIALOG_HOUSE_BUY, DIALOG_STYLE_MSGBOX,
        "Haus kaufen", body, "Kaufen", "Abbrechen");
    return 1;
}

CMD:sellhouse(playerid, params[])
{
    #pragma unused params

    new index = House_GetOwned(playerid);
    require(index != -1, "Du besitzt kein Haus.");

    // Rueckkauf zu 70 Prozent des Kaufpreises
    new refund = (gHouses[index][hPrice] * 70) / 100;

    gHouseOwner[index][0] = EOS;
    PlayerData[playerid][pHouse] = -1;
    GivePlayerMoney(playerid, refund);

    House_UpdateLabel(index);
    House_SaveAll();

    new price[24], msg[144];
    FormatNumber(refund, price, sizeof price);
    format(msg, sizeof msg, "%s verkauft. Du erhaeltst $%s zurueck (70%% des Kaufpreises).",
        gHouses[index][hName], price);
    SendSuccess(playerid, msg);
    return 1;
}

CMD:enter(playerid, params[])
{
    #pragma unused params

    new index = House_GetNearest(playerid);
    require(index != -1, "Du stehst an keinem Hauseingang.");
    require(gHouseOwner[index][0] != EOS, "Dieses Haus steht leer - /buyhouse");

    new name[MAX_PLAYER_NAME + 1];
    GetPlayerName(playerid, name, sizeof name);

    // Besucher duerfen rein, wenn der Besitzer online ist
    if (strcmp(gHouseOwner[index], name, true) != 0)
    {
        new owner = FindPlayer(gHouseOwner[index]);
        if (owner == INVALID_PLAYER_ID || owner == PLAYER_AMBIGUOUS)
        {
            SendError(playerid, "Abgeschlossen - der Besitzer ist nicht online.");
            return 1;
        }
        new msg[96];
        format(msg, sizeof msg, "%s hat dein Haus betreten.", GetName(playerid));
        SendInfo(owner, msg);
    }

    SetPlayerPos(playerid, gHouses[index][hIntX], gHouses[index][hIntY], gHouses[index][hIntZ]);
    SetPlayerInterior(playerid, gHouses[index][hInterior]);

    // Eigener Virtual World je Haus, damit sich Innenraeume nicht ueberlagern
    SetPlayerVirtualWorld(playerid, WORLD_HOUSE_BASE + index);
    SetCameraBehindPlayer(playerid);

    new msg[96];
    format(msg, sizeof msg, "Du hast %s betreten. " EC_WHITE "/exit" EC_CYAN " fuehrt wieder hinaus.",
        gHouses[index][hName]);
    SendInfo(playerid, msg);
    return 1;
}

CMD:exit(playerid, params[])
{
    #pragma unused params

    new world = GetPlayerVirtualWorld(playerid);
    require(world >= WORLD_HOUSE_BASE && world < WORLD_HOUSE_BASE + sizeof gHouses, "Du bist in keinem Haus.");

    new index = world - WORLD_HOUSE_BASE;

    SetPlayerPos(playerid, gHouses[index][hX], gHouses[index][hY], gHouses[index][hZ] + 0.5);
    SetPlayerInterior(playerid, 0);
    SetPlayerVirtualWorld(playerid, 0);
    SetCameraBehindPlayer(playerid);
    return 1;
}

CMD:houses(playerid, params[])
{
    #pragma unused params

    new body[1400], entry[160], price[24];

    for (new i = 0; i < sizeof gHouses; i++)
    {
        FormatNumber(gHouses[i][hPrice], price, sizeof price);

        if (gHouseOwner[i][0] == EOS)
        {
            format(entry, sizeof entry, "%s%s%s - %s$%s%s frei\n",
                EC_WHITE, gHouses[i][hName], EC_GREY, EC_YELLOW, price, EC_GREEN);
        }
        else
        {
            format(entry, sizeof entry, "%s%s%s - Besitzer: %s\n",
                EC_WHITE, gHouses[i][hName], EC_GREY, gHouseOwner[i]);
        }
        strcat(body, entry);
    }

    ShowPlayerDialog(playerid, DIALOG_HOUSE_MENU, DIALOG_STYLE_MSGBOX,
        "Haeuser auf dem Server", body, "Schliessen", "");
    return 1;
}

CMD:myhouse(playerid, params[])
{
    #pragma unused params

    new index = House_GetOwned(playerid);
    require(index != -1, "Du besitzt kein Haus. /houses zeigt alle Angebote.");
    if (!Teleport_CanUse(playerid)) return 1;

    Teleport_Player(playerid, gHouses[index][hX], gHouses[index][hY], gHouses[index][hZ] + 0.5);

    new msg[96];
    format(msg, sizeof msg, "Willkommen zu Hause: %s", gHouses[index][hName]);
    SendSuccess(playerid, msg);
    return 1;
}
