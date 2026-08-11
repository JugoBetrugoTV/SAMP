/*
 * Jebiga-Gaming - Jobs
 *
 * Bezahlte Fahrten entlang fester Routen. Jeder Job stellt ein passendes
 * Fahrzeug, setzt Wegpunkte als Checkpoints und zahlt pro erreichtem Punkt.
 * Wer die ganze Route schafft, bekommt zusaetzlich eine Abschlusspraemie.
 *
 * Die Routen benutzen bewusst Koordinaten, die im Gamemode bereits als
 * Teleportziele belegt sind - damit steht fest, dass sie auf befahrbarem
 * Untergrund liegen.
 *
 * Technisch laeuft das ueber SetPlayerCheckpoint (den einfachen Kugel-
 * Checkpoint). Die Rennen nutzen SetPlayerRaceCheckpoint, also einen anderen
 * Callback - beide Systeme kommen sich nicht in die Quere.
 */

#if defined _jebiga_job_included
    #endinput
#endif
#define _jebiga_job_included

#define JOB_MAX_STOPS           (7)
#define JOB_CHECKPOINT_SIZE     (8.0)
#define JOB_ABORT_DISTANCE      (400.0)  // so weit darf man sich vom Auto entfernen

enum E_JOB
{
    jbName[32],
    jbDescription[72],
    jbVehicle,
    jbPayPerStop,
    jbBonus,
    jbStopCount,
    // Wegpunkte flach als x,y,z - Pawn-Enums koennen keine mehrdimensionalen
    // Felder aufnehmen. Der erste Punkt ist zugleich der Startplatz.
    Float:jbStops[JOB_MAX_STOPS * 3]
};

static const gJobs[][E_JOB] =
{
    {
        "Fernfahrer LS - SF", "Sattelzug von Los Santos nach San Fierro",
        515, 900, 12000, 6,
        {
            1685.0000, -2334.0000,  13.5500,   // LS Flughafen (Start)
             364.0000, -1877.0000,   4.5600,   // LS Strand
           -1300.0000,  -300.0000,  14.1500,   // Kuestenstrasse
           -1400.0000,  -300.0000,  14.1500,   // SF Flughafen
           -1980.0000,   137.0000,  27.6900,   // SF Zentrum
           -2438.0000,  2287.0000,   4.9800,   // SF Werft (Ziel)
            0.0, 0.0, 0.0
        }
    },
    {
        "Fernfahrer SF - LV", "Sattelzug quer durch die Wueste nach Las Venturas",
        514, 950, 13000, 6,
        {
           -2438.0000,  2287.0000,   4.9800,   // SF Werft (Start)
           -1471.0000,  2620.0000,  55.8300,   // El Quebrados
            -969.0000,  1966.0000,  60.4000,   // Sherman-Staudamm
            -215.0000,   980.0000,  19.3000,   // Fort Carson
             400.0000,  2536.0000,  16.5000,   // Verlassener Flughafen
            1685.0000,  1447.0000,  10.7700,   // LV Flughafen (Ziel)
            0.0, 0.0, 0.0
        }
    },
    {
        "Kurierdienst LS", "Schnelle Zustellungen quer durch Los Santos",
        482, 600, 6000, 6,
        {
            1481.0000, -1770.0000,  18.7900,   // Innenstadt (Start)
            2495.0000, -1687.0000,  13.5100,   // Grove Street
            1969.0000, -1092.0000,  25.9000,   // Glen Park
            2769.0000, -1618.0000,  12.1200,   // Stadion
            1256.0000, -1400.0000,  13.5500,   // ZomboTech
            1481.0000, -1770.0000,  18.7900,   // zurueck (Ziel)
            0.0, 0.0, 0.0
        }
    },
    {
        "Muellabfuhr LS", "Die Runde durch Los Santos, Halt fuer Halt",
        408, 500, 4500, 5,
        {
            2495.0000, -1687.0000,  13.5100,
            2201.0000, -1153.0000,  25.7300,
            1969.0000, -1092.0000,  25.9000,
             615.0000, -1230.0000,  18.2600,
             364.0000, -1877.0000,   4.5600,
            0.0, 0.0, 0.0,
            0.0, 0.0, 0.0
        }
    },
    {
        "Buslinie Las Venturas", "Rundkurs durch Las Venturas mit fuenf Halten",
        437, 550, 5500, 5,
        {
            2036.0000,  1546.0000,  10.8200,
            2189.0000,  1677.0000,  12.3700,
            1800.0000,  2200.0000,  10.8200,
            1266.0000,  2000.0000,  10.8200,
            1685.0000,  1447.0000,  10.7700,
            0.0, 0.0, 0.0,
            0.0, 0.0, 0.0
        }
    },
    {
        "Pilot LS - LV", "Frachtflug vom Flughafen Los Santos nach Las Venturas",
        519, 1400, 18000, 4,
        {
            1685.0000, -2334.0000,  13.5500,   // LS Flughafen (Start)
            -394.0000,  1493.0000,  65.6000,   // Big Ear
             400.0000,  2536.0000,  16.5000,   // Verlassener Flughafen
            1685.0000,  1447.0000,  10.7700,   // LV Flughafen (Ziel)
            0.0, 0.0, 0.0,
            0.0, 0.0, 0.0,
            0.0, 0.0, 0.0
        }
    }
};

static Job_GetStop(job, index, &Float:x, &Float:y, &Float:z)
{
    new base = index * 3;
    x = gJobs[job][jbStops][base];
    y = gJobs[job][jbStops][base + 1];
    z = gJobs[job][jbStops][base + 2];
    return 1;
}

stock Job_OnGameModeInit()
{
    new msg[64];
    format(msg, sizeof msg, "%d Jobrouten geladen.", sizeof gJobs);
    Log(msg);
    return 1;
}

static Job_SetCheckpoint(playerid)
{
    new job   = PlayerData[playerid][pJob];
    new stage = PlayerData[playerid][pJobStage];

    if (job == -1 || stage >= gJobs[job][jbStopCount]) return 0;

    new Float:x, Float:y, Float:z;
    Job_GetStop(job, stage, x, y, z);
    SetPlayerCheckpoint(playerid, x, y, z, JOB_CHECKPOINT_SIZE);
    return 1;
}

stock Job_Stop(playerid, const reason[], bool:paid = false)
{
    if (PlayerData[playerid][pJob] == -1) return 0;

    PlayerData[playerid][pJob] = -1;
    PlayerData[playerid][pJobStage] = 0;
    DisablePlayerCheckpoint(playerid);

    if (PlayerData[playerid][pJobVehicle] != INVALID_VEHICLE_ID)
    {
        DestroyVehicle(PlayerData[playerid][pJobVehicle]);
        PlayerData[playerid][pJobVehicle] = INVALID_VEHICLE_ID;
    }

    if (IsPlayerConnected(playerid) && reason[0] != EOS)
    {
        if (paid) SendSuccess(playerid, reason);
        else      SendInfo(playerid, reason);
    }
    return 1;
}

stock Job_Start(playerid, job)
{
    if (job < 0 || job >= sizeof gJobs) return 0;

    require(PlayerData[playerid][pMinigame] == MINIGAME_NONE, "Waehrend eines Minispiels kannst du nicht arbeiten.");

    Job_Stop(playerid, "");

    new Float:x, Float:y, Float:z;
    Job_GetStop(job, 0, x, y, z);

    new vehicleid = CreateVehicle(gJobs[job][jbVehicle], x + 4.0, y + 4.0, z + 1.0,
        0.0, random(256), random(256), -1);

    if (vehicleid == INVALID_VEHICLE_ID)
    {
        SendError(playerid, "Es konnte kein Arbeitsfahrzeug bereitgestellt werden.");
        return 0;
    }

    PlayerData[playerid][pJob]        = job;
    PlayerData[playerid][pJobStage]   = 0;
    PlayerData[playerid][pJobVehicle] = vehicleid;

    SetPlayerPos(playerid, x + 4.0, y + 4.0, z + 1.0);
    SetPlayerInterior(playerid, 0);
    SetPlayerVirtualWorld(playerid, WORLD_MAIN);
    PutPlayerInVehicle(playerid, vehicleid, 0);
    GivePlayerSpawnProtection(playerid, 3000);

    Job_SetCheckpoint(playerid);

    new pay[24], msg[192];
    FormatNumber(gJobs[job][jbPayPerStop], pay, sizeof pay);
    format(msg, sizeof msg, "Job angenommen: %s%s%s. %d Halte, $%s je Halt.",
        EC_WHITE, gJobs[job][jbName], EC_GREEN, gJobs[job][jbStopCount], pay);
    SendSuccess(playerid, msg);
    SendInfo(playerid, "Folge den Checkpoints. Abbrechen mit " EC_WHITE "/quitjob");
    return 1;
}

stock Job_OnPlayerEnterCheckpoint(playerid)
{
    new job = PlayerData[playerid][pJob];
    if (job == -1) return 0;

    // Zu Fuss zaehlt der Halt nicht - der Transport gehoert ins Fahrzeug
    if (GetPlayerVehicleID(playerid) != PlayerData[playerid][pJobVehicle])
    {
        SendError(playerid, "Du musst dafuer in deinem Arbeitsfahrzeug sitzen.");
        return 1;
    }

    PlayerData[playerid][pJobStage]++;
    GivePlayerMoney(playerid, gJobs[job][jbPayPerStop]);
    PlayerData[playerid][pJobEarnings] += gJobs[job][jbPayPerStop];

    new pay[24], msg[176];
    FormatNumber(gJobs[job][jbPayPerStop], pay, sizeof pay);

    if (PlayerData[playerid][pJobStage] >= gJobs[job][jbStopCount])
    {
        new bonus = gJobs[job][jbBonus];
        GivePlayerMoney(playerid, bonus);
        PlayerData[playerid][pJobEarnings] += bonus;
        PlayerData[playerid][pJobsDone]++;
        PlayerData[playerid][pScore] += 3;
        SetPlayerScore(playerid, PlayerData[playerid][pScore]);

        new bonusText[24];
        FormatNumber(bonus, bonusText, sizeof bonusText);
        format(msg, sizeof msg, "Route abgeschlossen! $%s Lohn plus $%s Praemie.", pay, bonusText);
        Job_Stop(playerid, msg, true);
        return 1;
    }

    format(msg, sizeof msg, "Halt %d von %d erledigt - $%s erhalten.",
        PlayerData[playerid][pJobStage], gJobs[job][jbStopCount], pay);
    SendSuccess(playerid, msg);

    Job_SetCheckpoint(playerid);
    return 1;
}

/*
 * Bricht den Job ab, wenn sich jemand zu weit vom Arbeitsfahrzeug entfernt -
 * sonst liesse sich die Route zu Fuss oder per Teleport abkuerzen.
 */
stock Job_Tick()
{
    foreach_player(i)
    {
        if (PlayerData[i][pJob] == -1) continue;
        if (PlayerData[i][pJobVehicle] == INVALID_VEHICLE_ID) continue;

        new Float:x, Float:y, Float:z;
        GetVehiclePos(PlayerData[i][pJobVehicle], x, y, z);

        if (GetPlayerDistanceFromPoint(i, x, y, z) > JOB_ABORT_DISTANCE)
        {
            Job_Stop(i, "Job abgebrochen - du hast dein Fahrzeug zurueckgelassen.");
        }
    }
    return 1;
}

stock Job_OnPlayerDisconnect(playerid)
{
    return Job_Stop(playerid, "");
}

stock Job_ShowList(playerid)
{
    new list[1024], entry[176], pay[24], bonus[24];

    for (new i = 0; i < sizeof gJobs; i++)
    {
        FormatNumber(gJobs[i][jbPayPerStop], pay, sizeof pay);
        FormatNumber(gJobs[i][jbBonus], bonus, sizeof bonus);

        format(entry, sizeof entry, "%s%s  %s%d Halte, $%s je Halt, $%s Praemie\n",
            EC_WHITE, gJobs[i][jbName], EC_GREY, gJobs[i][jbStopCount], pay, bonus);
        strcat(list, entry);
    }

    return ShowPlayerDialog(playerid, DIALOG_JOB_LIST, DIALOG_STYLE_LIST,
        "Jobs", list, "Annehmen", "Abbrechen");
}

stock Job_OnDialogResponse(playerid, dialogid, response, listitem)
{
    if (dialogid != DIALOG_JOB_LIST) return 0;
    if (!response) return 1;

    Job_Start(playerid, listitem);
    return 1;
}

// ---------------------------------------------------------------------------
// Kommandos
// ---------------------------------------------------------------------------
CMD:jobs(playerid, params[])
{
    #pragma unused params
    return Job_ShowList(playerid);
}
ALIAS:job(jobs)

CMD:quitjob(playerid, params[])
{
    #pragma unused params
    require(PlayerData[playerid][pJob] != -1, "Du hast gerade keinen Job.");

    Job_Stop(playerid, "Job abgebrochen. Der bereits verdiente Lohn bleibt dir.");
    return 1;
}
