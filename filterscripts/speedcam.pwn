/*
 * Jebiga-Gaming - Filterscript: Blitzer
 *
 * Feste Radarfallen an mehreren Strassen. Wer zu schnell durchfaehrt, zahlt.
 *
 * Damit daraus kein Aergernis wird: es gibt eine Kulanzgrenze, eine Sperrzeit
 * je Spieler und Blitzer, und die Strafe ist gedeckelt. Wer kein Geld hat,
 * bekommt trotzdem nur eine Meldung statt eines negativen Kontostands.
 */

#include <a_samp>

#define CAM_CHECK_INTERVAL      (900)    // ms
#define CAM_RADIUS              (18.0)
#define CAM_SPEED_LIMIT         (90)     // km/h
#define CAM_TOLERANCE           (10)     // km/h Kulanz
#define CAM_FINE_PER_KMH        (60)
#define CAM_FINE_MAX            (12000)
#define CAM_COOLDOWN            (30)     // Sekunden je Spieler und Blitzer
#define CAM_COLOR               0xE67E22FF

enum E_CAMERA
{
    caName[32],
    Float:caX,
    Float:caY,
    Float:caZ
};

static const gCameras[][E_CAMERA] =
{
    {"Los Santos - Innenstadt",  1481.0000, -1700.0000,  13.5000},
    {"Los Santos - Strandweg",    364.0000, -1800.0000,   4.5000},
    {"Los Santos - Flughafen",   1685.0000, -2280.0000,  13.5000},
    {"San Fierro - Zentrum",    -1980.0000,   200.0000,  27.6000},
    {"San Fierro - Bruecke",    -2707.0000,  1380.0000,   7.1000},
    {"Las Venturas - Strip",     2036.0000,  1480.0000,  10.8000},
    {"Las Venturas - Flughafen", 1685.0000,  1380.0000,  10.7000},
    {"Fort Carson",              -215.0000,  1040.0000,  19.3000}
};

// Letzte Auszahlung je Spieler und Blitzer (gettime), verhindert Dauerfeuer
static gLastFine[MAX_PLAYERS][sizeof gCameras];

public OnFilterScriptInit()
{
    for (new p = 0; p < MAX_PLAYERS; p++)
    {
        for (new c = 0; c < sizeof gCameras; c++) gLastFine[p][c] = 0;
    }

    SetTimer("Cam_Check", CAM_CHECK_INTERVAL, true);

    printf("[Blitzer] %d Radarfallen aktiv, Limit %d km/h.", sizeof gCameras, CAM_SPEED_LIMIT);
    return 1;
}

public OnPlayerConnect(playerid)
{
    for (new c = 0; c < sizeof gCameras; c++) gLastFine[playerid][c] = 0;
    return 1;
}

/*
 * Geschwindigkeit in km/h. SA-MP liefert die Geschwindigkeit in Einheiten pro
 * Tick; der Faktor 180 rechnet das naeherungsweise um.
 */
static Cam_GetSpeed(vehicleid)
{
    new Float:vx, Float:vy, Float:vz;
    GetVehicleVelocity(vehicleid, vx, vy, vz);
    return floatround(VectorSize(vx, vy, vz) * 180.0);
}

forward Cam_Check();
public Cam_Check()
{
    new maxPlayers = GetPlayerPoolSize(), now = gettime();

    for (new p = 0; p <= maxPlayers; p++)
    {
        if (!IsPlayerConnected(p) || IsPlayerNPC(p)) continue;
        if (GetPlayerState(p) != PLAYER_STATE_DRIVER) continue;

        new vehicleid = GetPlayerVehicleID(p);
        if (!vehicleid) continue;

        new speed = Cam_GetSpeed(vehicleid);
        if (speed <= CAM_SPEED_LIMIT + CAM_TOLERANCE) continue;

        for (new c = 0; c < sizeof gCameras; c++)
        {
            if (!IsPlayerInRangeOfPoint(p, CAM_RADIUS, gCameras[c][caX], gCameras[c][caY], gCameras[c][caZ])) continue;
            if (now - gLastFine[p][c] < CAM_COOLDOWN) continue;

            gLastFine[p][c] = now;

            new over = speed - CAM_SPEED_LIMIT;
            new fine = over * CAM_FINE_PER_KMH;
            if (fine > CAM_FINE_MAX) fine = CAM_FINE_MAX;

            new msg[176];

            if (GetPlayerMoney(p) < fine)
            {
                format(msg, sizeof msg, "[BLITZER] %s: %d km/h bei erlaubten %d. Du kannst nicht zahlen - Verwarnung.",
                    gCameras[c][caName], speed, CAM_SPEED_LIMIT);
                SendClientMessage(p, CAM_COLOR, msg);
                break;
            }

            GivePlayerMoney(p, -fine);

            format(msg, sizeof msg, "[BLITZER] %s: %d km/h bei erlaubten %d - $%d Bussgeld.",
                gCameras[c][caName], speed, CAM_SPEED_LIMIT, fine);
            SendClientMessage(p, CAM_COLOR, msg);
            PlayerPlaySound(p, 1058, 0.0, 0.0, 0.0);
            break;
        }
    }
    return 1;
}

public OnPlayerCommandText(playerid, cmdtext[])
{
    if (strcmp(cmdtext, "/blitzer", true) != 0) return 0;

    new body[768], entry[80];
    format(body, sizeof body, "{FFFFFF}Erlaubt sind {F1C40F}%d km/h{FFFFFF}, Kulanz %d km/h.\n\n", CAM_SPEED_LIMIT, CAM_TOLERANCE);

    for (new c = 0; c < sizeof gCameras; c++)
    {
        format(entry, sizeof entry, "{AFAFAF}%s\n", gCameras[c][caName]);
        strcat(body, entry);
    }
    strcat(body, "\n{AFAFAF}Das Bussgeld richtet sich nach der Ueberschreitung.");

    ShowPlayerDialog(playerid, 9300, DIALOG_STYLE_MSGBOX, "Blitzer", body, "Schliessen", "");
    return 1;
}
