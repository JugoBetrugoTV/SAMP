/*
 * Jebiga-Gaming - HUD
 *
 * Zwei Textdraws am unteren Bildschirmrand, sichtbar sobald man in einem
 * Fahrzeug sitzt: Geschwindigkeit samt Modellname und darunter der
 * Boostvorrat als Balken.
 *
 * Die Textdraws gehoeren jeweils einem Spieler (CreatePlayerTextDraw), damit
 * jeder seine eigenen Werte sieht. Aktualisiert wird nur, wer tatsaechlich
 * faehrt - bei vollem Server spart das den Grossteil der Arbeit.
 */

#if defined _jebiga_hud_included
    #endinput
#endif
#define _jebiga_hud_included

#define HUD_UPDATE_INTERVAL     (400)   // ms
#define HUD_BAR_SEGMENTS        (12)

static PlayerText:gHudSpeed[MAX_PLAYERS];
static PlayerText:gHudBoost[MAX_PLAYERS];
static bool:gHudVisible[MAX_PLAYERS];

stock Hud_OnPlayerConnect(playerid)
{
    gHudVisible[playerid] = false;

    gHudSpeed[playerid] = CreatePlayerTextDraw(playerid, 498.0, 380.0, "_");
    PlayerTextDrawFont(playerid, gHudSpeed[playerid], 2);
    PlayerTextDrawLetterSize(playerid, gHudSpeed[playerid], 0.28, 1.35);
    PlayerTextDrawColor(playerid, gHudSpeed[playerid], 0xFFFFFFFF);
    PlayerTextDrawSetShadow(playerid, gHudSpeed[playerid], 0);
    PlayerTextDrawSetOutline(playerid, gHudSpeed[playerid], 1);
    PlayerTextDrawBackgroundColor(playerid, gHudSpeed[playerid], 0x000000CC);
    PlayerTextDrawAlignment(playerid, gHudSpeed[playerid], 3);   // rechtsbuendig

    gHudBoost[playerid] = CreatePlayerTextDraw(playerid, 498.0, 396.0, "_");
    PlayerTextDrawFont(playerid, gHudBoost[playerid], 1);
    PlayerTextDrawLetterSize(playerid, gHudBoost[playerid], 0.26, 1.10);
    PlayerTextDrawColor(playerid, gHudBoost[playerid], 0x1ABC9CFF);
    PlayerTextDrawSetShadow(playerid, gHudBoost[playerid], 0);
    PlayerTextDrawSetOutline(playerid, gHudBoost[playerid], 1);
    PlayerTextDrawBackgroundColor(playerid, gHudBoost[playerid], 0x000000CC);
    PlayerTextDrawAlignment(playerid, gHudBoost[playerid], 3);
    return 1;
}

stock Hud_Hide(playerid)
{
    if (!gHudVisible[playerid]) return 0;

    PlayerTextDrawHide(playerid, gHudSpeed[playerid]);
    PlayerTextDrawHide(playerid, gHudBoost[playerid]);
    gHudVisible[playerid] = false;
    return 1;
}

stock Hud_OnPlayerStateChange(playerid, newstate)
{
    if (newstate == PLAYER_STATE_DRIVER || newstate == PLAYER_STATE_PASSENGER) return 1;

    Hud_Hide(playerid);
    return 1;
}

/*
 * Baut den Boostbalken als Kette von Strichen. Zeichen statt Balkengrafik,
 * weil sich der String eines Textdraws billig ersetzen laesst.
 */
static Hud_FormatBar(energy, dest[], len = sizeof dest)
{
    new filled = (energy * HUD_BAR_SEGMENTS) / BOOST_MAX_ENERGY;
    if (filled < 0) filled = 0;
    if (filled > HUD_BAR_SEGMENTS) filled = HUD_BAR_SEGMENTS;

    // Voller Teil in Farbe, Rest gedaempft
    new bar[64];
    bar[0] = EOS;

    for (new i = 0; i < HUD_BAR_SEGMENTS && strlen(bar) < sizeof(bar) - 2; i++)
    {
        strcat(bar, (i < filled) ? "I" : ".");
    }

    format(dest, len, "%sBOOST %s%s",
        EC_GREY, (filled > HUD_BAR_SEGMENTS / 3) ? EC_CYAN : EC_RED, bar);
    return strlen(dest);
}

/*
 * Wird alle HUD_UPDATE_INTERVAL Millisekunden aufgerufen.
 */
forward Hud_Update();
public Hud_Update()
{
    new Float:vx, Float:vy, Float:vz, text[128], bar[80];

    foreach_player(i)
    {
        if (!IsLoggedIn(i)) continue;

        new pstate = GetPlayerState(i);
        if (pstate != PLAYER_STATE_DRIVER && pstate != PLAYER_STATE_PASSENGER)
        {
            Hud_Hide(i);
            continue;
        }

        new vehicleid = GetPlayerVehicleID(i);
        if (!vehicleid) { Hud_Hide(i); continue; }

        GetVehicleVelocity(vehicleid, vx, vy, vz);

        // SA-MP liefert die Geschwindigkeit in Einheiten pro Tick;
        // der Faktor 180 rechnet das naeherungsweise in km/h um.
        new speed = floatround(VectorSize(vx, vy, vz) * 180.0);

        new Float:health;
        GetVehicleHealth(vehicleid, health);

        format(text, sizeof text, "%s%s%s  %s%d %skm/h",
            EC_GREY, Vehicle_GetModelName(GetVehicleModel(vehicleid)),
            (health < 400.0) ? EC_RED : EC_GREY,
            EC_WHITE, speed, EC_GREY);
        PlayerTextDrawSetString(i, gHudSpeed[i], text);

        Hud_FormatBar(PlayerData[i][pBoostEnergy], bar, sizeof bar);
        PlayerTextDrawSetString(i, gHudBoost[i], bar);

        if (!gHudVisible[i])
        {
            PlayerTextDrawShow(i, gHudSpeed[i]);
            PlayerTextDrawShow(i, gHudBoost[i]);
            gHudVisible[i] = true;
        }
    }
    return 1;
}

stock Hud_OnGameModeInit()
{
    SetTimer("Hud_Update", HUD_UPDATE_INTERVAL, true);
    return 1;
}
