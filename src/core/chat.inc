/*
 * Jebiga-Gaming - Chat
 *
 * Hauptchat mit Rangfarben, Flood- und Werbeschutz, private Nachrichten sowie
 * getrennte Kanaele fuer Admins und VIPs.
 */

#if defined _jebiga_chat_included
    #endinput
#endif
#define _jebiga_chat_included

#define CHAT_MIN_INTERVAL       (1200)  // ms zwischen zwei Nachrichten
#define CHAT_MAX_STRIKES        (4)     // danach automatischer Mute
#define CHAT_AUTOMUTE_TIME      (120)   // Sekunden

// ---------------------------------------------------------------------------
// Werbeschutz
// ---------------------------------------------------------------------------

/*
 * Erkennt Serverwerbung: IP-Adressen mit optionalem Port sowie bekannte
 * Domainendungen. Bewusst konservativ, damit normale Unterhaltungen nicht
 * blockiert werden.
 */
stock bool:ContainsAdvertising(const text[])
{
    static const domains[][] = { ".com", ".net", ".org", ".de", ".ru", ".tk", ".pl", ".eu" };

    for (new i = 0; i < sizeof domains; i++)
    {
        if (strfind(text, domains[i], true) != -1) return true;
    }

    // Vier durch Punkte getrennte Zahlengruppen = IP-Adresse
    new groups = 0, digits = 0;
    for (new i = 0; ; i++)
    {
        new c = text[i];
        if (c >= '0' && c <= '9')
        {
            digits++;
        }
        else if (c == '.' || c == ',' || c == ':')
        {
            if (digits > 0) { groups++; digits = 0; }
        }
        else
        {
            if (digits > 0) groups++;
            if (groups >= 4) return true;
            groups = 0;
            digits = 0;
        }
        if (c == EOS) break;
    }
    return (groups >= 4);
}

// ---------------------------------------------------------------------------
// Mute-Verwaltung
// ---------------------------------------------------------------------------
stock bool:IsPlayerMuted(playerid)
{
    if (PlayerData[playerid][pMutedUntil] == 0) return false;

    if (gettime() >= PlayerData[playerid][pMutedUntil])
    {
        PlayerData[playerid][pMutedUntil] = 0;
        SendInfo(playerid, "Deine Stummschaltung ist abgelaufen.");
        return false;
    }
    return true;
}

stock MutePlayer(playerid, seconds)
{
    PlayerData[playerid][pMutedUntil] = gettime() + seconds;
    return 1;
}

stock UnmutePlayer(playerid)
{
    PlayerData[playerid][pMutedUntil] = 0;
    return 1;
}

// ---------------------------------------------------------------------------
// Kanaele
// ---------------------------------------------------------------------------
stock SendMessageToAdmins(minLevel, const message[])
{
    new buf[192];
    format(buf, sizeof buf, "%s%s", EC_ORANGE, message);

    foreach_player(i)
    {
        if (AdminLevel(i) >= minLevel) SendClientMessage(i, COL_WHITE, buf);
    }
    return 1;
}

stock SendMessageToVips(const message[])
{
    new buf[192];
    format(buf, sizeof buf, "%s%s", EC_GOLD, message);

    foreach_player(i)
    {
        if (IsVip(i) || AdminLevel(i) >= ADMIN_ADMIN) SendClientMessage(i, COL_WHITE, buf);
    }
    return 1;
}

// ---------------------------------------------------------------------------
// Hauptchat
// ---------------------------------------------------------------------------

/*
 * Verarbeitet eine Chatnachricht. Gibt 0 zurueck, wenn die Nachricht
 * unterdrueckt werden soll (SA-MP verschickt sie dann nicht selbst).
 */
stock Chat_OnPlayerText(playerid, text[])
{
    if (!IsLoggedIn(playerid))
    {
        SendError(playerid, "Du musst dich erst einloggen.");
        return 0;
    }

    if (IsPlayerMuted(playerid))
    {
        new left[32], msg[96];
        FormatDuration(PlayerData[playerid][pMutedUntil] - gettime(), left, sizeof left);
        format(msg, sizeof msg, "Du bist noch %s stummgeschaltet.", left);
        SendError(playerid, msg);
        return 0;
    }

    // Flood-Schutz
    new tick = GetTickCount();
    if (tick - PlayerData[playerid][pLastChatTick] < CHAT_MIN_INTERVAL)
    {
        PlayerData[playerid][pSpamStrikes]++;

        if (PlayerData[playerid][pSpamStrikes] >= CHAT_MAX_STRIKES)
        {
            MutePlayer(playerid, CHAT_AUTOMUTE_TIME);
            PlayerData[playerid][pSpamStrikes] = 0;

            new msg[128];
            format(msg, sizeof msg, "%s wurde wegen Spam fuer %d Sekunden stummgeschaltet.",
                GetName(playerid), CHAT_AUTOMUTE_TIME);
            SendClientMessageToAll(COL_ORANGE, msg);
            return 0;
        }
        SendError(playerid, "Nicht so schnell - warte kurz vor der naechsten Nachricht.");
        return 0;
    }
    PlayerData[playerid][pLastChatTick] = tick;

    if (PlayerData[playerid][pSpamStrikes] > 0) PlayerData[playerid][pSpamStrikes]--;

    // Werbeschutz (Admins ausgenommen)
    if (AdminLevel(playerid) < ADMIN_MODERATOR && ContainsAdvertising(text))
    {
        SendError(playerid, "Werbung fuer andere Server oder Webseiten ist nicht erlaubt.");

        new notice[160];
        format(notice, sizeof notice, "[WERBEFILTER] %s: %s", GetName(playerid), text);
        SendMessageToAdmins(ADMIN_MODERATOR, notice);
        return 0;
    }

    MarkActivity(playerid);

    new msg[192], prefix[32], crewTag[24];
    prefix[0] = EOS;

    if (AdminLevel(playerid) > ADMIN_NONE)  format(prefix, sizeof prefix, "%s[Admin] ", EC_ORANGE);
    else if (IsVip(playerid))               format(prefix, sizeof prefix, "%s[VIP] ", EC_GOLD);

    Crew_GetChatTag(playerid, crewTag, sizeof crewTag);

    format(msg, sizeof msg, "%s%s%s(%d): %s%s",
        prefix, crewTag, GetName(playerid), playerid, EC_WHITE, text);
    SendClientMessageToAll(GetPlayerRankColor(playerid), msg);
    return 0;
}

// ---------------------------------------------------------------------------
// Kommandos
// ---------------------------------------------------------------------------
CMD:pm(playerid, params[])
{
    require(Params_Count(params) >= 2, "Benutzung: /pm [Spieler] [Nachricht]");

    new arg[MAX_PLAYER_NAME + 1];
    Params_Get(params, 0, arg, sizeof arg);

    new target = ResolveTarget(playerid, arg);
    if (target == INVALID_PLAYER_ID) return 1;

    require(target != playerid, "Du kannst dir selbst keine Nachricht schicken.");
    require(!IsPlayerMuted(playerid), "Du bist stummgeschaltet.");

    new text[128];
    Params_Rest(params, 1, text, sizeof text);

    new msg[192];
    format(msg, sizeof msg, "%s[PM von %s(%d)]: %s%s",
        EC_YELLOW, GetName(playerid), playerid, EC_WHITE, text);
    SendClientMessage(target, COL_WHITE, msg);

    format(msg, sizeof msg, "%s[PM an %s(%d)]: %s%s",
        EC_GREY, GetName(target), target, EC_WHITE, text);
    SendClientMessage(playerid, COL_WHITE, msg);

    PlayerData[target][pPmPartner]   = playerid;
    PlayerData[playerid][pPmPartner] = target;
    return 1;
}

CMD:r(playerid, params[])
{
    new target = PlayerData[playerid][pPmPartner];

    require(target != INVALID_PLAYER_ID && IsPlayerConnected(target), "Es gibt keine Unterhaltung, auf die du antworten koenntest.");
    require(params[0] != EOS, "Benutzung: /r [Nachricht]");

    new msg[192];
    format(msg, sizeof msg, "%s[PM von %s(%d)]: %s%s",
        EC_YELLOW, GetName(playerid), playerid, EC_WHITE, params);
    SendClientMessage(target, COL_WHITE, msg);

    format(msg, sizeof msg, "%s[PM an %s(%d)]: %s%s",
        EC_GREY, GetName(target), target, EC_WHITE, params);
    SendClientMessage(playerid, COL_WHITE, msg);
    return 1;
}

CMD:report(playerid, params[])
{
    require(Params_Count(params) >= 2, "Benutzung: /report [Spieler] [Grund]");

    new arg[MAX_PLAYER_NAME + 1];
    Params_Get(params, 0, arg, sizeof arg);

    new target = ResolveTarget(playerid, arg);
    if (target == INVALID_PLAYER_ID) return 1;

    new reason[96];
    Params_Rest(params, 1, reason, sizeof reason);

    new msg[192];
    format(msg, sizeof msg, "[REPORT] %s(%d) meldet %s(%d): %s",
        GetName(playerid), playerid, GetName(target), target, reason);
    SendMessageToAdmins(ADMIN_MODERATOR, msg);
    Log(msg);

    SendSuccess(playerid, "Deine Meldung wurde an das Team weitergeleitet.");
    return 1;
}

CMD:a(playerid, params[])
{
    require(IsAdmin(playerid, ADMIN_MODERATOR), "Dieses Kommando ist dem Team vorbehalten.");
    require(params[0] != EOS, "Benutzung: /a [Nachricht]");

    new msg[192];
    format(msg, sizeof msg, "[A-Chat] %s %s: %s",
        GetAdminRankName(AdminLevel(playerid)), GetName(playerid), params);
    SendMessageToAdmins(ADMIN_MODERATOR, msg);
    return 1;
}

CMD:v(playerid, params[])
{
    require(IsVip(playerid) || IsAdmin(playerid, ADMIN_ADMIN), "Dieser Chat ist VIPs vorbehalten. Infos: /vip");
    require(params[0] != EOS, "Benutzung: /v [Nachricht]");

    new msg[192];
    format(msg, sizeof msg, "[VIP] %s: %s", GetName(playerid), params);
    SendMessageToVips(msg);
    return 1;
}
