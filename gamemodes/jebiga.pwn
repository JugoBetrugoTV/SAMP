/*
 * =============================================================================
 *  Jebiga-Gaming
 *  Ein Freeroam-Gamemode fuer SA-MP 0.3.7 / open.mp
 * =============================================================================
 *
 *  Aufbau
 *  ------
 *  Pawn kennt keine echten Module: jeder Callback darf pro Skript nur einmal
 *  existieren. Deshalb liegt hier die einzige Definition aller SA-MP-Callbacks,
 *  und jedes Modul stellt eigene Funktionen (Modul_OnPlayerConnect, ...) bereit,
 *  die von hier aus aufgerufen werden. So bleibt der Code getrennt, ohne dass
 *  sich Callbacks gegenseitig ueberschreiben.
 *
 *  Der Server braucht weder Plugins noch externe Includes - Kommandoparser,
 *  Parameter-Parsing und Datenspeicher sind Teil dieses Projekts.
 */

#include <a_samp>

// Streamer (Incognito): haelt Objekte, Textlabels und Map-Icons jenseits der
// festen SA-MP-Grenzen. Die Custom Map baut vollstaendig darauf auf.
#include <streamer>

// MySQL (pBlueG): optionaler Datenspeicher. Ist in scriptfiles/mysql.ini
// nichts aktiviert, laeuft der Server unveraendert auf Dateien weiter.
#include <a_mysql>

// Die Dialogtexte arbeiten mit grossen lokalen Puffern (bis 2 KB). Der
// Pawn-Standardstack von 4096 Zellen reicht dafuer nicht - der Compiler
// schaetzt den Spitzenbedarf auf rund 8800 Zellen.
#pragma dynamic 16384

// --- Referenzdaten (erzeugt von tools/gen_data.py) --------------------------
#include "src/data/skins.inc"
#include "src/data/weapons.inc"
#include "src/data/components.inc"
#include "src/data/interiors.inc"
#include "src/data/animations.inc"

// --- Kern -------------------------------------------------------------------
#include "src/core/config.inc"
#include "src/core/macros.inc"
#include "src/core/ini.inc"
#include "src/core/util.inc"
#include "src/core/db.inc"
#include "src/core/player.inc"
#include "src/core/account.inc"
// Crews stehen vor dem Chat, weil die Chatzeile das Crewkuerzel anzeigt.
#include "src/features/crew.inc"
#include "src/core/chat.inc"

// --- Features ---------------------------------------------------------------
#include "src/features/teleport.inc"
#include "src/features/boost.inc"
#include "src/features/weapon.inc"
#include "src/features/vehicle.inc"
#include "src/features/stunt.inc"
#include "src/features/house.inc"
#include "src/features/map.inc"
#include "src/features/skinshop.inc"
#include "src/features/tuning.inc"
#include "src/features/interior.inc"
#include "src/features/job.inc"
#include "src/features/bank.inc"
#include "src/features/achievement.inc"
#include "src/features/extras.inc"
#include "src/core/hud.inc"

// --- Team & Sicherheit ------------------------------------------------------
#include "src/admin/admin.inc"
#include "src/admin/vip.inc"
#include "src/admin/anticheat.inc"

// --- Minispiele -------------------------------------------------------------
#include "src/minigames/race.inc"
#include "src/minigames/derby.inc"
#include "src/minigames/arena.inc"

// --- Allgemeine Kommandos ---------------------------------------------------
#include "src/core/commands.inc"

// =============================================================================
//  Gamemode
// =============================================================================
main()
{
    print("\n----------------------------------------");
    print("  " SERVER_NAME);
    print("  Version " SERVER_VERSION);
    print("----------------------------------------\n");
}

public OnGameModeInit()
{
    SetGameModeText("Freeroam");
    ShowPlayerMarkers(PLAYER_MARKERS_MODE_GLOBAL);
    ShowNameTags(1);
    SetNameTagDrawDistance(70.0);
    EnableStuntBonusForAll(0);      // Stuntpunkte vergibt das eigene Stuntmodul
    DisableInteriorEnterExits();
    SetWeather(2);
    SetWorldTime(12);
    UsePlayerPedAnims();

    // Auswahlskins der Spawnauswahl
    for (new i = 0; i < sizeof gSpawnSkins; i++)
    {
        AddPlayerClass(gSpawnSkins[i], 1958.3783, 1343.1572, 15.3746, 270.1425, 0, 0, 0, 0, 0, 0);
    }

    // Vor allen Modulen, die Daten laden: entscheidet ueber MySQL oder Dateien
    DB_Connect();

    Teleport_OnGameModeInit();
    Map_OnGameModeInit();
    Vehicle_OnGameModeInit();
    House_OnGameModeInit();
    Race_OnGameModeInit();
    Derby_OnGameModeInit();
    Arena_OnGameModeInit();
    Stunt_OnGameModeInit();
    Crew_OnGameModeInit();
    Tuning_OnGameModeInit();
    Interior_OnGameModeInit();
    Job_OnGameModeInit();
    Hud_OnGameModeInit();

    SetTimer("OnServerSecond", 1000, true);
    SetTimer("OnServerMinute", 60000, true);

    Log("Gamemode " SERVER_NAME " " SERVER_VERSION " gestartet.");
    return 1;
}

public OnGameModeExit()
{
    foreach_player(i)
    {
        if (IsLoggedIn(i)) Account_Save(i);
    }
    House_SaveAll();
    Crew_SaveAll();
    DB_Close();
    Log("Gamemode wird beendet - alle Daten gespeichert.");
    return 1;
}

// =============================================================================
//  Spieler
// =============================================================================
public OnPlayerConnect(playerid)
{
    ResetPlayerData(playerid);

    new msg[144];
    format(msg, sizeof msg, "%s%s%s (ID %d) verbindet sich...",
        EC_GREY, GetName(playerid), EC_GREY, playerid);
    SendClientMessageToAll(COL_GREY, msg);

    SendClientMessage(playerid, COL_WHITE, " ");
    format(msg, sizeof msg, "%sWillkommen auf %s%s", EC_WHITE, EC_GOLD, SERVER_NAME);
    SendClientMessage(playerid, COL_WHITE, msg);
    format(msg, sizeof msg, "%sWebseite: %s%s  %s|  Discord: %s%s",
        EC_GREY, EC_CYAN, SERVER_WEBSITE, EC_GREY, EC_CYAN, SERVER_DISCORD);
    SendClientMessage(playerid, COL_WHITE, msg);

    // Prueft erst den Ban und laedt danach den Account - beides je nach
    // Speicherweg sofort oder ueber einen Abfrage-Callback.
    Admin_BeginBanCheck(playerid);
    AntiCheat_OnPlayerConnect(playerid);
    Boost_OnPlayerConnect(playerid);
    Hud_OnPlayerConnect(playerid);
    return 1;
}

public OnPlayerDisconnect(playerid, reason)
{
    static const reasons[][] = { "Verbindung verloren", "Verlassen", "Kick/Ban" };

    if (IsLoggedIn(playerid))
    {
        new msg[144];
        format(msg, sizeof msg, "%s hat den Server verlassen (%s).",
            GetName(playerid), reasons[reason]);
        SendClientMessageToAll(COL_GREY, msg);
    }

    Race_OnPlayerDisconnect(playerid);
    Derby_OnPlayerDisconnect(playerid);
    Arena_OnPlayerDisconnect(playerid);
    Job_OnPlayerDisconnect(playerid);
    Vehicle_OnPlayerDisconnect(playerid);
    Stunt_OnPlayerDisconnect(playerid);
    Account_OnDisconnect(playerid);
    ResetPlayerData(playerid);
    return 1;
}

public OnPlayerRequestClass(playerid, classid)
{
    // Ohne Login keine Skinauswahl - die Kamera bleibt auf der Willkommensszene.
    SetPlayerPos(playerid, 1958.3783, 1343.1572, 15.3746);
    SetPlayerCameraPos(playerid, 1958.3783, 1343.1572, 20.0);
    SetPlayerCameraLookAt(playerid, 1958.3783, 1343.1572, 15.3746);

    if (!IsLoggedIn(playerid)) return 0;

    PlayerData[playerid][pSkin] = gSpawnSkins[classid % sizeof gSpawnSkins];
    SetPlayerSkin(playerid, PlayerData[playerid][pSkin]);
    return 1;
}

public OnPlayerSpawn(playerid)
{
    if (!IsLoggedIn(playerid))
    {
        Kick(playerid);
        return 0;
    }

    PlayerData[playerid][pSpawned] = true;
    SetPlayerSkin(playerid, PlayerData[playerid][pSkin]);
    SetPlayerColor(playerid, GetPlayerRankColor(playerid));
    SetPlayerHealth(playerid, 100.0);
    GivePlayerSpawnProtection(playerid);

    // Nach Jailstrafe oder Minispiel nicht in die Freiheit spawnen
    if (IsPlayerJailed(playerid))       { Admin_PutInJail(playerid); return 1; }
    if (Derby_IsParticipant(playerid))  { Derby_RespawnParticipant(playerid); return 1; }
    if (Arena_OnPlayerSpawn(playerid))  return 1;

    Teleport_SpawnPlayer(playerid);
    AntiCheat_OnPlayerSpawn(playerid);
    return 1;
}

public OnPlayerDeath(playerid, killerid, reason)
{
    PlayerData[playerid][pDeaths]++;
    SetPlayerScore(playerid, PlayerData[playerid][pScore]);

    if (killerid != INVALID_PLAYER_ID)
    {
        PlayerData[killerid][pKills]++;
        PlayerData[killerid][pScore]++;
        SetPlayerScore(killerid, PlayerData[killerid][pScore]);
        GivePlayerMoney(killerid, 500);
    }

    Race_OnPlayerDeath(playerid);
    Derby_OnPlayerDeath(playerid, killerid);
    Arena_OnPlayerDeath(playerid, killerid);
    return 1;
}

public OnPlayerText(playerid, text[])
{
    return Chat_OnPlayerText(playerid, text);
}

public OnPlayerUpdate(playerid)
{
    if (!IsLoggedIn(playerid)) return 0;

    if (!AntiCheat_OnPlayerUpdate(playerid)) return 0;
    Stunt_OnPlayerUpdate(playerid);
    return 1;
}

public OnPlayerStateChange(playerid, newstate, oldstate)
{
    Vehicle_OnPlayerStateChange(playerid, newstate, oldstate);
    Hud_OnPlayerStateChange(playerid, newstate);
    MarkActivity(playerid);
    return 1;
}

public OnPlayerKeyStateChange(playerid, newkeys, oldkeys)
{
    MarkActivity(playerid);
    Boost_OnPlayerKeyStateChange(playerid, newkeys, oldkeys);
    Vehicle_OnPlayerKeyStateChange(playerid, newkeys, oldkeys);
    return 1;
}

public OnPlayerEnterRaceCheckpoint(playerid)
{
    Race_OnEnterRaceCheckpoint(playerid);
    return 1;
}

public OnPlayerPickUpPickup(playerid, pickupid)
{
    House_OnPlayerPickUpPickup(playerid, pickupid);
    return 1;
}

public OnVehicleDeath(vehicleid, killerid)
{
    Derby_OnVehicleDeath(vehicleid);
    return 1;
}

public OnPlayerTakeDamage(playerid, issuerid, Float:amount, weaponid, bodypart)
{
    // Spawnschutz: frisch gespawnte Spieler sind kurz unverwundbar.
    if (IsPlayerSpawnProtected(playerid))
    {
        new Float:health;
        GetPlayerHealth(playerid, health);
        SetPlayerHealth(playerid, health + amount);

        if (issuerid != INVALID_PLAYER_ID)
        {
            SendError(issuerid, "Dieser Spieler steht noch unter Spawnschutz.");
        }
        return 1;
    }
    if (PlayerData[playerid][pGodMode])
    {
        SetPlayerHealth(playerid, 100.0);
    }
    return 1;
}

// =============================================================================
//  Dialoge
// =============================================================================
public OnDialogResponse(playerid, dialogid, response, listitem, inputtext[])
{
    if (Account_OnDialogResponse(playerid, dialogid, response, inputtext))  return 1;
    if (Teleport_OnDialogResponse(playerid, dialogid, response, listitem))  return 1;
    if (Weapon_OnDialogResponse(playerid, dialogid, response, listitem))    return 1;
    if (Stunt_OnDialogResponse(playerid, dialogid, response, listitem))     return 1;
    if (House_OnDialogResponse(playerid, dialogid, response, listitem))     return 1;
    if (Vip_OnDialogResponse(playerid, dialogid, response, listitem))       return 1;
    if (Race_OnDialogResponse(playerid, dialogid, response, listitem))      return 1;
    if (Derby_OnDialogResponse(playerid, dialogid, response, listitem))     return 1;
    if (Arena_OnDialogResponse(playerid, dialogid, response, listitem))     return 1;
    if (Crew_OnDialogResponse(playerid, dialogid, response, listitem))      return 1;
    if (Extras_OnDialogResponse(playerid, dialogid, response, listitem))    return 1;
    if (Skin_OnDialogResponse(playerid, dialogid, response, listitem))      return 1;
    if (Tuning_OnDialogResponse(playerid, dialogid, response, listitem))    return 1;
    if (Interior_OnDialogResponse(playerid, dialogid, response, listitem))  return 1;
    if (Job_OnDialogResponse(playerid, dialogid, response, listitem))       return 1;
    return 0;
}

// =============================================================================
//  Kommando-Dispatcher
// =============================================================================
public OnPlayerCommandText(playerid, cmdtext[])
{
    new cmd[33], params[128], i = 1, pos = 0;

    // Kommandonamen einlesen und in Kleinbuchstaben normalisieren
    while (cmdtext[i] != EOS && cmdtext[i] != ' ' && pos < sizeof(cmd) - 1)
    {
        new c = cmdtext[i++];
        if (c >= 'A' && c <= 'Z') c += 32;
        cmd[pos++] = c;
    }
    cmd[pos] = EOS;

    if (pos == 0)
    {
        SendError(playerid, "Unbekanntes Kommando. Tippe /help fuer eine Uebersicht.");
        return 1;
    }

    while (cmdtext[i] == ' ') i++;
    strmid(params, cmdtext, i, strlen(cmdtext), sizeof params);

    // Vor dem Login sind keine Kommandos erlaubt
    if (!IsLoggedIn(playerid))
    {
        SendError(playerid, "Du musst dich erst einloggen.");
        return 1;
    }
    if (IsPlayerJailed(playerid) && !Admin_IsJailCommand(cmd))
    {
        SendError(playerid, "Im Jail sind keine Kommandos erlaubt.");
        return 1;
    }

    MarkActivity(playerid);

    new func[36];
    format(func, sizeof func, "cmd_%s", cmd);

    if (!CallLocalFunction(func, "is", playerid, params))
    {
        new msg[128];
        format(msg, sizeof msg, "Unbekanntes Kommando '/%s'. Tippe /help fuer eine Uebersicht.", cmd);
        SendError(playerid, msg);
    }
    return 1;
}

// =============================================================================
//  Timer
// =============================================================================
/*
 * Wird vom Accountsystem nach erfolgreichem Login aufgerufen.
 */
public OnPlayerLoggedIn(playerid)
{
    Crew_OnPlayerLogin(playerid);
    Achievement_Check(playerid);
    return 1;
}

public OnPlayerEnterCheckpoint(playerid)
{
    Job_OnPlayerEnterCheckpoint(playerid);
    return 1;
}

forward OnServerSecond();
public OnServerSecond()
{
    Race_Tick();
    Derby_Tick();
    Admin_Tick();
    Vehicle_Tick();
    Boost_Tick();
    Job_Tick();
    return 1;
}

forward OnServerMinute();
public OnServerMinute()
{
    new now = gettime();

    foreach_player(i)
    {
        if (!IsLoggedIn(i)) continue;

        // Regelmaessig speichern, damit ein Absturz keine Fortschritte kostet
        Account_Save(i);
        Achievement_Check(i);

        // AFK-Erkennung
        if (!PlayerData[i][pAfk] && (now - PlayerData[i][pLastActivity]) >= AFK_THRESHOLD)
        {
            PlayerData[i][pAfk] = true;

            new msg[96];
            format(msg, sizeof msg, "%s ist jetzt AFK.", GetName(i));
            SendClientMessageToAll(COL_GREY, msg);
        }
    }
    Bank_Tick();
    Announce_Rotate();
    return 1;
}
