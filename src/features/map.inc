/*
 * Jebiga-Gaming - Custom Map
 *
 * Eigene Bauwerke, die es in San Andreas nicht gibt: ein Stuntpark ueber der
 * Wueste, eine VIP-Lounge, eine Goldinsel und die Umrandung der Derby-Arena.
 * Alles entsteht zur Laufzeit ueber den Streamer, damit die Objektgrenzen von
 * SA-MP (1000 Objekte, 1024 Textlabels) kein Thema sind.
 *
 * Zu den Modellen
 * ---------------
 * Die verwendeten Modell-IDs stammen aus der Objektdatenbank von GTA San
 * Andreas, jeweils mit ihren tatsaechlichen Abmessungen. Fuer die Platzierung
 * zaehlt nicht die Groesse, sondern die Lage des Modellursprungs: bei
 * "loopbig" etwa reicht die Geometrie von -11.57 bis +11.59 auf der Z-Achse,
 * der Ursprung sitzt also in der Mitte. Damit ein Objekt sauber auf dem Boden
 * steht, muss es um den Betrag seiner unteren Kante angehoben werden - genau
 * dafuer stehen die MODEL_*_LIFT-Konstanten.
 *
 * Als Untergrund dienen Strassenstuecke. Die sind flach, gross und vor allem
 * garantiert kollidierbar; reine Bodenmarkierungen ohne Hoehe waeren zwar
 * huebscher, haben aber unter Umstaenden keine Kollision.
 */

#if defined _jebiga_map_included
    #endinput
#endif
#define _jebiga_map_included

// --- Untergrund ------------------------------------------------------------
#define MODEL_PLATFORM_BIG      (8472)   // VegasEroad094, 240 x 180, Oberkante +0.37
#define PLATFORM_BIG_SURFACE    (0.37)
#define MODEL_PLATFORM_MID      (5486)   // laeroad21, 89 x 89, Oberkante +0.82
#define PLATFORM_MID_SURFACE    (0.82)

// --- Stuntobjekte ----------------------------------------------------------
#define MODEL_LOOP_BIG          (13592)  // loopbig,    22 x 16 x 23
#define LIFT_LOOP_BIG           (11.57)
#define MODEL_LOOP_SMALL        (13666)  // loopwee,     8 x 11 x 12
#define LIFT_LOOP_SMALL         (5.78)
#define MODEL_KICKRAMP_BIG      (13604)  // kickramp05, 22 x 20 x 3.4
#define LIFT_KICKRAMP_BIG       (1.71)
#define MODEL_KICKRAMP_SMALL    (13593)  // kickramp03,  3 x 4 x 1.5
#define LIFT_KICKRAMP_SMALL     (0.74)
#define MODEL_TUBERAMP          (13637)  // tuberamp,   15 x 15 x 4.2
#define LIFT_TUBERAMP           (2.08)
#define MODEL_LOGRAMPS          (13636)  // logramps,   20 x 20 x 4.6
#define LIFT_LOGRAMPS           (2.28)
#define MODEL_OTTOS_RAMP        (10245)  // ottos_ramp, 14 x 13 x 12.5
#define LIFT_OTTOS_RAMP         (6.53)
#define MODEL_NEWRAMP           (1245)   // newramp,     5 x 3.4 x 3
#define LIFT_NEWRAMP            (1.41)
#define MODEL_SF_JUMP           (3852)   // sf_jump,   4.6 x 14 x 3.4
#define LIFT_SF_JUMP            (1.81)

// --- Kleinteile ------------------------------------------------------------
#define MODEL_BARRIER           (973)    // sub_roadbarrier, 9.4 lang, 1.3 hoch
#define LIFT_BARRIER            (0.84)
#define BARRIER_LENGTH          (9.4)
#define MODEL_CONE              (1238)   // trafficcone
#define LIFT_CONE               (0.32)

// Derby-Arena: quadratische Umrandung am verlassenen Flughafen
#define DERBY_ARENA_X           (400.0)
#define DERBY_ARENA_Y           (2536.0)
#define DERBY_ARENA_Z           (16.5)
#define DERBY_ARENA_HALFSIZE    (48.0)

static gMapObjectCount = 0;

/*
 * Legt ein Objekt an und zaehlt mit. worldid -1 heisst: in allen Welten
 * sichtbar.
 */
static Map_Object(model, Float:x, Float:y, Float:z,
                  Float:rx = 0.0, Float:ry = 0.0, Float:rz = 0.0, world = -1)
{
    gMapObjectCount++;
    return _:CreateDynamicObject(model, x, y, z, rx, ry, rz, world);
}

/*
 * Zieht eine gerade Wand aus Absperrungen.
 *
 * Die Absperrung liegt im Modell entlang der X-Achse. Eine Wand in
 * X-Richtung braucht daher keine Drehung, eine Wand in Y-Richtung 90 Grad.
 * Die Segmente ueberlappen leicht (Schrittweite kleiner als die Bauteillaenge),
 * damit auch bei ungerader Laenge keine Luecke bleibt.
 */
static Map_BarrierWall(Float:startX, Float:startY, Float:z, Float:length,
                       bool:alongY, world = -1)
{
    new Float:step = BARRIER_LENGTH - 0.4;
    new count = floatround(length / step, floatround_ceil);
    if (count < 1) count = 1;

    for (new i = 0; i < count; i++)
    {
        new Float:offset = (float(i) + 0.5) * step;
        if (offset > length) offset = length;

        if (alongY) Map_Object(MODEL_BARRIER, startX, startY + offset, z + LIFT_BARRIER, 0.0, 0.0, 90.0, world);
        else        Map_Object(MODEL_BARRIER, startX + offset, startY, z + LIFT_BARRIER, 0.0, 0.0,  0.0, world);
    }
    return count;
}

/*
 * Gelaender rund um eine quadratische Plattform.
 */
static Map_BarrierSquare(Float:cx, Float:cy, Float:z, Float:halfSize, world = -1)
{
    new Float:side = halfSize * 2.0;

    Map_BarrierWall(cx - halfSize, cy - halfSize, z, side, false, world);  // Sued
    Map_BarrierWall(cx - halfSize, cy + halfSize, z, side, false, world);  // Nord
    Map_BarrierWall(cx - halfSize, cy - halfSize, z, side, true,  world);  // West
    Map_BarrierWall(cx + halfSize, cy - halfSize, z, side, true,  world);  // Ost
    return 1;
}

// ---------------------------------------------------------------------------
// Bauwerke
// ---------------------------------------------------------------------------

/*
 * Jebiga Stuntpark - eine schwebende Strassenplatte von 240 x 180 Metern mit
 * Looping, Rampen und Sprungschanzen. Bewusst ohne Gelaender: herunterfallen
 * gehoert zum Stunten dazu.
 */
static Map_BuildStuntPark()
{
    new Float:cx = MAP_PARK_X, Float:cy = MAP_PARK_Y;
    new Float:base = MAP_PARK_Z - PLATFORM_BIG_SURFACE;   // Ursprung der Platte
    new Float:g = MAP_PARK_Z;                             // begehbare Oberflaeche

    Map_Object(MODEL_PLATFORM_BIG, cx, cy, base);

    // Grosser Looping in der Mitte, Anfahrt aus Sueden
    Map_Object(MODEL_LOOP_BIG, cx, cy, g + LIFT_LOOP_BIG);
    Map_Object(MODEL_KICKRAMP_BIG, cx, cy - 40.0, g + LIFT_KICKRAMP_BIG);

    // Zweiter, kleinerer Looping im Osten
    Map_Object(MODEL_LOOP_SMALL, cx + 60.0, cy, g + LIFT_LOOP_SMALL);
    Map_Object(MODEL_KICKRAMP_SMALL, cx + 60.0, cy - 18.0, g + LIFT_KICKRAMP_SMALL);

    // Sprungschanze nach Norden, mit Landerampe dahinter
    Map_Object(MODEL_OTTOS_RAMP, cx - 60.0, cy - 20.0, g + LIFT_OTTOS_RAMP);
    Map_Object(MODEL_KICKRAMP_BIG, cx - 60.0, cy + 30.0, g + LIFT_KICKRAMP_BIG, 0.0, 0.0, 180.0);

    // Roehrenrampe und Baumstammrampen als Hindernisparcours
    Map_Object(MODEL_TUBERAMP, cx + 20.0, cy + 55.0, g + LIFT_TUBERAMP);
    Map_Object(MODEL_LOGRAMPS, cx - 25.0, cy + 60.0, g + LIFT_LOGRAMPS);

    // Kleine Schanzen im Karree, jeweils zur Mitte ausgerichtet
    Map_Object(MODEL_NEWRAMP, cx + 90.0, cy + 60.0, g + LIFT_NEWRAMP, 0.0, 0.0, 180.0);
    Map_Object(MODEL_NEWRAMP, cx - 90.0, cy + 60.0, g + LIFT_NEWRAMP, 0.0, 0.0, 180.0);
    Map_Object(MODEL_NEWRAMP, cx + 90.0, cy - 60.0, g + LIFT_NEWRAMP);
    Map_Object(MODEL_NEWRAMP, cx - 90.0, cy - 60.0, g + LIFT_NEWRAMP);

    Map_Object(MODEL_SF_JUMP, cx + 40.0, cy - 60.0, g + LIFT_SF_JUMP);
    Map_Object(MODEL_SF_JUMP, cx - 40.0, cy - 60.0, g + LIFT_SF_JUMP);

    // Pylonen markieren die Anfahrt zum grossen Looping
    for (new i = 0; i < 8; i++)
    {
        new Float:offset = 8.0 + float(i) * 4.0;
        Map_Object(MODEL_CONE, cx - 9.0, cy - offset, g + LIFT_CONE);
        Map_Object(MODEL_CONE, cx + 9.0, cy - offset, g + LIFT_CONE);
    }

    CreateDynamic3DTextLabel("{FFD700}Jebiga Stuntpark\n{FFFFFF}Looping, Schanzen, freier Fall\n{AFAFAF}Taste 2 gibt Schub",
        COL_WHITE, cx, cy - 55.0, g + 2.0, 60.0);

    CreateDynamicMapIcon(cx, cy, g, 53, 0, -1, -1, -1, 400.0, MAPICON_GLOBAL);
    return 1;
}

/*
 * VIP-Lounge - kleinere Plattform mit umlaufendem Gelaender.
 */
static Map_BuildVipLounge()
{
    new Float:cx = MAP_LOUNGE_X, Float:cy = MAP_LOUNGE_Y;
    new Float:base = MAP_LOUNGE_Z - PLATFORM_MID_SURFACE;
    new Float:g = MAP_LOUNGE_Z;

    Map_Object(MODEL_PLATFORM_MID, cx, cy, base);
    Map_BarrierSquare(cx, cy, g, 40.0);

    // Zwei kleine Schanzen, damit man die Lounge auch fahrend verlassen kann
    Map_Object(MODEL_NEWRAMP, cx + 20.0, cy + 34.0, g + LIFT_NEWRAMP, 0.0, 0.0, 180.0);
    Map_Object(MODEL_NEWRAMP, cx - 20.0, cy + 34.0, g + LIFT_NEWRAMP, 0.0, 0.0, 180.0);

    CreateDynamic3DTextLabel("{FFD700}VIP-Lounge\n{FFFFFF}Nur fuer Unterstuetzer des Servers",
        COL_WHITE, cx, cy, g + 2.0, 50.0);
    return 1;
}

/*
 * Goldinsel - Plattform knapp ueber dem Meer.
 */
static Map_BuildGoldIsland()
{
    new Float:cx = MAP_ISLAND_X, Float:cy = MAP_ISLAND_Y;
    new Float:base = MAP_ISLAND_Z - PLATFORM_MID_SURFACE;
    new Float:g = MAP_ISLAND_Z;

    Map_Object(MODEL_PLATFORM_MID, cx, cy, base);
    Map_BarrierSquare(cx, cy, g, 40.0);

    CreateDynamic3DTextLabel("{FFD700}Gold-Insel\n{FFFFFF}Der ruhigste Ort in San Andreas",
        COL_WHITE, cx, cy, g + 2.0, 50.0);

    CreateDynamicMapIcon(cx, cy, g, 0, 0, -1, -1, -1, 400.0, MAPICON_GLOBAL);
    return 1;
}

/*
 * Umrandung der Derby-Arena. Sie entsteht ausschliesslich in der Derbywelt,
 * damit sie im normalen Freeroam niemandem im Weg steht.
 */
static Map_BuildDerbyArena()
{
    Map_BarrierSquare(DERBY_ARENA_X, DERBY_ARENA_Y, DERBY_ARENA_Z,
        DERBY_ARENA_HALFSIZE, WORLD_DERBY);

    // Hindernisse in der Mitte machen das Derby interessanter
    Map_Object(MODEL_TUBERAMP, DERBY_ARENA_X, DERBY_ARENA_Y,
        DERBY_ARENA_Z + LIFT_TUBERAMP, 0.0, 0.0, 0.0, WORLD_DERBY);
    Map_Object(MODEL_KICKRAMP_SMALL, DERBY_ARENA_X + 20.0, DERBY_ARENA_Y,
        DERBY_ARENA_Z + LIFT_KICKRAMP_SMALL, 0.0, 0.0, 90.0, WORLD_DERBY);
    Map_Object(MODEL_KICKRAMP_SMALL, DERBY_ARENA_X - 20.0, DERBY_ARENA_Y,
        DERBY_ARENA_Z + LIFT_KICKRAMP_SMALL, 0.0, 0.0, 270.0, WORLD_DERBY);
    return 1;
}

stock Map_OnGameModeInit()
{
    gMapObjectCount = 0;

    Map_BuildStuntPark();
    Map_BuildVipLounge();
    Map_BuildGoldIsland();
    Map_BuildDerbyArena();

    new msg[80];
    format(msg, sizeof msg, "Custom Map gebaut: %d dynamische Objekte.", gMapObjectCount);
    Log(msg);
    return gMapObjectCount;
}

stock Map_GetObjectCount()
{
    return gMapObjectCount;
}

// ---------------------------------------------------------------------------
// Kommandos
// ---------------------------------------------------------------------------
CMD:stuntpark(playerid, params[])
{
    #pragma unused params
    if (!Teleport_CanUse(playerid)) return 1;

    // Suedlich des grossen Loopings absetzen, mit Blick zur Anfahrt
    Teleport_Player(playerid, MAP_PARK_X, MAP_PARK_Y - 70.0, MAP_PARK_Z + 1.0, 0.0);
    SendSuccess(playerid, "Jebiga Stuntpark - der Looping liegt genau vor dir.");
    return 1;
}
ALIAS:park(stuntpark)
