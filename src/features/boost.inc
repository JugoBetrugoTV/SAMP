/*
 * UIF Freeroam - Speedboost
 *
 * Das Markenzeichen von UIF: wer im Fahrzeug die Taste 2 spammt, bekommt bei
 * jedem Druck einen Schub nach vorne. Umgesetzt ueber KEY_SUBMISSION - das ist
 * die Taste, auf der GTA San Andreas standardmaessig die "Sub-Mission" liegt,
 * also die 2 in der Zahlenreihe.
 *
 * Damit daraus kein Dauerflug wird, haengt der Schub an einem Energievorrat:
 * jeder Druck kostet, der Vorrat fuellt sich im Sekundentakt wieder auf. So
 * bleibt der typische Ablauf erhalten - kurzer heftiger Spam-Burst, danach
 * muss man den Vorrat nachladen lassen.
 */

#if defined _uif_boost_included
    #endinput
#endif
#define _uif_boost_included

#define BOOST_MAX_ENERGY        (100)
#define BOOST_COST              (8)      // Energie je Schub
#define BOOST_REGEN             (12)     // Energie je Sekunde
#define BOOST_REGEN_VIP         (18)
#define BOOST_MIN_INTERVAL      (110)    // ms, begrenzt reines Autofire
#define BOOST_POWER             (0.28)   // Geschwindigkeitszuwachs je Schub
#define BOOST_POWER_VIP         (0.34)
#define BOOST_MAX_SPEED         (1.70)   // ~305 km/h, darueber kein Schub mehr
#define BOOST_LIFT              (0.02)   // minimaler Auftrieb gegen Bodenreibung

/*
 * Fluggeraete sind ausgenommen - die haben ihren Schub bereits.
 */
static bool:Boost_IsAllowedModel(model)
{
    switch (model)
    {
        // Flugzeuge und Helikopter
        case 417, 425, 447, 460, 469, 476, 487, 488, 497, 511, 512, 513,
             519, 520, 548, 553, 563, 577, 592, 593: return false;

        // Ferngesteuerte Modelle und Schienenfahrzeuge
        case 441, 464, 465, 501, 564, 594, 449, 537, 538, 569, 590: return false;
    }
    return true;
}

stock Boost_OnPlayerConnect(playerid)
{
    PlayerData[playerid][pBoostEnergy]   = BOOST_MAX_ENERGY;
    PlayerData[playerid][pBoostLastTick] = 0;
    PlayerData[playerid][pBoostCount]    = 0;
    return 1;
}

/*
 * Fuellt den Energievorrat aller Spieler auf. Laeuft im Sekundentakt.
 */
stock Boost_Tick()
{
    foreach_player(i)
    {
        if (!IsLoggedIn(i)) continue;
        if (PlayerData[i][pBoostEnergy] >= BOOST_MAX_ENERGY) continue;

        new regen = IsVip(i) ? BOOST_REGEN_VIP : BOOST_REGEN;

        PlayerData[i][pBoostEnergy] += regen;
        if (PlayerData[i][pBoostEnergy] > BOOST_MAX_ENERGY)
        {
            PlayerData[i][pBoostEnergy] = BOOST_MAX_ENERGY;
        }
    }
    return 1;
}

/*
 * Ein einzelner Schub. Gibt 0 zurueck, wenn er nicht ausgeloest wurde -
 * der Aufrufer entscheidet, ob das eine Meldung wert ist.
 */
stock Boost_Apply(playerid)
{
    if (!IsPlayerInAnyVehicleAsDriver(playerid)) return 0;

    new vehicleid = GetPlayerVehicleID(playerid);
    if (!Boost_IsAllowedModel(GetVehicleModel(vehicleid))) return 0;

    // Im Rennen waere der Boost ein unfairer Vorteil gegenueber Mitfahrern,
    // die ihn nicht kennen. Im Derby ist er fuer alle gleich verfuegbar.
    if (PlayerData[playerid][pMinigame] == MINIGAME_RACE) return 0;

    new tick = GetTickCount();
    if (tick - PlayerData[playerid][pBoostLastTick] < BOOST_MIN_INTERVAL) return 0;

    if (PlayerData[playerid][pBoostEnergy] < BOOST_COST) return -1;

    new Float:vx, Float:vy, Float:vz;
    GetVehicleVelocity(vehicleid, vx, vy, vz);

    // Bei Hoechstgeschwindigkeit verpufft der Schub, statt endlos zu stapeln
    if (VectorSize(vx, vy, vz) >= BOOST_MAX_SPEED) return 0;

    new Float:angle;
    GetVehicleZAngle(vehicleid, angle);

    // Blickrichtung des Fahrzeugs: bei Winkel 0 zeigt die Front nach +Y
    new Float:power = IsVip(playerid) ? BOOST_POWER_VIP : BOOST_POWER;
    new Float:fx = -floatsin(angle, degrees) * power;
    new Float:fy =  floatcos(angle, degrees) * power;

    SetVehicleVelocity(vehicleid, vx + fx, vy + fy, vz + BOOST_LIFT);

    PlayerData[playerid][pBoostEnergy] -= BOOST_COST;
    PlayerData[playerid][pBoostLastTick] = tick;
    PlayerData[playerid][pBoostCount]++;

    PlayerPlaySound(playerid, 1133, 0.0, 0.0, 0.0);
    return 1;
}

/*
 * Aus OnPlayerKeyStateChange. Der Callback feuert je Tastendruck, deshalb
 * ergibt Spammen genau die Kette einzelner Schuebe, die den Server ausmacht.
 */
stock Boost_OnPlayerKeyStateChange(playerid, newkeys, oldkeys)
{
    if (!(newkeys & KEY_SUBMISSION) || (oldkeys & KEY_SUBMISSION)) return 0;

    new result = Boost_Apply(playerid);

    // Leerer Vorrat: einmalig eine kurze Rueckmeldung, kein Chatspam
    if (result == -1)
    {
        GameTextForPlayer(playerid, "~r~Boost leer", 700, 6);
        return 0;
    }
    return result;
}

// ---------------------------------------------------------------------------
// Kommandos
// ---------------------------------------------------------------------------
CMD:boost(playerid, params[])
{
    #pragma unused params

    new body[640];
    format(body, sizeof body,
        "%sIm Fahrzeug die Taste %s2%s druecken - jeder Druck gibt einen Schub\n\
        nach vorne. Spammen verkettet die Schuebe zu einer Beschleunigung.\n\n\
        %sEnergie:%s %d von %d\n\
        %sKosten je Schub:%s %d\n\
        %sAufladung:%s %d pro Sekunde%s\n\
        %sSchuebe diese Sitzung:%s %d\n\n\
        %sFluggeraete, Schienenfahrzeuge und ferngesteuerte Modelle sind\n\
        ausgenommen. Im Rennen ist der Boost deaktiviert.",
        EC_WHITE, EC_YELLOW, EC_WHITE,
        EC_GREY, EC_WHITE, PlayerData[playerid][pBoostEnergy], BOOST_MAX_ENERGY,
        EC_GREY, EC_WHITE, BOOST_COST,
        EC_GREY, EC_WHITE, IsVip(playerid) ? BOOST_REGEN_VIP : BOOST_REGEN,
        IsVip(playerid) ? " (VIP-Bonus)" : "",
        EC_GREY, EC_WHITE, PlayerData[playerid][pBoostCount],
        EC_GREY);

    ShowPlayerDialog(playerid, DIALOG_HELP, DIALOG_STYLE_MSGBOX,
        "Speedboost", body, "Schliessen", "");
    return 1;
}
ALIAS:nitro(boost)
