/*
 * Jebiga-Gaming - Administration
 *
 * Fuenf Teamstufen von Moderator bis Owner. Jedes Kommando prueft die
 * benoetigte Stufe selbst; alle eingreifenden Aktionen landen im Logfile und
 * im Adminchat, damit Entscheidungen nachvollziehbar bleiben.
 */

#if defined _jebiga_admin_included
    #endinput
#endif
#define _jebiga_admin_included

// Zelle im LSPD
#define JAIL_X                  (264.6000)
#define JAIL_Y                  (77.5000)
#define JAIL_Z                  (1001.0000)
#define JAIL_INTERIOR           (6)

// ---------------------------------------------------------------------------
// Bans
// ---------------------------------------------------------------------------

// Bans liegen namensbasiert in scriptfiles/bans.ini: <name>=<Grund> | <Admin> | <Datum>
static Admin_BanKey(const name[], dest[], len = sizeof dest)
{
    new i = 0;
    while (name[i] != EOS && i < len - 1)
    {
        new c = name[i];
        dest[i] = (c >= 'A' && c <= 'Z') ? c + 32 : c;
        i++;
    }
    dest[i] = EOS;
    return i;
}

stock bool:Admin_IsBanned(const name[], reason[], reasonLen = sizeof reason)
{
    new key[MAX_PLAYER_NAME + 1];
    Admin_BanKey(name, key, sizeof key);
    return INI_GetString(PATH_BANS, key, reason, reasonLen);
}

/*
 * Haengt einen Ban an bans.ini an. Die Datei wird nicht neu geschrieben,
 * damit parallele Bans sich nicht gegenseitig ueberschreiben.
 */
stock Admin_AddBan(const name[], const reason[], const adminName[])
{
    new key[MAX_PLAYER_NAME + 1];
    Admin_BanKey(name, key, sizeof key);

    new stamp[32];
    GetTimestamp(stamp, sizeof stamp);

    new line[256];
    format(line, sizeof line, "%s=%s | von %s | %s\r\n", key, reason, adminName, stamp);

    new File:handle = fopen(PATH_BANS, io_append);
    if (!handle) return 0;

    fwrite(handle, line);
    fclose(handle);
    return 1;
}

/*
 * Entfernt einen Ban, indem die Datei ohne die betreffende Zeile neu
 * geschrieben wird.
 */
stock Admin_RemoveBan(const name[])
{
    if (!fexist(PATH_BANS)) return 0;

    new key[MAX_PLAYER_NAME + 1];
    Admin_BanKey(name, key, sizeof key);

    new File:source = fopen(PATH_BANS, io_read);
    if (!source) return 0;

    // Verbleibende Zeilen zuerst sammeln, dann Datei ersetzen
    new buffer[8192], line[INI_MAX_LINE], removed = 0;
    buffer[0] = EOS;

    while (fread(source, line, sizeof line))
    {
        new trimmed[INI_MAX_LINE];
        strmid(trimmed, line, 0, strlen(line), sizeof trimmed);
        INI_Trim(trimmed);
        if (trimmed[0] == EOS) continue;

        new sep = strfind(trimmed, "=");
        if (sep != -1)
        {
            new entry[MAX_PLAYER_NAME + 1];
            strmid(entry, trimmed, 0, (sep < sizeof entry) ? sep : sizeof entry - 1, sizeof entry);
            INI_Trim(entry);

            if (!strcmp(entry, key, true)) { removed++; continue; }
        }

        if (strlen(buffer) + strlen(trimmed) + 2 < sizeof buffer)
        {
            strcat(buffer, trimmed);
            strcat(buffer, "\r\n");
        }
    }
    fclose(source);

    if (!removed) return 0;

    new File:target = fopen(PATH_BANS, io_write);
    if (!target) return 0;

    fwrite(target, buffer);
    fclose(target);
    return removed;
}

/*
 * Prueft beim Verbinden, ob der Name gesperrt ist.
 */
stock bool:Admin_OnPlayerConnect(playerid)
{
    new reason[128];
    if (!Admin_IsBanned(GetName(playerid), reason, sizeof reason)) return false;

    new msg[192];
    format(msg, sizeof msg, "Du bist von diesem Server gebannt. Grund: %s", reason);
    SendError(playerid, msg);
    SendInfo(playerid, "Entbannungsantrag: " SERVER_WEBSITE);
    KickDelayed(playerid, 1000);

    format(msg, sizeof msg, "Gebannter Spieler %s hat versucht zu verbinden.", GetName(playerid));
    Log(msg);
    return true;
}

// ---------------------------------------------------------------------------
// Jail
// ---------------------------------------------------------------------------
stock Admin_PutInJail(playerid)
{
    SetPlayerPos(playerid, JAIL_X, JAIL_Y, JAIL_Z);
    SetPlayerInterior(playerid, JAIL_INTERIOR);
    SetPlayerVirtualWorld(playerid, WORLD_JAIL);
    ResetPlayerWeapons(playerid);
    SetCameraBehindPlayer(playerid);
    return 1;
}

stock Admin_ReleaseFromJail(playerid)
{
    PlayerData[playerid][pJailUntil] = 0;

    SetPlayerInterior(playerid, 0);
    SetPlayerVirtualWorld(playerid, 0);
    Teleport_SpawnPlayer(playerid);
    GivePlayerSpawnProtection(playerid);

    SendSuccess(playerid, "Du wurdest aus dem Jail entlassen.");
    return 1;
}

/*
 * Kommandos, die auch im Jail erlaubt sind.
 */
stock bool:Admin_IsJailCommand(const cmd[])
{
    static const allowed[][] =
    {
        "help", "cmds", "rules", "stats", "report", "pm", "r", "a",
        "jailtime", "credits", "players", "kill"
    };

    for (new i = 0; i < sizeof allowed; i++)
    {
        if (!strcmp(cmd, allowed[i], true)) return true;
    }
    return false;
}

/*
 * Laeuft im Sekundentakt: entlaesst Spieler, deren Jailzeit abgelaufen ist.
 */
stock Admin_Tick()
{
    new now = gettime();

    foreach_player(i)
    {
        if (!IsLoggedIn(i)) continue;

        if (PlayerData[i][pJailUntil] > 0 && now >= PlayerData[i][pJailUntil])
        {
            Admin_ReleaseFromJail(i);
        }
        else if (PlayerData[i][pJailUntil] > 0)
        {
            new left[32], msg[64];
            FormatDuration(PlayerData[i][pJailUntil] - now, left, sizeof left);
            format(msg, sizeof msg, "~r~JAIL~n~~w~%s", left);
            GameTextForPlayer(i, msg, 1100, 3);
        }
    }
    return 1;
}

// ---------------------------------------------------------------------------
// Gemeinsame Pruefungen
// ---------------------------------------------------------------------------

/*
 * Prueft die Teamstufe und meldet fehlende Rechte.
 */
stock bool:Admin_Require(playerid, level)
{
    if (AdminLevel(playerid) >= level) return true;

    new msg[112];
    format(msg, sizeof msg, "Dafuer brauchst du mindestens Stufe %d (%s).",
        level, GetAdminRankName(level));
    SendError(playerid, msg);
    return false;
}

/*
 * Verhindert Aktionen gegen ranghoehere oder gleichrangige Teammitglieder.
 */
stock bool:Admin_CanActOn(playerid, targetid)
{
    if (playerid == targetid) return true;

    if (AdminLevel(targetid) >= AdminLevel(playerid))
    {
        SendError(playerid, "Gegen dieses Teammitglied kannst du nicht vorgehen.");
        return false;
    }
    return true;
}

/*
 * Meldet eine Adminaktion an alle Spieler und schreibt sie ins Log.
 */
stock Admin_Broadcast(const message[])
{
    new buf[192];
    format(buf, sizeof buf, "%s[ADMIN] %s%s", EC_ORANGE, EC_WHITE, message);
    SendClientMessageToAll(COL_WHITE, buf);
    Log(message);
    return 1;
}

// ---------------------------------------------------------------------------
// Kommandos - Stufe 1 (Moderator)
// ---------------------------------------------------------------------------
CMD:acmds(playerid, params[])
{
    #pragma unused params
    require(Admin_Require(playerid, ADMIN_MODERATOR), "Du gehoerst nicht zum Team.");

    new body[1400];

    strcat(body, "{F1C40F}Stufe 1 - Moderator\n{FFFFFF}\
        /kick /mute /unmute /warn /goto /gethere /spec /specoff /freeze /unfreeze /slap /ip /a\n\n");

    if (AdminLevel(playerid) >= ADMIN_ADMIN)
    {
        strcat(body, "{F1C40F}Stufe 2 - Admin\n{FFFFFF}\
            /ban /jail /unjail /sethealth /setarmour /akill /explode /god /setskin\n\n");
    }
    if (AdminLevel(playerid) >= ADMIN_SENIOR)
    {
        strcat(body, "{F1C40F}Stufe 3 - Senior Admin\n{FFFFFF}\
            /unban /setmoney /setscore /ann /setweather /settime\n\n");
    }
    if (AdminLevel(playerid) >= ADMIN_HEAD)
    {
        strcat(body, "{F1C40F}Stufe 4 - Head Admin\n{FFFFFF}\
            /setvip /aduty\n\n");
    }
    if (AdminLevel(playerid) >= ADMIN_OWNER)
    {
        strcat(body, "{F1C40F}Stufe 5 - Owner\n{FFFFFF}\
            /setlevel /gmx\n\n");
    }

    ShowPlayerDialog(playerid, DIALOG_ADMIN_MENU, DIALOG_STYLE_MSGBOX,
        "Adminkommandos", body, "Schliessen", "");
    return 1;
}

CMD:kick(playerid, params[])
{
    require(Admin_Require(playerid, ADMIN_MODERATOR), "Keine Berechtigung.");
    require(Params_Count(params) >= 2, "Benutzung: /kick [Spieler] [Grund]");

    new arg[MAX_PLAYER_NAME + 1];
    Params_Get(params, 0, arg, sizeof arg);

    new target = ResolveTarget(playerid, arg);
    if (target == INVALID_PLAYER_ID) return 1;
    if (!Admin_CanActOn(playerid, target)) return 1;

    new reason[96];
    Params_Rest(params, 1, reason, sizeof reason);

    new msg[192];
    format(msg, sizeof msg, "%s hat %s vom Server geworfen. Grund: %s",
        GetName(playerid), GetName(target), reason);
    Admin_Broadcast(msg);

    KickDelayed(target);
    return 1;
}

CMD:mute(playerid, params[])
{
    require(Admin_Require(playerid, ADMIN_MODERATOR), "Keine Berechtigung.");
    require(Params_Count(params) >= 3, "Benutzung: /mute [Spieler] [Minuten] [Grund]");

    new arg[MAX_PLAYER_NAME + 1];
    Params_Get(params, 0, arg, sizeof arg);

    new target = ResolveTarget(playerid, arg);
    if (target == INVALID_PLAYER_ID) return 1;
    if (!Admin_CanActOn(playerid, target)) return 1;

    new minutes = Params_Int(params, 1);
    require(minutes >= 1 && minutes <= 1440, "Die Dauer muss zwischen 1 und 1440 Minuten liegen.");

    new reason[96];
    Params_Rest(params, 2, reason, sizeof reason);

    MutePlayer(target, minutes * 60);

    new msg[192];
    format(msg, sizeof msg, "%s hat %s fuer %d Minuten stummgeschaltet. Grund: %s",
        GetName(playerid), GetName(target), minutes, reason);
    Admin_Broadcast(msg);
    return 1;
}

CMD:unmute(playerid, params[])
{
    require(Admin_Require(playerid, ADMIN_MODERATOR), "Keine Berechtigung.");
    require(params[0] != EOS, "Benutzung: /unmute [Spieler]");

    new target = ResolveTarget(playerid, params);
    if (target == INVALID_PLAYER_ID) return 1;

    UnmutePlayer(target);

    new msg[144];
    format(msg, sizeof msg, "%s hat die Stummschaltung von %s aufgehoben.",
        GetName(playerid), GetName(target));
    Admin_Broadcast(msg);
    return 1;
}

CMD:warn(playerid, params[])
{
    require(Admin_Require(playerid, ADMIN_MODERATOR), "Keine Berechtigung.");
    require(Params_Count(params) >= 2, "Benutzung: /warn [Spieler] [Grund]");

    new arg[MAX_PLAYER_NAME + 1];
    Params_Get(params, 0, arg, sizeof arg);

    new target = ResolveTarget(playerid, arg);
    if (target == INVALID_PLAYER_ID) return 1;
    if (!Admin_CanActOn(playerid, target)) return 1;

    new reason[96];
    Params_Rest(params, 1, reason, sizeof reason);

    PlayerData[target][pWarnings]++;

    new msg[192];
    format(msg, sizeof msg, "%s hat %s verwarnt (%d/3). Grund: %s",
        GetName(playerid), GetName(target), PlayerData[target][pWarnings], reason);
    Admin_Broadcast(msg);

    // Drei Verwarnungen fuehren automatisch zum Kick
    if (PlayerData[target][pWarnings] >= 3)
    {
        format(msg, sizeof msg, "%s wurde nach drei Verwarnungen automatisch gekickt.", GetName(target));
        Admin_Broadcast(msg);
        KickDelayed(target);
    }
    return 1;
}

CMD:goto(playerid, params[])
{
    require(Admin_Require(playerid, ADMIN_MODERATOR), "Keine Berechtigung.");
    require(params[0] != EOS, "Benutzung: /goto [Spieler]");

    new target = ResolveTarget(playerid, params);
    if (target == INVALID_PLAYER_ID) return 1;
    require(target != playerid, "Du bist bereits bei dir selbst.");

    new Float:x, Float:y, Float:z;
    GetPlayerPos(target, x, y, z);
    Teleport_Player(playerid, x + 1.5, y + 1.5, z,
        0.0, GetPlayerInterior(target), GetPlayerVirtualWorld(target));

    new msg[96];
    format(msg, sizeof msg, "Du wurdest zu %s teleportiert.", GetName(target));
    SendSuccess(playerid, msg);
    return 1;
}

CMD:gethere(playerid, params[])
{
    require(Admin_Require(playerid, ADMIN_MODERATOR), "Keine Berechtigung.");
    require(params[0] != EOS, "Benutzung: /gethere [Spieler]");

    new target = ResolveTarget(playerid, params);
    if (target == INVALID_PLAYER_ID) return 1;
    require(target != playerid, "Du bist bereits hier.");

    new Float:x, Float:y, Float:z;
    GetPlayerPos(playerid, x, y, z);
    Teleport_Player(target, x + 1.5, y + 1.5, z,
        0.0, GetPlayerInterior(playerid), GetPlayerVirtualWorld(playerid));

    new msg[128];
    format(msg, sizeof msg, "%s hat dich zu sich geholt.", GetName(playerid));
    SendInfo(target, msg);

    format(msg, sizeof msg, "%s wurde zu dir teleportiert.", GetName(target));
    SendSuccess(playerid, msg);
    return 1;
}

CMD:slap(playerid, params[])
{
    require(Admin_Require(playerid, ADMIN_MODERATOR), "Keine Berechtigung.");
    require(params[0] != EOS, "Benutzung: /slap [Spieler]");

    new target = ResolveTarget(playerid, params);
    if (target == INVALID_PLAYER_ID) return 1;
    if (!Admin_CanActOn(playerid, target)) return 1;

    new Float:x, Float:y, Float:z, Float:health;
    GetPlayerPos(target, x, y, z);
    SetPlayerPos(target, x, y, z + 6.0);

    GetPlayerHealth(target, health);
    SetPlayerHealth(target, (health > 15.0) ? health - 15.0 : 1.0);
    PlayerPlaySound(target, 1130, x, y, z);

    new msg[144];
    format(msg, sizeof msg, "%s hat %s geohrfeigt.", GetName(playerid), GetName(target));
    Admin_Broadcast(msg);
    return 1;
}

CMD:freeze(playerid, params[])
{
    require(Admin_Require(playerid, ADMIN_MODERATOR), "Keine Berechtigung.");
    require(params[0] != EOS, "Benutzung: /freeze [Spieler]");

    new target = ResolveTarget(playerid, params);
    if (target == INVALID_PLAYER_ID) return 1;
    if (!Admin_CanActOn(playerid, target)) return 1;

    TogglePlayerControllable(target, false);
    PlayerData[target][pFrozen] = true;

    new msg[144];
    format(msg, sizeof msg, "%s hat %s eingefroren.", GetName(playerid), GetName(target));
    Admin_Broadcast(msg);
    return 1;
}

CMD:unfreeze(playerid, params[])
{
    require(Admin_Require(playerid, ADMIN_MODERATOR), "Keine Berechtigung.");
    require(params[0] != EOS, "Benutzung: /unfreeze [Spieler]");

    new target = ResolveTarget(playerid, params);
    if (target == INVALID_PLAYER_ID) return 1;

    TogglePlayerControllable(target, true);
    PlayerData[target][pFrozen] = false;

    new msg[144];
    format(msg, sizeof msg, "%s hat %s wieder freigegeben.", GetName(playerid), GetName(target));
    Admin_Broadcast(msg);
    return 1;
}

CMD:ip(playerid, params[])
{
    require(Admin_Require(playerid, ADMIN_MODERATOR), "Keine Berechtigung.");
    require(params[0] != EOS, "Benutzung: /ip [Spieler]");

    new target = ResolveTarget(playerid, params);
    if (target == INVALID_PLAYER_ID) return 1;

    new ip[16], msg[128];
    GetPlayerIp(target, ip, sizeof ip);
    format(msg, sizeof msg, "IP von %s(%d): %s", GetName(target), target, ip);
    SendInfo(playerid, msg);
    return 1;
}

CMD:spec(playerid, params[])
{
    require(Admin_Require(playerid, ADMIN_MODERATOR), "Keine Berechtigung.");
    require(params[0] != EOS, "Benutzung: /spec [Spieler]");

    new target = ResolveTarget(playerid, params);
    if (target == INVALID_PLAYER_ID) return 1;
    require(target != playerid, "Dich selbst kannst du nicht beobachten.");

    TogglePlayerSpectating(playerid, true);

    if (IsPlayerInAnyVehicle(target)) PlayerSpectateVehicle(playerid, GetPlayerVehicleID(target));
    else                              PlayerSpectatePlayer(playerid, target);

    SetPlayerInterior(playerid, GetPlayerInterior(target));
    SetPlayerVirtualWorld(playerid, GetPlayerVirtualWorld(target));
    PlayerData[playerid][pSpectating] = target;

    new msg[96];
    format(msg, sizeof msg, "Du beobachtest jetzt %s. Beenden mit /specoff.", GetName(target));
    SendInfo(playerid, msg);
    return 1;
}

CMD:specoff(playerid, params[])
{
    #pragma unused params
    require(Admin_Require(playerid, ADMIN_MODERATOR), "Keine Berechtigung.");
    require(PlayerData[playerid][pSpectating] != INVALID_PLAYER_ID, "Du beobachtest niemanden.");

    PlayerData[playerid][pSpectating] = INVALID_PLAYER_ID;
    TogglePlayerSpectating(playerid, false);
    SetPlayerInterior(playerid, 0);
    SetPlayerVirtualWorld(playerid, 0);
    SpawnPlayer(playerid);
    return 1;
}

// ---------------------------------------------------------------------------
// Kommandos - Stufe 2 (Admin)
// ---------------------------------------------------------------------------
CMD:ban(playerid, params[])
{
    require(Admin_Require(playerid, ADMIN_ADMIN), "Keine Berechtigung.");
    require(Params_Count(params) >= 2, "Benutzung: /ban [Spieler] [Grund]");

    new arg[MAX_PLAYER_NAME + 1];
    Params_Get(params, 0, arg, sizeof arg);

    new target = ResolveTarget(playerid, arg);
    if (target == INVALID_PLAYER_ID) return 1;
    if (!Admin_CanActOn(playerid, target)) return 1;

    new reason[96];
    Params_Rest(params, 1, reason, sizeof reason);

    Admin_AddBan(GetName(target), reason, GetName(playerid));

    new msg[192];
    format(msg, sizeof msg, "%s hat %s gebannt. Grund: %s",
        GetName(playerid), GetName(target), reason);
    Admin_Broadcast(msg);

    SendError(target, "Du wurdest von diesem Server gebannt.");
    SendInfo(target, "Entbannungsantrag: " SERVER_WEBSITE);
    KickDelayed(target, 800);
    return 1;
}

CMD:jail(playerid, params[])
{
    require(Admin_Require(playerid, ADMIN_ADMIN), "Keine Berechtigung.");
    require(Params_Count(params) >= 3, "Benutzung: /jail [Spieler] [Minuten] [Grund]");

    new arg[MAX_PLAYER_NAME + 1];
    Params_Get(params, 0, arg, sizeof arg);

    new target = ResolveTarget(playerid, arg);
    if (target == INVALID_PLAYER_ID) return 1;
    if (!Admin_CanActOn(playerid, target)) return 1;

    new minutes = Params_Int(params, 1);
    require(minutes >= 1 && minutes <= 720, "Die Dauer muss zwischen 1 und 720 Minuten liegen.");

    new reason[96];
    Params_Rest(params, 2, reason, sizeof reason);

    PlayerData[target][pJailUntil] = gettime() + (minutes * 60);
    Admin_PutInJail(target);

    new msg[192];
    format(msg, sizeof msg, "%s hat %s fuer %d Minuten ins Jail gesteckt. Grund: %s",
        GetName(playerid), GetName(target), minutes, reason);
    Admin_Broadcast(msg);
    return 1;
}

CMD:unjail(playerid, params[])
{
    require(Admin_Require(playerid, ADMIN_ADMIN), "Keine Berechtigung.");
    require(params[0] != EOS, "Benutzung: /unjail [Spieler]");

    new target = ResolveTarget(playerid, params);
    if (target == INVALID_PLAYER_ID) return 1;
    require(IsPlayerJailed(target), "Dieser Spieler sitzt nicht im Jail.");

    Admin_ReleaseFromJail(target);

    new msg[144];
    format(msg, sizeof msg, "%s hat %s vorzeitig aus dem Jail entlassen.",
        GetName(playerid), GetName(target));
    Admin_Broadcast(msg);
    return 1;
}

CMD:sethealth(playerid, params[])
{
    require(Admin_Require(playerid, ADMIN_ADMIN), "Keine Berechtigung.");
    require(Params_Count(params) >= 2, "Benutzung: /sethealth [Spieler] [0-100]");

    new arg[MAX_PLAYER_NAME + 1];
    Params_Get(params, 0, arg, sizeof arg);

    new target = ResolveTarget(playerid, arg);
    if (target == INVALID_PLAYER_ID) return 1;

    new value = Params_Int(params, 1);
    require(value >= 0 && value <= 100, "Der Wert muss zwischen 0 und 100 liegen.");

    SetPlayerHealth(target, float(value));

    new msg[144];
    format(msg, sizeof msg, "%s hat die Gesundheit von %s auf %d gesetzt.",
        GetName(playerid), GetName(target), value);
    SendMessageToAdmins(ADMIN_MODERATOR, msg);
    Log(msg);
    return 1;
}

CMD:setarmour(playerid, params[])
{
    require(Admin_Require(playerid, ADMIN_ADMIN), "Keine Berechtigung.");
    require(Params_Count(params) >= 2, "Benutzung: /setarmour [Spieler] [0-100]");

    new arg[MAX_PLAYER_NAME + 1];
    Params_Get(params, 0, arg, sizeof arg);

    new target = ResolveTarget(playerid, arg);
    if (target == INVALID_PLAYER_ID) return 1;

    new value = Params_Int(params, 1);
    require(value >= 0 && value <= 100, "Der Wert muss zwischen 0 und 100 liegen.");

    SetPlayerArmour(target, float(value));

    new msg[144];
    format(msg, sizeof msg, "%s hat die Ruestung von %s auf %d gesetzt.",
        GetName(playerid), GetName(target), value);
    SendMessageToAdmins(ADMIN_MODERATOR, msg);
    return 1;
}

CMD:akill(playerid, params[])
{
    require(Admin_Require(playerid, ADMIN_ADMIN), "Keine Berechtigung.");
    require(params[0] != EOS, "Benutzung: /akill [Spieler]");

    new target = ResolveTarget(playerid, params);
    if (target == INVALID_PLAYER_ID) return 1;
    if (!Admin_CanActOn(playerid, target)) return 1;

    SetPlayerHealth(target, 0.0);

    new msg[144];
    format(msg, sizeof msg, "%s hat %s getoetet.", GetName(playerid), GetName(target));
    Admin_Broadcast(msg);
    return 1;
}

CMD:explode(playerid, params[])
{
    require(Admin_Require(playerid, ADMIN_ADMIN), "Keine Berechtigung.");
    require(params[0] != EOS, "Benutzung: /explode [Spieler]");

    new target = ResolveTarget(playerid, params);
    if (target == INVALID_PLAYER_ID) return 1;
    if (!Admin_CanActOn(playerid, target)) return 1;

    new Float:x, Float:y, Float:z;
    GetPlayerPos(target, x, y, z);
    CreateExplosion(x, y, z, 7, 6.0);

    new msg[144];
    format(msg, sizeof msg, "%s hat %s gesprengt.", GetName(playerid), GetName(target));
    Admin_Broadcast(msg);
    return 1;
}

CMD:god(playerid, params[])
{
    #pragma unused params
    require(Admin_Require(playerid, ADMIN_ADMIN), "Keine Berechtigung.");

    PlayerData[playerid][pGodMode] = !PlayerData[playerid][pGodMode];

    if (PlayerData[playerid][pGodMode])
    {
        SetPlayerHealth(playerid, 100.0);
        SendSuccess(playerid, "Godmode aktiviert.");
    }
    else
    {
        SendSuccess(playerid, "Godmode deaktiviert.");
    }
    return 1;
}

CMD:setskin(playerid, params[])
{
    require(Admin_Require(playerid, ADMIN_ADMIN), "Keine Berechtigung.");
    require(Params_Count(params) >= 2, "Benutzung: /setskin [Spieler] [Skin 0-311]");

    new arg[MAX_PLAYER_NAME + 1];
    Params_Get(params, 0, arg, sizeof arg);

    new target = ResolveTarget(playerid, arg);
    if (target == INVALID_PLAYER_ID) return 1;

    new skin = Params_Int(params, 1);
    require(skin >= 0 && skin <= 311, "Die Skin-ID muss zwischen 0 und 311 liegen.");

    PlayerData[target][pSkin] = skin;
    SetPlayerSkin(target, skin);

    new msg[144];
    format(msg, sizeof msg, "%s hat den Skin von %s auf %d gesetzt.",
        GetName(playerid), GetName(target), skin);
    SendMessageToAdmins(ADMIN_MODERATOR, msg);
    return 1;
}

// ---------------------------------------------------------------------------
// Kommandos - Stufe 3 (Senior Admin)
// ---------------------------------------------------------------------------
CMD:unban(playerid, params[])
{
    require(Admin_Require(playerid, ADMIN_SENIOR), "Keine Berechtigung.");
    require(params[0] != EOS, "Benutzung: /unban [Name]");

    new msg[160];

    if (!Admin_RemoveBan(params))
    {
        format(msg, sizeof msg, "Fuer '%s' liegt kein Ban vor.", params);
        SendError(playerid, msg);
        return 1;
    }

    format(msg, sizeof msg, "%s hat den Ban von %s aufgehoben.", GetName(playerid), params);
    Admin_Broadcast(msg);
    return 1;
}

CMD:setmoney(playerid, params[])
{
    require(Admin_Require(playerid, ADMIN_SENIOR), "Keine Berechtigung.");
    require(Params_Count(params) >= 2, "Benutzung: /setmoney [Spieler] [Betrag]");

    new arg[MAX_PLAYER_NAME + 1];
    Params_Get(params, 0, arg, sizeof arg);

    new target = ResolveTarget(playerid, arg);
    if (target == INVALID_PLAYER_ID) return 1;
    require(Params_IsInt(params, 1), "Der Betrag muss eine Zahl sein.");

    new amount = Params_Int(params, 1);

    ResetPlayerMoney(target);
    GivePlayerMoney(target, amount);
    PlayerData[target][pMoney] = amount;

    new formatted[24], msg[160];
    FormatNumber(amount, formatted, sizeof formatted);
    format(msg, sizeof msg, "%s hat das Geld von %s auf $%s gesetzt.",
        GetName(playerid), GetName(target), formatted);
    SendMessageToAdmins(ADMIN_MODERATOR, msg);
    Log(msg);
    return 1;
}

CMD:setscore(playerid, params[])
{
    require(Admin_Require(playerid, ADMIN_SENIOR), "Keine Berechtigung.");
    require(Params_Count(params) >= 2, "Benutzung: /setscore [Spieler] [Punkte]");

    new arg[MAX_PLAYER_NAME + 1];
    Params_Get(params, 0, arg, sizeof arg);

    new target = ResolveTarget(playerid, arg);
    if (target == INVALID_PLAYER_ID) return 1;
    require(Params_IsInt(params, 1), "Die Punktzahl muss eine Zahl sein.");

    new score = Params_Int(params, 1);
    PlayerData[target][pScore] = score;
    SetPlayerScore(target, score);

    new msg[160];
    format(msg, sizeof msg, "%s hat die Punkte von %s auf %d gesetzt.",
        GetName(playerid), GetName(target), score);
    SendMessageToAdmins(ADMIN_MODERATOR, msg);
    return 1;
}

CMD:ann(playerid, params[])
{
    require(Admin_Require(playerid, ADMIN_SENIOR), "Keine Berechtigung.");
    require(params[0] != EOS, "Benutzung: /ann [Nachricht]");

    new gametext[160];
    format(gametext, sizeof gametext, "~y~%s", params);
    GameTextForAll(gametext, 5000, 3);

    new msg[192];
    format(msg, sizeof msg, "%s", params);
    SendAnnounce(msg);

    format(msg, sizeof msg, "Ankuendigung von %s: %s", GetName(playerid), params);
    Log(msg);
    return 1;
}

CMD:setweather(playerid, params[])
{
    require(Admin_Require(playerid, ADMIN_SENIOR), "Keine Berechtigung.");
    require(Params_IsInt(params, 0), "Benutzung: /setweather [0-45]");

    new weather = Params_Int(params, 0);
    require(weather >= 0 && weather <= 45, "Die Wetter-ID muss zwischen 0 und 45 liegen.");

    SetWeather(weather);

    new msg[128];
    format(msg, sizeof msg, "%s hat das Wetter auf %d gestellt.", GetName(playerid), weather);
    Admin_Broadcast(msg);
    return 1;
}

CMD:settime(playerid, params[])
{
    require(Admin_Require(playerid, ADMIN_SENIOR), "Keine Berechtigung.");
    require(Params_IsInt(params, 0), "Benutzung: /settime [0-23]");

    new hour = Params_Int(params, 0);
    require(hour >= 0 && hour <= 23, "Die Uhrzeit muss zwischen 0 und 23 liegen.");

    SetWorldTime(hour);

    new msg[128];
    format(msg, sizeof msg, "%s hat die Uhrzeit auf %d:00 gestellt.", GetName(playerid), hour);
    Admin_Broadcast(msg);
    return 1;
}

// ---------------------------------------------------------------------------
// Kommandos - Stufe 4 (Head Admin)
// ---------------------------------------------------------------------------
CMD:setvip(playerid, params[])
{
    require(Admin_Require(playerid, ADMIN_HEAD), "Keine Berechtigung.");
    require(Params_Count(params) >= 2, "Benutzung: /setvip [Spieler] [0-3]");

    new arg[MAX_PLAYER_NAME + 1];
    Params_Get(params, 0, arg, sizeof arg);

    new target = ResolveTarget(playerid, arg);
    if (target == INVALID_PLAYER_ID) return 1;

    new level = Params_Int(params, 1);
    require(level >= VIP_NONE && level <= VIP_GOLD, "Die VIP-Stufe muss zwischen 0 und 3 liegen.");

    PlayerData[target][pVipLevel] = level;
    SetPlayerColor(target, GetPlayerRankColor(target));
    Account_Save(target);

    new msg[160];
    format(msg, sizeof msg, "%s hat %s die Stufe '%s' gegeben.",
        GetName(playerid), GetName(target), GetVipRankName(level));
    Admin_Broadcast(msg);
    return 1;
}

CMD:aduty(playerid, params[])
{
    #pragma unused params
    require(Admin_Require(playerid, ADMIN_MODERATOR), "Keine Berechtigung.");

    PlayerData[playerid][pAdminDuty] = !PlayerData[playerid][pAdminDuty];

    new msg[144];
    if (PlayerData[playerid][pAdminDuty])
    {
        SetPlayerHealth(playerid, 100.0);
        SetPlayerArmour(playerid, 100.0);
        format(msg, sizeof msg, "%s (%s) ist jetzt im Dienst.",
            GetName(playerid), GetAdminRankName(AdminLevel(playerid)));
    }
    else
    {
        format(msg, sizeof msg, "%s (%s) ist nicht mehr im Dienst.",
            GetName(playerid), GetAdminRankName(AdminLevel(playerid)));
    }
    Admin_Broadcast(msg);
    return 1;
}

// ---------------------------------------------------------------------------
// Kommandos - Stufe 5 (Owner)
// ---------------------------------------------------------------------------
CMD:setlevel(playerid, params[])
{
    require(Admin_Require(playerid, ADMIN_OWNER), "Keine Berechtigung.");
    require(Params_Count(params) >= 2, "Benutzung: /setlevel [Spieler] [0-5]");

    new arg[MAX_PLAYER_NAME + 1];
    Params_Get(params, 0, arg, sizeof arg);

    new target = ResolveTarget(playerid, arg);
    if (target == INVALID_PLAYER_ID) return 1;

    new level = Params_Int(params, 1);
    require(level >= ADMIN_NONE && level <= ADMIN_OWNER, "Die Stufe muss zwischen 0 und 5 liegen.");

    PlayerData[target][pAdminLevel] = level;
    SetPlayerColor(target, GetPlayerRankColor(target));
    Account_Save(target);

    new msg[160];
    format(msg, sizeof msg, "%s hat %s zum '%s' gemacht.",
        GetName(playerid), GetName(target), GetAdminRankName(level));
    Admin_Broadcast(msg);
    return 1;
}

CMD:gmx(playerid, params[])
{
    #pragma unused params
    require(Admin_Require(playerid, ADMIN_OWNER), "Keine Berechtigung.");

    new msg[128];
    format(msg, sizeof msg, "%s startet den Gamemode neu. Alle Daten werden gespeichert.",
        GetName(playerid));
    Admin_Broadcast(msg);

    foreach_player(i)
    {
        if (IsLoggedIn(i)) Account_Save(i);
    }
    House_SaveAll();

    GameModeExit();
    return 1;
}
