/*
 * UIF Freeroam - Fahrzeuge
 *
 * Statische Fahrzeuge an allen wichtigen Punkten der Karte plus ein
 * Spawnkommando fuer eigene Fahrzeuge. Jeder Spieler besitzt hoechstens ein
 * selbst gespawntes Fahrzeug; es verschwindet beim Verlassen des Servers.
 */

#if defined _uif_vehicle_included
    #endinput
#endif
#define _uif_vehicle_included

#define VEHICLE_MODEL_MIN       (400)
#define VEHICLE_MODEL_MAX       (611)

// Vom Spieler selbst gespawntes Fahrzeug (INVALID_VEHICLE_ID = keins)
static gPlayerVehicle[MAX_PLAYERS];

// Modellnamen 400-611 fuer die Suche per Name
static const gVehicleNames[][] =
{
    "Landstalker", "Bravura", "Buffalo", "Linerunner", "Perennial", "Sentinel",
    "Dumper", "Firetruck", "Trashmaster", "Stretch", "Manana", "Infernus",
    "Voodoo", "Pony", "Mule", "Cheetah", "Ambulance", "Leviathan", "Moonbeam",
    "Esperanto", "Taxi", "Washington", "Bobcat", "Mr Whoopee", "BF Injection",
    "Hunter", "Premier", "Enforcer", "Securicar", "Banshee", "Predator",
    "Bus", "Rhino", "Barracks", "Hotknife", "Trailer", "Previon", "Coach",
    "Cabbie", "Stallion", "Rumpo", "RC Bandit", "Romero", "Packer", "Monster",
    "Admiral", "Squalo", "Seasparrow", "Pizzaboy", "Tram", "Trailer",
    "Turismo", "Speeder", "Reefer", "Tropic", "Flatbed", "Yankee", "Caddy",
    "Solair", "Berkley's RC Van", "Skimmer", "PCJ-600", "Faggio", "Freeway",
    "RC Baron", "RC Raider", "Glendale", "Oceanic", "Sanchez", "Sparrow",
    "Patriot", "Quadbike", "Coastguard", "Dinghy", "Hermes", "Sabre",
    "Rustler", "ZR-350", "Walton", "Regina", "Comet", "BMX", "Burrito",
    "Camper", "Marquis", "Baggage", "Dozer", "Maverick", "News Chopper",
    "Rancher", "FBI Rancher", "Virgo", "Greenwood", "Jetmax", "Hotring",
    "Sandking", "Blista Compact", "Police Maverick", "Boxville", "Benson",
    "Mesa", "RC Goblin", "Hotring Racer A", "Hotring Racer B", "Bloodring Banger",
    "Rancher", "Super GT", "Elegant", "Journey", "Bike", "Mountain Bike",
    "Beagle", "Cropduster", "Stunt", "Tanker", "Roadtrain", "Nebula",
    "Majestic", "Buccaneer", "Shamal", "Hydra", "FCR-900", "NRG-500",
    "HPV1000", "Cement Truck", "Tow Truck", "Fortune", "Cadrona", "FBI Truck",
    "Willard", "Forklift", "Tractor", "Combine", "Feltzer", "Remington",
    "Slamvan", "Blade", "Freight", "Streak", "Vortex", "Vincent", "Bullet",
    "Clover", "Sadler", "Firetruck LA", "Hustler", "Intruder", "Primo",
    "Cargobob", "Tampa", "Sunrise", "Merit", "Utility", "Nevada", "Yosemite",
    "Windsor", "Monster A", "Monster B", "Uranus", "Jester", "Sultan",
    "Stratum", "Elegy", "Raindance", "RC Tiger", "Flash", "Tahoma", "Savanna",
    "Bandito", "Freight Flat", "Streak Carriage", "Kart", "Mower",
    "Duneride", "Sweeper", "Broadway", "Tornado", "AT-400", "DFT-30",
    "Huntley", "Stafford", "BF-400", "News Van", "Tug", "Trailer A", "Emperor",
    "Wayfarer", "Euros", "Hotdog", "Club", "Freight Carriage", "Trailer B",
    "Andromada", "Dodo", "RC Cam", "Launch", "Police Car (LSPD)",
    "Police Car (SFPD)", "Police Car (LVPD)", "Police Ranger", "Picador",
    "S.W.A.T. Van", "Alpha", "Phoenix", "Glendale Shit", "Sadler Shit",
    "Baggage Trailer A", "Baggage Trailer B", "Tug Stairs Trailer", "Boxville",
    "Farm Plow", "Utility Trailer"
};

// Die Tabelle muss jedes Modell von 400 bis 611 abdecken, sonst greift
// Vehicle_GetModelName daneben.
#assert sizeof(gVehicleNames) == (VEHICLE_MODEL_MAX - VEHICLE_MODEL_MIN + 1)

enum E_STATIC_VEHICLE
{
    svModel,
    Float:svX,
    Float:svY,
    Float:svZ,
    Float:svAngle
};

static const gStaticVehicles[][E_STATIC_VEHICLE] =
{
    // Las Venturas
    {411, 1968.0, 1343.0, 15.4,  90.0},
    {541, 1968.0, 1338.0, 15.4,  90.0},
    {522, 1963.0, 1348.0, 15.3,   0.0},
    {487, 1930.0, 1360.0, 15.5, 180.0},
    {560, 2036.0, 1546.0, 10.8,  90.0},
    {451, 2041.0, 1546.0, 10.8,  90.0},
    {522, 2046.0, 1546.0, 10.8,  90.0},
    {429, 2189.0, 1677.0, 12.4, 180.0},
    {415, 2194.0, 1677.0, 12.4, 180.0},
    {468, 1685.0, 1447.0, 10.8,   0.0},
    {519, 1650.0, 1500.0, 11.0,   0.0},
    {476, 1620.0, 1520.0, 11.0,   0.0},

    // Los Santos
    {560, 2495.0, -1687.0, 13.5,   0.0},
    {567, 2500.0, -1687.0, 13.5,   0.0},
    {475, 2505.0, -1687.0, 13.5,   0.0},
    {522, 2490.0, -1682.0, 13.4,  90.0},
    {411, 1481.0, -1770.0, 18.8,  90.0},
    {541, 1486.0, -1770.0, 18.8,  90.0},
    {429, 1491.0, -1770.0, 18.8,  90.0},
    {487, 1685.0, -2334.0, 13.6,   0.0},
    {519, 1650.0, -2380.0, 14.0,   0.0},
    {468,  364.0, -1877.0,  4.6, 180.0},
    {473,  340.0, -1900.0,  0.5, 180.0},
    {581, 2769.0, -1618.0, 12.1,  90.0},

    // San Fierro
    {560, -1980.0,  137.0, 27.7,  90.0},
    {451, -1975.0,  137.0, 27.7,  90.0},
    {522, -1985.0,  137.0, 27.7,  90.0},
    {487, -1400.0, -300.0, 14.2,   0.0},
    {519, -1350.0, -280.0, 14.5,   0.0},
    {411, -2707.0, 1439.0,  7.1, 180.0},
    {468, -2438.0, 2287.0,  5.0,   0.0},
    {453, -2460.0, 2300.0,  0.5,   0.0},

    // Landgebiete & Sperrzonen
    {487,  135.0, 1943.0, 19.9,   0.0},
    {432,  213.0, 1909.0, 17.7,  90.0},
    {476,  400.0, 2536.0, 16.5, 270.0},
    {468, -2340.0, -1600.0, 483.7, 180.0},
    {509, -2335.0, -1605.0, 483.7, 180.0},
    {471,  595.0,  852.0, -42.5,   0.0},
    {541, -215.0,  980.0, 19.3,  90.0},
    {415, -1471.0, 2620.0, 55.8,   0.0}
};

stock Vehicle_OnGameModeInit()
{
    for (new i = 0; i < MAX_PLAYERS; i++) gPlayerVehicle[i] = INVALID_VEHICLE_ID;

    new created = 0;
    for (new i = 0; i < sizeof gStaticVehicles; i++)
    {
        if (AddStaticVehicleEx(gStaticVehicles[i][svModel],
                gStaticVehicles[i][svX], gStaticVehicles[i][svY], gStaticVehicles[i][svZ],
                gStaticVehicles[i][svAngle], -1, -1, 120) != INVALID_VEHICLE_ID)
        {
            created++;
        }
    }

    new msg[64];
    format(msg, sizeof msg, "%d statische Fahrzeuge erstellt.", created);
    Log(msg);
    return created;
}

// ---------------------------------------------------------------------------
// Modellsuche
// ---------------------------------------------------------------------------
stock Vehicle_GetModelName(model)
{
    new name[32];
    if (model < VEHICLE_MODEL_MIN || model > VEHICLE_MODEL_MAX) name = "Unbekannt";
    else format(name, sizeof name, "%s", gVehicleNames[model - VEHICLE_MODEL_MIN]);
    return name;
}

/*
 * Findet ein Modell anhand einer ID oder eines Namensfragments.
 * Rueckgabe: Modell-ID, -1 = nichts gefunden, -2 = mehrdeutig.
 */
stock Vehicle_FindModel(const input[])
{
    if (input[0] == EOS) return -1;

    new bool:numeric = true;
    for (new i = 0; input[i] != EOS; i++)
    {
        if (input[i] < '0' || input[i] > '9') { numeric = false; break; }
    }

    if (numeric)
    {
        new model = strval(input);
        if (model >= VEHICLE_MODEL_MIN && model <= VEHICLE_MODEL_MAX) return model;
        return -1;
    }

    new found = -1, matches = 0;
    for (new i = 0; i < sizeof gVehicleNames; i++)
    {
        if (!strcmp(gVehicleNames[i], input, true)) return VEHICLE_MODEL_MIN + i;

        if (strfind(gVehicleNames[i], input, true) != -1)
        {
            found = VEHICLE_MODEL_MIN + i;
            matches++;
        }
    }

    if (matches > 1) return -2;
    return found;
}

// ---------------------------------------------------------------------------
// Eigene Fahrzeuge
// ---------------------------------------------------------------------------
stock Vehicle_DestroyPlayerVehicle(playerid)
{
    if (gPlayerVehicle[playerid] == INVALID_VEHICLE_ID) return 0;

    DestroyVehicle(gPlayerVehicle[playerid]);
    gPlayerVehicle[playerid] = INVALID_VEHICLE_ID;
    return 1;
}

stock Vehicle_SpawnForPlayer(playerid, model, color1 = -1, color2 = -1)
{
    new Float:x, Float:y, Float:z, Float:angle;
    GetPlayerPos(playerid, x, y, z);
    GetPlayerFacingAngle(playerid, angle);

    // Vor dem Spieler absetzen, damit er nicht im Fahrzeug steckt
    x += 3.0 * floatsin(-angle, degrees);
    y += 3.0 * floatcos(-angle, degrees);

    Vehicle_DestroyPlayerVehicle(playerid);

    new vehicleid = CreateVehicle(model, x, y, z + 0.5, angle, color1, color2, -1);
    if (vehicleid == INVALID_VEHICLE_ID)
    {
        SendError(playerid, "Das Fahrzeuglimit des Servers ist erreicht. Bitte spaeter erneut versuchen.");
        return INVALID_VEHICLE_ID;
    }

    gPlayerVehicle[playerid] = vehicleid;
    LinkVehicleToInterior(vehicleid, GetPlayerInterior(playerid));
    SetVehicleVirtualWorld(vehicleid, GetPlayerVirtualWorld(playerid));
    PutPlayerInVehicle(playerid, vehicleid, 0);
    return vehicleid;
}

// ---------------------------------------------------------------------------
// Callbacks
// ---------------------------------------------------------------------------
stock Vehicle_OnPlayerDisconnect(playerid)
{
    return Vehicle_DestroyPlayerVehicle(playerid);
}

stock Vehicle_OnPlayerStateChange(playerid, newstate, oldstate)
{
    #pragma unused oldstate

    if (newstate == PLAYER_STATE_DRIVER)
    {
        new vehicleid = GetPlayerVehicleID(playerid);

        // /vd aus: Fahrzeug beim Einsteigen optisch instand setzen
        if (!PlayerData[playerid][pVehicleDamage])
        {
            UpdateVehicleDamageStatus(vehicleid, 0, 0, 0, 0);
        }
    }
    return 1;
}

stock Vehicle_OnPlayerKeyStateChange(playerid, newkeys, oldkeys)
{
    #pragma unused oldkeys

    // Hupe im Fahrzeug repariert bei VIPs die Karosserie (kleines Extra)
    if ((newkeys & KEY_CROUCH) && IsPlayerInAnyVehicleAsDriver(playerid) && IsVip(playerid))
    {
        new vehicleid = GetPlayerVehicleID(playerid);
        RepairVehicle(vehicleid);
        SendInfo(playerid, "VIP-Bonus: Fahrzeug repariert.");
    }
    return 1;
}

/*
 * Haelt Fahrzeuge von Spielern mit deaktiviertem Schadensmodell optisch heil.
 */
stock Vehicle_Tick()
{
    foreach_player(i)
    {
        if (!IsLoggedIn(i) || PlayerData[i][pVehicleDamage]) continue;
        if (!IsPlayerInAnyVehicleAsDriver(i)) continue;

        UpdateVehicleDamageStatus(GetPlayerVehicleID(i), 0, 0, 0, 0);
    }
    return 1;
}

// ---------------------------------------------------------------------------
// Kommandos
// ---------------------------------------------------------------------------
CMD:veh(playerid, params[])
{
    require(params[0] != EOS, "Benutzung: /veh [Modell-ID oder Name] [Farbe1] [Farbe2]");
    require(PlayerData[playerid][pMinigame] == MINIGAME_NONE, "Waehrend eines Minispiels kannst du keine Fahrzeuge spawnen.");

    new arg[32];
    Params_Get(params, 0, arg, sizeof arg);

    new model = Vehicle_FindModel(arg);
    require(model != -2, "Mehrere Modelle passen zu diesem Namen - bitte genauer angeben.");
    require(model != -1, "Unbekanntes Fahrzeug. Erlaubt sind IDs 400-611 oder ein Modellname.");

    new color1 = (Params_Count(params) >= 2) ? Params_Int(params, 1) : random(256);
    new color2 = (Params_Count(params) >= 3) ? Params_Int(params, 2) : random(256);

    if (Vehicle_SpawnForPlayer(playerid, model, color1, color2) == INVALID_VEHICLE_ID) return 1;

    new msg[96];
    format(msg, sizeof msg, "%s (ID %d) gespawnt.", Vehicle_GetModelName(model), model);
    SendSuccess(playerid, msg);
    return 1;
}
ALIAS:car(veh)
CMD:dv(playerid, params[])
{
    #pragma unused params
    require(gPlayerVehicle[playerid] != INVALID_VEHICLE_ID, "Du hast kein eigenes Fahrzeug gespawnt.");

    Vehicle_DestroyPlayerVehicle(playerid);
    SendSuccess(playerid, "Dein Fahrzeug wurde entfernt.");
    return 1;
}

CMD:fix(playerid, params[])
{
    #pragma unused params
    require(IsPlayerInAnyVehicle(playerid), "Du sitzt in keinem Fahrzeug.");

    new vehicleid = GetPlayerVehicleID(playerid);
    RepairVehicle(vehicleid);
    SetVehicleHealth(vehicleid, 1000.0);
    SendSuccess(playerid, "Fahrzeug repariert.");
    return 1;
}
ALIAS:repair(fix)
CMD:flip(playerid, params[])
{
    #pragma unused params
    require(IsPlayerInAnyVehicleAsDriver(playerid), "Du musst der Fahrer sein.");

    new vehicleid = GetPlayerVehicleID(playerid);
    new Float:x, Float:y, Float:z, Float:angle;
    GetVehiclePos(vehicleid, x, y, z);
    GetVehicleZAngle(vehicleid, angle);

    SetVehiclePos(vehicleid, x, y, z + 2.0);
    SetVehicleZAngle(vehicleid, angle);
    SendSuccess(playerid, "Fahrzeug wieder auf die Raeder gestellt.");
    return 1;
}

CMD:nos(playerid, params[])
{
    #pragma unused params
    require(IsPlayerInAnyVehicleAsDriver(playerid), "Du musst der Fahrer sein.");

    AddVehicleComponent(GetPlayerVehicleID(playerid), 1010);
    SendSuccess(playerid, "NOS eingebaut.");
    return 1;
}

CMD:color(playerid, params[])
{
    require(IsPlayerInAnyVehicleAsDriver(playerid), "Du musst der Fahrer sein.");
    require(Params_Count(params) >= 2, "Benutzung: /color [Farbe1 0-255] [Farbe2 0-255]");

    new c1 = Params_Int(params, 0), c2 = Params_Int(params, 1);
    require(c1 >= 0 && c1 <= 255 && c2 >= 0 && c2 <= 255, "Farbwerte muessen zwischen 0 und 255 liegen.");

    ChangeVehicleColor(GetPlayerVehicleID(playerid), c1, c2);
    SendSuccess(playerid, "Farbe geaendert.");
    return 1;
}

/*
 * /vd - sichtbaren Fahrzeugschaden an- oder abschalten (Einstellung wird
 * im Account gespeichert, wie im Original).
 */
CMD:vd(playerid, params[])
{
    #pragma unused params

    PlayerData[playerid][pVehicleDamage] = !PlayerData[playerid][pVehicleDamage];

    if (PlayerData[playerid][pVehicleDamage])
    {
        SendSuccess(playerid, "Sichtbarer Fahrzeugschaden: " EC_WHITE "an");
    }
    else
    {
        SendSuccess(playerid, "Sichtbarer Fahrzeugschaden: " EC_WHITE "aus");
        if (IsPlayerInAnyVehicleAsDriver(playerid))
        {
            UpdateVehicleDamageStatus(GetPlayerVehicleID(playerid), 0, 0, 0, 0);
        }
    }
    return 1;
}

CMD:lock(playerid, params[])
{
    #pragma unused params
    require(IsPlayerInAnyVehicleAsDriver(playerid), "Du musst der Fahrer sein.");

    new vehicleid = GetPlayerVehicleID(playerid);
    foreach_player(i)
    {
        if (i != playerid) SetVehicleParamsForPlayer(vehicleid, i, 0, 1);
    }
    SendSuccess(playerid, "Fahrzeug abgeschlossen.");
    return 1;
}

CMD:unlock(playerid, params[])
{
    #pragma unused params
    require(IsPlayerInAnyVehicleAsDriver(playerid), "Du musst der Fahrer sein.");

    new vehicleid = GetPlayerVehicleID(playerid);
    foreach_player(i)
    {
        SetVehicleParamsForPlayer(vehicleid, i, 0, 0);
    }
    SendSuccess(playerid, "Fahrzeug aufgeschlossen.");
    return 1;
}
