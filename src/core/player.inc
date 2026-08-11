/*
 * UIF Freeroam - Spielerdaten
 *
 * Zentrale Datenstruktur pro Spieler. Alles was gespeichert wird, steht im
 * oberen Block; der untere Block haelt reine Laufzeitwerte, die beim Verbinden
 * zurueckgesetzt werden.
 */

#if defined _uif_player_included
    #endinput
#endif
#define _uif_player_included

enum E_PLAYER
{
    // --- Persistente Daten (werden in scriptfiles/accounts/ gespeichert) ---
    pPassword[MAX_PASSWORD_LEN],
    pSalt[MAX_SALT_LEN],
    pRegDate[24],
    pLastIP[16],
    pAdminLevel,
    pVipLevel,
    pScore,
    pMoney,
    pKills,
    pDeaths,
    pPlayTime,              // Gesamtspielzeit in Sekunden
    pSkin,
    pStuntPoints,
    pRaceWins,
    pDerbyWins,
    pHouse,                 // Index des eigenen Hauses, -1 = keins
    bool:pVehicleDamage,    // /vd - sichtbarer Fahrzeugschaden

    // --- Laufzeitdaten ---
    bool:pRegistered,
    bool:pLoggedIn,
    bool:pSpawned,
    bool:pFrozen,
    bool:pGodMode,
    bool:pAdminDuty,
    bool:pAfk,
    pLoginTries,
    pSessionStart,          // gettime() beim Login
    pSpawnProtectUntil,     // GetTickCount() bis Spawnschutz endet
    pMutedUntil,            // gettime() bis Stummschaltung endet
    pJailUntil,             // gettime() bis Jail endet
    pLastChatTick,
    pSpamStrikes,
    pSpectating,            // beobachtete Spieler-ID oder INVALID_PLAYER_ID
    pLastActivity,          // gettime() der letzten Aktion (AFK-Erkennung)
    pPmPartner,             // fuer /r (Antwort auf letzte PM)
    pDialogPage,            // Seitenindex fuer mehrseitige Dialoge
    pDialogContext,         // freier Kontextwert des zuletzt geoeffneten Dialogs

    // --- Anti-Cheat ---
    Float:pLastPos[3],
    pLastUpdateTick,
    pWarnings,

    // --- Speedboost ---
    pBoostEnergy,           // 0..BOOST_MAX_ENERGY
    pBoostLastTick,
    pBoostCount,            // Schuebe der laufenden Sitzung (Statistik)

    // --- Stunts ---
    bool:pStuntInAir,
    pStuntStartTick,
    Float:pStuntStartPos[3],
    pStuntObject,           // per /ramp gesetzte Rampe

    // --- Minispiele ---
    pMinigame,              // MINIGAME_NONE / MINIGAME_RACE / MINIGAME_DERBY
    pRaceId,
    pRaceCheckpoint,
    pRaceStartTime,
    pMinigameVehicle,
    pArena,                 // Index der DM-Arena, -1 = keine
    pArenaKills,
    pDuelSlot,              // belegter Duellplatz, -1 = keiner
    pDuelInviteFrom,        // offene Einladung, INVALID_PLAYER_ID = keine
    pDuelInviteWeapon,

    // --- Crew ---
    pCrewName[24],          // gespeicherter Crewname ("" = keine Crew)
    pCrew,                  // zur Laufzeit aufgeloester Crewindex, -1 = keine
    pCrewInviteFrom
};

new PlayerData[MAX_PLAYERS][E_PLAYER];

// Auswahlskins der Spawnauswahl (Pfeiltasten in der Klassenauswahl)
new const gSpawnSkins[] =
{
    26, 29, 30, 46, 60, 72, 92, 101, 105, 116,
    125, 134, 154, 170, 190, 202, 212, 230, 240, 250
};

// ---------------------------------------------------------------------------
// Kurzformen fuer haeufige Zugriffe
// ---------------------------------------------------------------------------
#define IsLoggedIn(%0)      (PlayerData[%0][pLoggedIn])
#define AdminLevel(%0)      (PlayerData[%0][pAdminLevel])
#define VipLevel(%0)        (PlayerData[%0][pVipLevel])
#define IsAdmin(%0,%1)      (PlayerData[%0][pAdminLevel] >= %1)
#define IsVip(%0)           (PlayerData[%0][pVipLevel] > VIP_NONE)

/*
 * Setzt alle Spielerdaten auf den Ausgangszustand. Wird beim Verbinden und
 * beim Trennen aufgerufen, damit eine neue Spieler-ID keine Reste erbt.
 */
stock ResetPlayerData(playerid)
{
    new empty[E_PLAYER];
    PlayerData[playerid] = empty;

    PlayerData[playerid][pHouse]         = -1;
    PlayerData[playerid][pSpectating]    = INVALID_PLAYER_ID;
    PlayerData[playerid][pPmPartner]     = INVALID_PLAYER_ID;
    PlayerData[playerid][pSkin]          = 26;
    PlayerData[playerid][pVehicleDamage] = true;
    PlayerData[playerid][pLastActivity]  = gettime();
    PlayerData[playerid][pMinigame]      = MINIGAME_NONE;
    PlayerData[playerid][pRaceId]        = -1;
    PlayerData[playerid][pMinigameVehicle] = INVALID_VEHICLE_ID;
    PlayerData[playerid][pStuntObject]   = INVALID_OBJECT_ID;
    PlayerData[playerid][pArena]         = -1;
    PlayerData[playerid][pDuelSlot]      = -1;
    PlayerData[playerid][pDuelInviteFrom] = INVALID_PLAYER_ID;
    PlayerData[playerid][pCrew]          = -1;
    PlayerData[playerid][pCrewInviteFrom] = -1;
    return 1;
}

/*
 * Bezeichnung des Admin-Rangs fuer Anzeigen.
 */
stock GetAdminRankName(level)
{
    new name[16];
    switch (level)
    {
        case ADMIN_MODERATOR: name = "Moderator";
        case ADMIN_ADMIN:     name = "Admin";
        case ADMIN_SENIOR:    name = "Senior Admin";
        case ADMIN_HEAD:      name = "Head Admin";
        case ADMIN_OWNER:     name = "Owner";
        default:              name = "Spieler";
    }
    return name;
}

stock GetVipRankName(level)
{
    new name[16];
    switch (level)
    {
        case VIP_BRONZE: name = "Bronze VIP";
        case VIP_SILVER: name = "Silber VIP";
        case VIP_GOLD:   name = "Gold VIP";
        default:         name = "-";
    }
    return name;
}

/*
 * Chatfarbe nach Rang: Admins orange, VIPs gold, alle anderen weiss.
 */
stock GetPlayerRankColor(playerid)
{
    if (AdminLevel(playerid) > ADMIN_NONE) return COL_ADMIN;
    if (IsVip(playerid))                   return COL_VIP;
    return COL_WHITE;
}

/*
 * Jailstatus liegt bewusst im Kern: mehrere Module fragen ihn ab, und so
 * entsteht keine Abhaengigkeit auf das erst spaeter eingebundene Adminmodul.
 */
stock bool:IsPlayerJailed(playerid)
{
    return (PlayerData[playerid][pJailUntil] > gettime());
}

stock bool:IsPlayerSpawnProtected(playerid)
{
    return (GetTickCount() < PlayerData[playerid][pSpawnProtectUntil]);
}

stock GivePlayerSpawnProtection(playerid, duration = SPAWN_PROTECTION_TIME)
{
    PlayerData[playerid][pSpawnProtectUntil] = GetTickCount() + duration;
    SetPlayerHealth(playerid, 100.0);
    return 1;
}

/*
 * Gesamtspielzeit inklusive der laufenden Sitzung.
 */
stock GetTotalPlayTime(playerid)
{
    new total = PlayerData[playerid][pPlayTime];
    if (PlayerData[playerid][pSessionStart] > 0)
    {
        total += gettime() - PlayerData[playerid][pSessionStart];
    }
    return total;
}

stock MarkActivity(playerid)
{
    PlayerData[playerid][pLastActivity] = gettime();
    if (PlayerData[playerid][pAfk])
    {
        PlayerData[playerid][pAfk] = false;

        new msg[96];
        format(msg, sizeof msg, "%s ist zurueck (nicht mehr AFK).", GetName(playerid));
        SendClientMessageToAll(COL_GREY, msg);
    }
    return 1;
}
