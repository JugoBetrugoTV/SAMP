/*
 * UIF Freeroam - Allgemeine Hilfsfunktionen
 *
 * Nachrichten-Wrapper, Spielersuche, Zeit-/Zahlformatierung und Logging.
 */

#if defined _uif_util_included
    #endinput
#endif
#define _uif_util_included

// Rueckgabewert von FindPlayer, wenn ein Namensfragment auf mehrere Spieler passt
#define PLAYER_AMBIGUOUS        (-2)

// ---------------------------------------------------------------------------
// Nachrichten
// ---------------------------------------------------------------------------
stock SendInfo(playerid, const message[])
{
    new buf[160];
    format(buf, sizeof buf, "%s* %s", EC_CYAN, message);
    return SendClientMessage(playerid, COL_WHITE, buf);
}

stock SendError(playerid, const message[])
{
    new buf[160];
    format(buf, sizeof buf, "%s* %s", EC_RED, message);
    return SendClientMessage(playerid, COL_WHITE, buf);
}

stock SendSuccess(playerid, const message[])
{
    new buf[160];
    format(buf, sizeof buf, "%s* %s", EC_GREEN, message);
    return SendClientMessage(playerid, COL_WHITE, buf);
}

stock SendUsage(playerid, const usage[])
{
    new buf[160];
    format(buf, sizeof buf, "%s* Benutzung: %s%s", EC_YELLOW, EC_WHITE, usage);
    return SendClientMessage(playerid, COL_WHITE, buf);
}

stock SendAnnounce(const message[])
{
    new buf[176];
    format(buf, sizeof buf, "%s[SERVER] %s%s", EC_GOLD, EC_WHITE, message);
    return SendClientMessageToAll(COL_WHITE, buf);
}

// ---------------------------------------------------------------------------
// Spieler
// ---------------------------------------------------------------------------
stock GetName(playerid)
{
    new name[MAX_PLAYER_NAME + 1];
    if (!GetPlayerName(playerid, name, sizeof name)) name = "Unbekannt";
    return name;
}

/*
 * Sucht einen Spieler anhand einer ID oder eines Namensfragments.
 * Gibt die Spieler-ID zurueck, INVALID_PLAYER_ID wenn nichts gefunden wurde
 * oder PLAYER_AMBIGUOUS, wenn das Fragment auf mehrere Spieler passt.
 */
stock FindPlayer(const input[])
{
    if (input[0] == EOS) return INVALID_PLAYER_ID;

    // Reine Zahl -> als Spieler-ID behandeln
    new bool:numeric = true;
    for (new i = 0; input[i] != EOS; i++)
    {
        if (input[i] < '0' || input[i] > '9') { numeric = false; break; }
    }

    if (numeric)
    {
        new id = strval(input);
        if (IsPlayerConnected(id)) return id;
        return INVALID_PLAYER_ID;
    }

    new found = INVALID_PLAYER_ID, matches = 0, name[MAX_PLAYER_NAME + 1];

    foreach_player(i)
    {
        GetPlayerName(i, name, sizeof name);

        // Exakter Treffer gewinnt sofort
        if (!strcmp(name, input, true)) return i;

        if (strfind(name, input, true) != -1)
        {
            found = i;
            matches++;
        }
    }

    if (matches > 1) return PLAYER_AMBIGUOUS;
    return found;
}

/*
 * Loest ein Kommando-Argument in einen Spieler auf und meldet Fehler direkt an
 * den Aufrufer. Gibt INVALID_PLAYER_ID zurueck, wenn nichts gefunden wurde.
 */
stock ResolveTarget(playerid, const input[])
{
    new target = FindPlayer(input);

    if (target == PLAYER_AMBIGUOUS)
    {
        SendError(playerid, "Mehrere Spieler passen zu diesem Namen - bitte die ID benutzen.");
        return INVALID_PLAYER_ID;
    }
    if (target == INVALID_PLAYER_ID)
    {
        SendError(playerid, "Spieler nicht gefunden.");
        return INVALID_PLAYER_ID;
    }
    return target;
}

stock Float:GetPlayerDistanceToPlayer(playerid, targetid)
{
    new Float:x, Float:y, Float:z;
    GetPlayerPos(targetid, x, y, z);
    return GetPlayerDistanceFromPoint(playerid, x, y, z);
}

stock bool:IsPlayerInAnyVehicleAsDriver(playerid)
{
    return (GetPlayerVehicleID(playerid) != 0 &&
            GetPlayerState(playerid) == PLAYER_STATE_DRIVER);
}

/*
 * Kick mit kurzer Verzoegerung. Ein sofortiger Kick() verwirft Nachrichten,
 * die im selben Frame noch verschickt wurden - der Spieler saehe den Grund nie.
 */
forward OnKickDelayed(playerid);
public OnKickDelayed(playerid)
{
    Kick(playerid);
    return 1;
}

stock KickDelayed(playerid, delay = 500)
{
    return SetTimerEx("OnKickDelayed", delay, false, "i", playerid);
}

// ---------------------------------------------------------------------------
// Formatierung
// ---------------------------------------------------------------------------

// Sekunden -> "2d 4h 13m" / "4h 13m" / "13m 20s"
stock FormatDuration(seconds, dest[], len = sizeof dest)
{
    new days    = seconds / 86400;
    new hours   = (seconds % 86400) / 3600;
    new minutes = (seconds % 3600) / 60;
    new secs    = seconds % 60;

    if (days > 0)       format(dest, len, "%dd %dh %dm", days, hours, minutes);
    else if (hours > 0) format(dest, len, "%dh %dm", hours, minutes);
    else if (minutes>0) format(dest, len, "%dm %ds", minutes, secs);
    else                format(dest, len, "%ds", secs);

    return strlen(dest);
}

// 1234567 -> "1.234.567"
stock FormatNumber(value, dest[], len = sizeof dest)
{
    new raw[24], bool:negative = (value < 0);
    format(raw, sizeof raw, "%d", negative ? -value : value);

    new rawLen = strlen(raw), pos = 0;
    if (negative && pos < len - 1) dest[pos++] = '-';

    for (new i = 0; i < rawLen && pos < len - 1; i++)
    {
        if (i > 0 && (rawLen - i) % 3 == 0) dest[pos++] = '.';
        if (pos < len - 1) dest[pos++] = raw[i];
    }
    dest[pos] = EOS;
    return pos;
}

stock GetTimestamp(dest[], len = sizeof dest)
{
    new y, m, d, h, mi, s;
    getdate(y, m, d);
    gettime(h, mi, s);
    format(dest, len, "%02d.%02d.%d %02d:%02d:%02d", d, m, y, h, mi, s);
    return strlen(dest);
}

// ---------------------------------------------------------------------------
// Logging
// ---------------------------------------------------------------------------
stock Log(const text[])
{
    new stamp[32];
    GetTimestamp(stamp, sizeof stamp);

    new line[256];
    format(line, sizeof line, "[%s] %s\r\n", stamp, text);

    new File:handle = fopen(PATH_LOG, io_append);
    if (handle)
    {
        fwrite(handle, line);
        fclose(handle);
    }
    print(text);
    return 1;
}
