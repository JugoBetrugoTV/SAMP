/*
 * Jebiga-Gaming - Anti-Cheat
 *
 * Bewusst konservative Heuristiken: lieber ein Cheat unentdeckt als ein
 * ehrlicher Spieler gekickt. Erkannt werden nur Zustaende, die im normalen
 * Spiel nicht vorkommen koennen (Werte ueber dem Maximum, Sprungweiten weit
 * jenseits jeder Fortbewegung). Alles andere meldet das System an das Team,
 * statt selbst zu bestrafen.
 *
 * Wichtig: Geld und Punktestand liegen bei SA-MP auf dem Server. Ein Client
 * kann sie nicht manipulieren, deshalb gibt es dafuer bewusst keine Pruefung.
 */

#if defined _jebiga_anticheat_included
    #endinput
#endif
#define _jebiga_anticheat_included

#define AC_MAX_HEALTH           (100.0)
#define AC_MAX_ARMOUR           (100.0)
#define AC_MAX_FOOT_STEP        (60.0)   // Meter zwischen zwei Updates zu Fuss
#define AC_MAX_VEHICLE_SPEED    (600.0)  // km/h, klar jenseits jedes Fahrzeugs
#define AC_WARN_LIMIT           (5)      // danach automatischer Kick

stock AntiCheat_OnPlayerConnect(playerid)
{
    PlayerData[playerid][pWarnings]       = 0;
    PlayerData[playerid][pLastUpdateTick] = GetTickCount();
    return 1;
}

stock AntiCheat_OnPlayerSpawn(playerid)
{
    GetPlayerPos(playerid,
        PlayerData[playerid][pLastPos][0],
        PlayerData[playerid][pLastPos][1],
        PlayerData[playerid][pLastPos][2]);

    PlayerData[playerid][pLastUpdateTick] = GetTickCount();
    return 1;
}

/*
 * Zaehlt einen Verdachtsfall, meldet ihn dem Team und kickt bei Ueberschreiten
 * des Limits.
 */
static AntiCheat_Flag(playerid, const detail[])
{
    PlayerData[playerid][pWarnings]++;

    new msg[192];
    format(msg, sizeof msg, "[AC] %s(%d): %s (%d/%d)",
        GetName(playerid), playerid, detail,
        PlayerData[playerid][pWarnings], AC_WARN_LIMIT);
    SendMessageToAdmins(ADMIN_MODERATOR, msg);
    Log(msg);

    if (PlayerData[playerid][pWarnings] >= AC_WARN_LIMIT)
    {
        format(msg, sizeof msg, "%s wurde vom Anti-Cheat entfernt (%s).", GetName(playerid), detail);
        Admin_Broadcast(msg);
        KickDelayed(playerid);
    }
    return 1;
}

/*
 * Geschwindigkeitspruefung im Fahrzeug. Nur ein Meldewert - eingegriffen wird
 * nicht, weil Rampen, Explosionen und Gefaelle kurzzeitig hohe Werte erzeugen.
 */
stock AntiCheat_CheckVehicleSpeed(playerid)
{
    new vehicleid = GetPlayerVehicleID(playerid);
    if (!vehicleid) return 0;

    new Float:vx, Float:vy, Float:vz;
    GetVehicleVelocity(vehicleid, vx, vy, vz);

    // Umrechnung der SA-MP-Geschwindigkeit in km/h
    new Float:speed = VectorSize(vx, vy, vz) * 180.0;

    if (speed > AC_MAX_VEHICLE_SPEED)
    {
        new detail[96];
        format(detail, sizeof detail, "Fahrzeuggeschwindigkeit %.0f km/h", speed);
        AntiCheat_Flag(playerid, detail);
    }
    return 1;
}

/*
 * Wird aus OnPlayerUpdate aufgerufen. Rueckgabe 0 verwirft das Update.
 */
stock AntiCheat_OnPlayerUpdate(playerid)
{
    // Team im Dienst und Godmode sind vom Anti-Cheat ausgenommen
    if (AdminLevel(playerid) >= ADMIN_ADMIN) return 1;

    new Float:health, Float:armour;
    GetPlayerHealth(playerid, health);
    GetPlayerArmour(playerid, armour);

    // Werte oberhalb des Maximums kann das Spiel selbst nie erzeugen
    if (health > AC_MAX_HEALTH + 0.5)
    {
        SetPlayerHealth(playerid, AC_MAX_HEALTH);
        AntiCheat_Flag(playerid, "Gesundheit ueber Maximum");
        return 1;
    }
    if (armour > AC_MAX_ARMOUR + 0.5)
    {
        SetPlayerArmour(playerid, AC_MAX_ARMOUR);
        AntiCheat_Flag(playerid, "Ruestung ueber Maximum");
        return 1;
    }

    new tick = GetTickCount();
    new elapsed = tick - PlayerData[playerid][pLastUpdateTick];
    PlayerData[playerid][pLastUpdateTick] = tick;

    new Float:x, Float:y, Float:z;
    GetPlayerPos(playerid, x, y, z);

    new Float:previous[3];
    previous[0] = PlayerData[playerid][pLastPos][0];
    previous[1] = PlayerData[playerid][pLastPos][1];
    previous[2] = PlayerData[playerid][pLastPos][2];

    PlayerData[playerid][pLastPos][0] = x;
    PlayerData[playerid][pLastPos][1] = y;
    PlayerData[playerid][pLastPos][2] = z;

    // Nach Teleports, Spawns und langen Pausen keine Distanzpruefung
    if (IsPlayerSpawnProtected(playerid))    return 1;
    if (elapsed <= 0 || elapsed > 1500)      return 1;
    if (previous[0] == 0.0 && previous[1] == 0.0) return 1;
    if (GetPlayerState(playerid) != PLAYER_STATE_ONFOOT)
    {
        AntiCheat_CheckVehicleSpeed(playerid);
        return 1;
    }
    // Fallschirm und Jetpack erzeugen ebenfalls grosse Spruenge
    switch (GetPlayerSpecialAction(playerid))
    {
        case SPECIAL_ACTION_USEJETPACK: return 1;
    }

    new Float:distance = VectorSize(x - previous[0], y - previous[1], z - previous[2]);

    if (distance > AC_MAX_FOOT_STEP)
    {
        new detail[96];
        format(detail, sizeof detail, "Positionssprung von %.0fm in %dms", distance, elapsed);
        AntiCheat_Flag(playerid, detail);

        SetPlayerPos(playerid, previous[0], previous[1], previous[2]);
        PlayerData[playerid][pLastPos][0] = previous[0];
        PlayerData[playerid][pLastPos][1] = previous[1];
        PlayerData[playerid][pLastPos][2] = previous[2];
        return 0;
    }
    return 1;
}
