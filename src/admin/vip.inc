/*
 * UIF Freeroam - VIP
 *
 * Drei Stufen (Bronze, Silber, Gold) mit gestaffelten Vorteilen. Die Stufe
 * vergibt das Team per /setvip; hier stehen nur die Vorteile selbst.
 */

#if defined _uif_vip_included
    #endinput
#endif
#define _uif_vip_included

#define VIP_HEAL_COOLDOWN       (60)    // Sekunden zwischen zwei /vheal
#define VIP_MAX_NAME_COLORS     (8)

static gVipLastHeal[MAX_PLAYERS];

static const gVipColors[VIP_MAX_NAME_COLORS] =
{
    COL_RED, COL_GREEN, COL_BLUE, COL_YELLOW,
    COL_ORANGE, COL_PURPLE, COL_CYAN, COL_PINK
};

static const gVipColorNames[VIP_MAX_NAME_COLORS][] =
{
    "Rot", "Gruen", "Blau", "Gelb", "Orange", "Lila", "Tuerkis", "Pink"
};

/*
 * Prueft die VIP-Stufe und meldet fehlende Berechtigung.
 */
stock bool:Vip_Require(playerid, level)
{
    if (VipLevel(playerid) >= level || IsAdmin(playerid, ADMIN_ADMIN)) return true;

    new msg[128];
    format(msg, sizeof msg, "Dafuer brauchst du mindestens %s. Infos: /vip",
        GetVipRankName(level));
    SendError(playerid, msg);
    return false;
}

stock Vip_ShowMenu(playerid)
{
    new body[1024];

    format(body, sizeof body,
        "%sDein Status: %s%s\n\n\
        %sBronze VIP\n%s\
        - Raketenwerfer- und Jetpack-Set\n\
        - VIP-Chat (/v) und VIP-Lounge\n\
        - /vheal einmal pro Minute\n\
        - Fahrzeugreparatur per Ducktaste\n\n\
        %sSilber VIP %s(zusaetzlich)\n%s\
        - Rambo-Waffenset\n\
        - VIP-Stuntpark\n\
        - /vcolor - eigene Namensfarbe\n\
        - 25%% mehr Stuntpunkte\n\n\
        %sGold VIP %s(zusaetzlich)\n%s\
        - Gold-Insel\n\
        - /vjetpack jederzeit\n\
        - Exklusive Haeuser\n\n\
        %sVIP-Status gibt es auf %s",
        EC_GREY, EC_GOLD, GetVipRankName(VipLevel(playerid)),
        EC_GOLD, EC_WHITE,
        EC_GOLD, EC_GREY, EC_WHITE,
        EC_GOLD, EC_GREY, EC_WHITE,
        EC_CYAN, SERVER_WEBSITE);

    return ShowPlayerDialog(playerid, DIALOG_VIP_MENU, DIALOG_STYLE_MSGBOX,
        "VIP-Vorteile", body, "Schliessen", "");
}

stock Vip_OnDialogResponse(playerid, dialogid, response, listitem)
{
    if (dialogid != DIALOG_VIP_MENU) return 0;

    #pragma unused response
    #pragma unused listitem
    #pragma unused playerid
    return 1;
}

// ---------------------------------------------------------------------------
// Kommandos
// ---------------------------------------------------------------------------
CMD:vip(playerid, params[])
{
    #pragma unused params
    return Vip_ShowMenu(playerid);
}

CMD:vheal(playerid, params[])
{
    #pragma unused params
    if (!Vip_Require(playerid, VIP_BRONZE)) return 1;

    new now = gettime(), elapsed = now - gVipLastHeal[playerid];

    if (elapsed < VIP_HEAL_COOLDOWN)
    {
        new msg[96];
        format(msg, sizeof msg, "Noch %d Sekunden bis zur naechsten Heilung.",
            VIP_HEAL_COOLDOWN - elapsed);
        SendError(playerid, msg);
        return 1;
    }
    require(PlayerData[playerid][pMinigame] == MINIGAME_NONE, "Waehrend eines Minispiels kannst du dich nicht heilen.");

    SetPlayerHealth(playerid, 100.0);
    SetPlayerArmour(playerid, 100.0);
    gVipLastHeal[playerid] = now;

    SendSuccess(playerid, "VIP-Bonus: voll geheilt und ausgeruestet.");
    return 1;
}

CMD:vjetpack(playerid, params[])
{
    #pragma unused params
    if (!Vip_Require(playerid, VIP_GOLD)) return 1;
    require(PlayerData[playerid][pMinigame] == MINIGAME_NONE, "Waehrend eines Minispiels gibt es kein Jetpack.");

    SetPlayerSpecialAction(playerid, SPECIAL_ACTION_USEJETPACK);
    SendSuccess(playerid, "Jetpack aktiviert.");
    return 1;
}

CMD:vcolor(playerid, params[])
{
    if (!Vip_Require(playerid, VIP_SILVER)) return 1;

    if (params[0] == EOS)
    {
        new list[192], entry[32];
        for (new i = 0; i < VIP_MAX_NAME_COLORS; i++)
        {
            format(entry, sizeof entry, "%d = %s  ", i + 1, gVipColorNames[i]);
            strcat(list, entry);
        }
        SendUsage(playerid, "/vcolor [Nummer]");
        SendInfo(playerid, list);
        return 1;
    }

    new choice = Params_Int(params, 0);
    require(choice >= 1 && choice <= VIP_MAX_NAME_COLORS, "Ungueltige Farbnummer.");

    SetPlayerColor(playerid, gVipColors[choice - 1]);

    new msg[96];
    format(msg, sizeof msg, "Namensfarbe auf %s gesetzt.", gVipColorNames[choice - 1]);
    SendSuccess(playerid, msg);
    return 1;
}
