/*
 * UIF Freeroam - Crews
 *
 * Spielergruppen mit Kuerzel, eigenem Chat und gemeinsamer Statistik.
 *
 * Zur Speicherung: im Account steht der Crew*name*, nicht ihr Index. Indizes
 * verschieben sich, sobald eine Crew aufgeloest wird - der Name bleibt stabil.
 * Beim Login wird er einmal in den Laufzeitindex aufgeloest.
 */

#if defined _uif_crew_included
    #endinput
#endif
#define _uif_crew_included

#define MAX_CREWS               (48)
#define CREW_NAME_LEN           (24)
#define CREW_TAG_LEN            (6)
#define CREW_CREATE_COST        (50000)
#define CREW_MIN_TAG            (2)
#define CREW_MIN_NAME           (3)

#define PATH_CREWS              "crews.ini"

enum E_CREW
{
    cName[CREW_NAME_LEN],
    cTag[CREW_TAG_LEN],
    cLeader[MAX_PLAYER_NAME + 1],
    cScore,
    bool:cUsed
};

static gCrews[MAX_CREWS][E_CREW];

// ---------------------------------------------------------------------------
// Suche und Verwaltung
// ---------------------------------------------------------------------------
stock Crew_FindByName(const name[])
{
    if (name[0] == EOS) return -1;

    for (new i = 0; i < MAX_CREWS; i++)
    {
        if (gCrews[i][cUsed] && !strcmp(gCrews[i][cName], name, true)) return i;
    }
    return -1;
}

static Crew_FindFreeSlot()
{
    for (new i = 0; i < MAX_CREWS; i++)
    {
        if (!gCrews[i][cUsed]) return i;
    }
    return -1;
}

stock Crew_CountMembers(index)
{
    new count = 0;
    foreach_player(i)
    {
        if (PlayerData[i][pCrew] == index) count++;
    }
    return count;
}

stock bool:Crew_IsLeader(playerid, index)
{
    if (index < 0 || !gCrews[index][cUsed]) return false;
    return (strcmp(gCrews[index][cLeader], GetName(playerid), true) == 0);
}

// ---------------------------------------------------------------------------
// Speichern und Laden
// ---------------------------------------------------------------------------
stock Crew_SaveAll()
{
    if (!INI_Open(PATH_CREWS)) return 0;

    new key[24];
    for (new i = 0; i < MAX_CREWS; i++)
    {
        if (!gCrews[i][cUsed]) continue;

        format(key, sizeof key, "Crew%d_Name", i);   INI_WriteString(key, gCrews[i][cName]);
        format(key, sizeof key, "Crew%d_Tag", i);    INI_WriteString(key, gCrews[i][cTag]);
        format(key, sizeof key, "Crew%d_Leader", i); INI_WriteString(key, gCrews[i][cLeader]);
        format(key, sizeof key, "Crew%d_Score", i);  INI_WriteInt(key, gCrews[i][cScore]);
    }
    INI_Close();
    return 1;
}

stock Crew_OnGameModeInit()
{
    for (new i = 0; i < MAX_CREWS; i++) gCrews[i][cUsed] = false;

    new loaded = 0;
    if (fexist(PATH_CREWS))
    {
        new key[24];
        for (new i = 0; i < MAX_CREWS; i++)
        {
            format(key, sizeof key, "Crew%d_Name", i);
            if (!INI_GetString(PATH_CREWS, key, gCrews[i][cName], CREW_NAME_LEN)) continue;
            if (gCrews[i][cName][0] == EOS) continue;

            format(key, sizeof key, "Crew%d_Tag", i);
            INI_GetString(PATH_CREWS, key, gCrews[i][cTag], CREW_TAG_LEN);
            format(key, sizeof key, "Crew%d_Leader", i);
            INI_GetString(PATH_CREWS, key, gCrews[i][cLeader], MAX_PLAYER_NAME + 1);
            format(key, sizeof key, "Crew%d_Score", i);
            gCrews[i][cScore] = INI_GetInt(PATH_CREWS, key);

            gCrews[i][cUsed] = true;
            loaded++;
        }
    }

    new msg[64];
    format(msg, sizeof msg, "%d Crews geladen.", loaded);
    Log(msg);
    return loaded;
}

/*
 * Loest den gespeicherten Crewnamen in den Laufzeitindex auf. Existiert die
 * Crew nicht mehr, wird die Zuordnung im Account still entfernt.
 */
stock Crew_OnPlayerLogin(playerid)
{
    PlayerData[playerid][pCrew] = -1;
    if (PlayerData[playerid][pCrewName][0] == EOS) return 0;

    new index = Crew_FindByName(PlayerData[playerid][pCrewName]);
    if (index == -1)
    {
        PlayerData[playerid][pCrewName][0] = EOS;
        return 0;
    }

    PlayerData[playerid][pCrew] = index;

    new msg[128];
    format(msg, sizeof msg, "Du bist Mitglied der Crew %s[%s] %s%s",
        EC_WHITE, gCrews[index][cTag], gCrews[index][cName], EC_CYAN);
    SendInfo(playerid, msg);
    return 1;
}

/*
 * Kuerzel fuer die Chatzeile. Leerer String, wenn der Spieler in keiner Crew ist.
 */
stock Crew_GetChatTag(playerid, dest[], len = sizeof dest)
{
    dest[0] = EOS;

    new index = PlayerData[playerid][pCrew];
    if (index == -1 || !gCrews[index][cUsed]) return 0;

    format(dest, len, "%s[%s] ", EC_PURPLE, gCrews[index][cTag]);
    return strlen(dest);
}

stock Crew_SendMessage(index, const message[])
{
    new buf[192];
    format(buf, sizeof buf, "%s%s", EC_PURPLE, message);

    foreach_player(i)
    {
        if (PlayerData[i][pCrew] == index) SendClientMessage(i, COL_WHITE, buf);
    }
    return 1;
}

stock Crew_OnDialogResponse(playerid, dialogid, response, listitem)
{
    #pragma unused listitem
    #pragma unused response
    #pragma unused playerid

    if (dialogid != DIALOG_CREW_MENU && dialogid != DIALOG_CREW_LIST) return 0;
    return 1;
}

// ---------------------------------------------------------------------------
// Kommandos
// ---------------------------------------------------------------------------
CMD:createcrew(playerid, params[])
{
    require(PlayerData[playerid][pCrew] == -1, "Du bist bereits in einer Crew. (/leavecrew)");
    require(Params_Count(params) >= 2, "Benutzung: /createcrew [Kuerzel] [Name]");

    new money[24];
    FormatNumber(CREW_CREATE_COST, money, sizeof money);

    if (GetPlayerMoney(playerid) < CREW_CREATE_COST)
    {
        new msg[112];
        format(msg, sizeof msg, "Eine Crew zu gruenden kostet $%s.", money);
        SendError(playerid, msg);
        return 1;
    }

    new tag[CREW_TAG_LEN];
    Params_Get(params, 0, tag, sizeof tag);
    require(strlen(tag) >= CREW_MIN_TAG, "Das Kuerzel braucht mindestens 2 Zeichen.");

    new name[CREW_NAME_LEN];
    Params_Rest(params, 1, name, sizeof name);
    require(strlen(name) >= CREW_MIN_NAME, "Der Crewname braucht mindestens 3 Zeichen.");
    require(Crew_FindByName(name) == -1, "Diesen Crewnamen gibt es bereits.");

    new slot = Crew_FindFreeSlot();
    require(slot != -1, "Der Server hat die maximale Anzahl an Crews erreicht.");

    format(gCrews[slot][cName], CREW_NAME_LEN, "%s", name);
    format(gCrews[slot][cTag], CREW_TAG_LEN, "%s", tag);
    format(gCrews[slot][cLeader], MAX_PLAYER_NAME + 1, "%s", GetName(playerid));
    gCrews[slot][cScore] = 0;
    gCrews[slot][cUsed]  = true;

    PlayerData[playerid][pCrew] = slot;
    format(PlayerData[playerid][pCrewName], CREW_NAME_LEN, "%s", name);

    GivePlayerMoney(playerid, -CREW_CREATE_COST);
    Crew_SaveAll();
    Account_Save(playerid);

    new msg[176];
    format(msg, sizeof msg, "%s hat die Crew [%s] %s gegruendet.", GetName(playerid), tag, name);
    SendClientMessageToAll(COL_PURPLE, msg);
    return 1;
}

CMD:invitecrew(playerid, params[])
{
    new index = PlayerData[playerid][pCrew];
    require(index != -1, "Du bist in keiner Crew.");
    require(Crew_IsLeader(playerid, index), "Nur die Crewleitung darf einladen.");
    require(params[0] != EOS, "Benutzung: /invitecrew [Spieler]");

    new target = ResolveTarget(playerid, params);
    if (target == INVALID_PLAYER_ID) return 1;
    require(target != playerid, "Du bist bereits dabei.");
    require(PlayerData[target][pCrew] == -1, "Dieser Spieler ist bereits in einer Crew.");

    PlayerData[target][pCrewInviteFrom] = index;

    new msg[176];
    format(msg, sizeof msg, "%s laedt dich in die Crew [%s] %s ein. Annehmen mit %s/joincrew",
        GetName(playerid), gCrews[index][cTag], gCrews[index][cName], EC_WHITE);
    SendInfo(target, msg);

    format(msg, sizeof msg, "Einladung an %s gesendet.", GetName(target));
    SendSuccess(playerid, msg);
    return 1;
}

CMD:joincrew(playerid, params[])
{
    #pragma unused params
    require(PlayerData[playerid][pCrew] == -1, "Du bist bereits in einer Crew.");

    new index = PlayerData[playerid][pCrewInviteFrom];
    require(index != -1, "Du hast keine offene Einladung.");
    require(index >= 0 && index < MAX_CREWS && gCrews[index][cUsed], "Diese Crew existiert nicht mehr.");

    PlayerData[playerid][pCrew] = index;
    PlayerData[playerid][pCrewInviteFrom] = -1;
    format(PlayerData[playerid][pCrewName], CREW_NAME_LEN, "%s", gCrews[index][cName]);
    Account_Save(playerid);

    new msg[144];
    format(msg, sizeof msg, "%s ist der Crew beigetreten (%d Mitglieder online).",
        GetName(playerid), Crew_CountMembers(index));
    Crew_SendMessage(index, msg);
    return 1;
}

CMD:leavecrew(playerid, params[])
{
    #pragma unused params

    new index = PlayerData[playerid][pCrew];
    require(index != -1, "Du bist in keiner Crew.");

    new msg[144];
    format(msg, sizeof msg, "%s hat die Crew verlassen.", GetName(playerid));
    Crew_SendMessage(index, msg);

    PlayerData[playerid][pCrew] = -1;
    PlayerData[playerid][pCrewName][0] = EOS;
    Account_Save(playerid);

    // Verlaesst die Leitung, loest sich die Crew auf
    if (Crew_IsLeader(playerid, index))
    {
        format(msg, sizeof msg, "Die Crew [%s] %s wurde aufgeloest - die Leitung ist ausgetreten.",
            gCrews[index][cTag], gCrews[index][cName]);
        SendClientMessageToAll(COL_PURPLE, msg);

        foreach_player(i)
        {
            if (PlayerData[i][pCrew] != index) continue;

            PlayerData[i][pCrew] = -1;
            PlayerData[i][pCrewName][0] = EOS;
            Account_Save(i);
        }
        gCrews[index][cUsed] = false;
    }

    Crew_SaveAll();
    SendSuccess(playerid, "Du hast die Crew verlassen.");
    return 1;
}

CMD:c(playerid, params[])
{
    new index = PlayerData[playerid][pCrew];
    require(index != -1, "Du bist in keiner Crew.");
    require(params[0] != EOS, "Benutzung: /c [Nachricht]");
    require(PlayerData[playerid][pMutedUntil] <= gettime(), "Du bist stummgeschaltet.");

    new msg[192];
    format(msg, sizeof msg, "[%s] %s: %s", gCrews[index][cTag], GetName(playerid), params);
    Crew_SendMessage(index, msg);
    return 1;
}

CMD:crews(playerid, params[])
{
    #pragma unused params

    new body[1400], entry[144], count = 0;

    for (new i = 0; i < MAX_CREWS; i++)
    {
        if (!gCrews[i][cUsed]) continue;

        format(entry, sizeof entry, "{9B59B6}[%s] {FFFFFF}%s  {AFAFAF}Leitung: %s, %d online\n",
            gCrews[i][cTag], gCrews[i][cName], gCrews[i][cLeader], Crew_CountMembers(i));

        if (strlen(body) + strlen(entry) >= sizeof(body) - 32) break;
        strcat(body, entry);
        count++;
    }

    if (count == 0) strcat(body, "{AFAFAF}Es gibt noch keine Crew. /createcrew gruendet die erste.");

    ShowPlayerDialog(playerid, DIALOG_CREW_LIST, DIALOG_STYLE_MSGBOX,
        "Crews auf dem Server", body, "Schliessen", "");
    return 1;
}

CMD:crew(playerid, params[])
{
    #pragma unused params

    new index = PlayerData[playerid][pCrew];
    new body[768];

    if (index == -1)
    {
        new money[24];
        FormatNumber(CREW_CREATE_COST, money, sizeof money);
        format(body, sizeof body,
            "%sDu bist in keiner Crew.\n\n\
            %s/createcrew [Kuerzel] [Name]%s - eigene Crew gruenden ($%s)\n\
            %s/crews%s - alle Crews des Servers ansehen\n\
            %s/joincrew%s - offene Einladung annehmen",
            EC_GREY, EC_WHITE, EC_GREY, money, EC_WHITE, EC_GREY, EC_WHITE, EC_GREY);
    }
    else
    {
        format(body, sizeof body,
            "%sCrew:%s [%s] %s\n\
            %sLeitung:%s %s\n\
            %sMitglieder online:%s %d\n\
            %sCrewpunkte:%s %d\n\n\
            %s/c [Text]%s - Crewchat\n\
            %s/invitecrew [Spieler]%s - einladen (nur Leitung)\n\
            %s/leavecrew%s - austreten",
            EC_GREY, EC_WHITE, gCrews[index][cTag], gCrews[index][cName],
            EC_GREY, EC_WHITE, gCrews[index][cLeader],
            EC_GREY, EC_WHITE, Crew_CountMembers(index),
            EC_GREY, EC_WHITE, gCrews[index][cScore],
            EC_WHITE, EC_GREY, EC_WHITE, EC_GREY, EC_WHITE, EC_GREY);
    }

    ShowPlayerDialog(playerid, DIALOG_CREW_MENU, DIALOG_STYLE_MSGBOX,
        "Crew", body, "Schliessen", "");
    return 1;
}
