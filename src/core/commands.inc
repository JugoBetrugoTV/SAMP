/*
 * Jebiga-Gaming - Allgemeine Kommandos
 *
 * Alles, was keinem Fachmodul gehoert: Hilfe, Statistiken, Spielerliste und
 * die rotierenden Servertipps. Wird als letztes Modul eingebunden und darf
 * deshalb auf alle anderen zugreifen.
 */

#if defined _jebiga_commands_included
    #endinput
#endif
#define _jebiga_commands_included

// ---------------------------------------------------------------------------
// Rotierende Servertipps
// ---------------------------------------------------------------------------
static const gAnnouncements[][] =
{
    "Im Fahrzeug die Taste 2 spammen gibt Schub - alles dazu mit /boost.",
    "Tippe /tp fuer ueber 40 Teleportziele quer ueber San Andreas.",
    "Der Jebiga Stuntpark hat einen echten Looping: /stuntpark",
    "/dm fuehrt in die Deathmatch-Arenen, /duel fordert jemanden direkt heraus.",
    "Eigene Crew gruenden und im Chat mit Kuerzel auftreten: /crew",
    "Geld verdienen ohne Kaempfen: /jobs bringt dich auf sechs Routen.",
    "Zinsen statt Bargeld herumliegen lassen: /bank",
    "Ueber 300 Skins zur Auswahl: /skins",
    "127 Innenraeume von San Andreas stehen offen: /interiors",
    "20 Erfolge mit Geldpraemien warten: /achievements",
    "Mit /veh [Name] spawnst du jedes Fahrzeug - zum Beispiel /veh infernus.",
    "/race eroeffnet ein Rennen, /derby ein Autoderby. Mitmachen lohnt sich.",
    "Spruenge mit dem Fahrzeug geben Stuntpunkte. /stunt bringt dich zu den besten Rampen.",
    "Mit /ramp setzt du dir jederzeit eine eigene Sprungrampe.",
    "Regelverstoesse bitte mit /report [Spieler] [Grund] melden.",
    "Haeuser kaufen? /houses zeigt alle Angebote auf dem Server.",
    "VIP-Vorteile und Unterstuetzung des Servers: /vip",
    "/stats zeigt deine Kills, Tode, Stuntpunkte und Spielzeit.",
    "Werbung fuer andere Server fuehrt zum Ban. Bitte fair bleiben."
};

static gAnnounceIndex = 0;

stock Announce_Rotate()
{
    new msg[192];
    format(msg, sizeof msg, "%s[TIPP] %s%s",
        EC_YELLOW, EC_WHITE, gAnnouncements[gAnnounceIndex]);
    SendClientMessageToAll(COL_WHITE, msg);

    gAnnounceIndex = (gAnnounceIndex + 1) % sizeof gAnnouncements;
    return 1;
}

// ---------------------------------------------------------------------------
// Hilfe
// ---------------------------------------------------------------------------
CMD:help(playerid, params[])
{
    #pragma unused params

    new body[2048];

    strcat(body, "{F1C40F}Teleport & Bewegung\n{FFFFFF}\
        /tp  /ls  /sf  /lv  /a51  /chiliad  /zt  /stunt  /tpto [Spieler]\n\n");

    strcat(body, "{F1C40F}Fahrzeuge\n{FFFFFF}\
        /veh [Name]  /dv  /fix  /flip  /nos  /color  /lock  /unlock  /vd  /tune\n\
        {2ECC71}Taste 2 im Fahrzeug gibt Schub - Details mit /boost\n\n");

    strcat(body, "{F1C40F}Waffen\n{FFFFFF}\
        /weapons  /gun [ID] [Munition]  /disarm\n\n");

    strcat(body, "{F1C40F}Minispiele\n{FFFFFF}\
        /race  /join  /derby  /leave  /ramp  /delramp\n\
        /dm  /leavedm  /duel [Spieler]  /accept  /stuntpark\n\n");

    strcat(body, "{F1C40F}Haeuser\n{FFFFFF}\
        /houses  /buyhouse  /sellhouse  /enter  /exit  /myhouse\n\n");

    strcat(body, "{F1C40F}Geld & Arbeit\n{FFFFFF}\
        /bank  /deposit  /withdraw  /transfer  /jobs  /quitjob  /pay\n\n");

    strcat(body, "{F1C40F}Aussehen & Orte\n{FFFFFF}\
        /skins  /interiors  /exitint  /anims  /weaponinfo\n\n");

    strcat(body, "{F1C40F}Crew\n{FFFFFF}\
        /crew  /crews  /createcrew  /invitecrew  /joincrew  /leavecrew  /c [Text]\n\n");

    strcat(body, "{F1C40F}Kommunikation\n{FFFFFF}\
        /pm [Spieler] [Text]  /r [Text]  /report  /v (VIP)\n\n");

    strcat(body, "{F1C40F}Sonstiges\n{FFFFFF}\
        /stats  /players  /top  /pos  /kill  /afk  /rules  /credits  /vip\n\
        /achievements  /stopanim  /rocket  /parachute  /eject\n\n");

    if (AdminLevel(playerid) > ADMIN_NONE)
    {
        strcat(body, "{E67E22}Als Teammitglied: /acmds zeigt deine Adminkommandos.\n");
    }

    ShowPlayerDialog(playerid, DIALOG_HELP, DIALOG_STYLE_MSGBOX,
        SERVER_NAME " - Hilfe", body, "Schliessen", "");
    return 1;
}
ALIAS:cmds(help)
ALIAS:befehle(help)
CMD:rules(playerid, params[])
{
    #pragma unused params

    new body[1024];

    strcat(body, "{FFFFFF}1. {AFAFAF}Kein Cheaten, Hacken oder Ausnutzen von Fehlern.\n");
    strcat(body, "{FFFFFF}2. {AFAFAF}Keine Werbung fuer andere Server oder Webseiten.\n");
    strcat(body, "{FFFFFF}3. {AFAFAF}Kein Spam und kein Grossbuchstaben-Dauerfeuer im Chat.\n");
    strcat(body, "{FFFFFF}4. {AFAFAF}Keine Beleidigungen, kein Rassismus, keine Bedrohungen.\n");
    strcat(body, "{FFFFFF}5. {AFAFAF}Keine Angriffe auf Spieler unter Spawnschutz.\n");
    strcat(body, "{FFFFFF}6. {AFAFAF}Anweisungen des Teams ist Folge zu leisten.\n");
    strcat(body, "{FFFFFF}7. {AFAFAF}Kein Missbrauch von Bugs im Haus- oder Minispielsystem.\n");
    strcat(body, "{FFFFFF}8. {AFAFAF}Ein Account pro Person. Keine Zweitaccounts zur Umgehung von Strafen.\n\n");
    strcat(body, "{E74C3C}Verstoesse fuehren zu Mute, Jail, Kick oder Ban.\n");
    strcat(body, "{AFAFAF}Beschwerden und Entbannungsantraege: " SERVER_WEBSITE);

    ShowPlayerDialog(playerid, DIALOG_RULES, DIALOG_STYLE_MSGBOX,
        "Serverregeln", body, "Verstanden", "");
    return 1;
}

CMD:credits(playerid, params[])
{
    #pragma unused params

    new body[512];
    format(body, sizeof body,
        "%s%s\n%sVersion %s\n\n\
        %sGamemode:%s eigenstaendiger Freeroam-Modus fuer SA-MP 0.3.7 / open.mp\n\
        %sPlugins:%s ausschliesslich Streamer (Incognito) fuer die Custom Map.\n\
        Kommandoparser, Parameter-Parsing und Datenspeicher sind Teil des Projekts.\n\n\
        %sWebseite: %s\n%sDiscord: %s",
        EC_GOLD, SERVER_NAME, EC_GREY, SERVER_VERSION,
        EC_YELLOW, EC_WHITE,
        EC_YELLOW, EC_WHITE,
        EC_CYAN, SERVER_WEBSITE, EC_CYAN, SERVER_DISCORD);

    ShowPlayerDialog(playerid, DIALOG_HELP, DIALOG_STYLE_MSGBOX,
        "Credits", body, "Schliessen", "");
    return 1;
}

// ---------------------------------------------------------------------------
// Statistiken
// ---------------------------------------------------------------------------
stock Stats_Show(playerid, targetid)
{
    new playtime[32], money[24], bank[24], body[1400];
    FormatDuration(GetTotalPlayTime(targetid), playtime, sizeof playtime);
    FormatNumber(GetPlayerMoney(targetid), money, sizeof money);
    FormatNumber(PlayerData[targetid][pBank], bank, sizeof bank);

    // Kill/Death-Verhaeltnis mit einer Nachkommastelle, ohne Division durch null
    new Float:ratio = (PlayerData[targetid][pDeaths] > 0)
        ? float(PlayerData[targetid][pKills]) / float(PlayerData[targetid][pDeaths])
        : float(PlayerData[targetid][pKills]);

    new houseName[48];
    new houseIndex = House_GetOwned(targetid);
    if (houseIndex == -1) houseName = "keins";
    else                  format(houseName, sizeof houseName, "Haus #%d", houseIndex + 1);

    format(body, sizeof body,
        "%sSpieler:%s %s (ID %d)\n\
        %sRang:%s %s\n\
        %sVIP:%s %s\n\n\
        %sKills:%s %d\n\
        %sTode:%s %d\n\
        %sVerhaeltnis:%s %.2f\n\
        %sPunkte:%s %d\n\
        %sGeld:%s $%s\n\n\
        %sStuntpunkte:%s %d\n\
        %sRennsiege:%s %d\n\
        %sDerbysiege:%s %d\n\
        %sBoost-Schuebe:%s %d\n\n\
        %sBankkonto:%s $%s\n\
        %sJobrouten:%s %d\n\
        %sErfolge:%s %d von %d\n\
        %sHaus:%s %s\n\n\
        %sSpielzeit:%s %s\n\
        %sRegistriert:%s %s",
        EC_GREY, EC_WHITE, GetName(targetid), targetid,
        EC_GREY, EC_ORANGE, GetAdminRankName(AdminLevel(targetid)),
        EC_GREY, EC_GOLD, GetVipRankName(VipLevel(targetid)),
        EC_GREY, EC_WHITE, PlayerData[targetid][pKills],
        EC_GREY, EC_WHITE, PlayerData[targetid][pDeaths],
        EC_GREY, EC_WHITE, ratio,
        EC_GREY, EC_WHITE, PlayerData[targetid][pScore],
        EC_GREY, EC_GREEN, money,
        EC_GREY, EC_CYAN, PlayerData[targetid][pStuntPoints],
        EC_GREY, EC_CYAN, PlayerData[targetid][pRaceWins],
        EC_GREY, EC_CYAN, PlayerData[targetid][pDerbyWins],
        EC_GREY, EC_CYAN, PlayerData[targetid][pBoostCount],
        EC_GREY, EC_GREEN, bank,
        EC_GREY, EC_CYAN, PlayerData[targetid][pJobsDone],
        EC_GREY, EC_CYAN, Achievement_Count(targetid), ACHIEVEMENT_COUNT,
        EC_GREY, EC_WHITE, houseName,
        EC_GREY, EC_WHITE, playtime,
        EC_GREY, EC_WHITE, PlayerData[targetid][pRegDate]);

    new caption[64];
    format(caption, sizeof caption, "Statistik - %s", GetName(targetid));

    return ShowPlayerDialog(playerid, DIALOG_STATS, DIALOG_STYLE_MSGBOX,
        caption, body, "Schliessen", "");
}

CMD:stats(playerid, params[])
{
    if (params[0] == EOS) return Stats_Show(playerid, playerid);

    new target = ResolveTarget(playerid, params);
    if (target == INVALID_PLAYER_ID) return 1;

    return Stats_Show(playerid, target);
}

CMD:players(playerid, params[])
{
    #pragma unused params

    new body[2048], entry[144], count = 0;

    foreach_player(i)
    {
        if (!IsLoggedIn(i)) continue;

        new tag[24];
        tag[0] = EOS;

        if (AdminLevel(i) > ADMIN_NONE)      format(tag, sizeof tag, " {E67E22}[%s]", GetAdminRankName(AdminLevel(i)));
        else if (IsVip(i))                   format(tag, sizeof tag, " {FFD700}[VIP]");

        new status[16];
        if (PlayerData[i][pAfk])             status = " {AFAFAF}(AFK)";
        else if (IsPlayerJailed(i))          status = " {E74C3C}(Jail)";
        else                                 status[0] = EOS;

        format(entry, sizeof entry, "{FFFFFF}%d - %s%s%s  {AFAFAF}%d Punkte\n",
            i, GetName(i), tag, status, PlayerData[i][pScore]);

        if (strlen(body) + strlen(entry) >= sizeof(body) - 64)
        {
            strcat(body, "{AFAFAF}...\n");
            break;
        }
        strcat(body, entry);
        count++;
    }

    new caption[64];
    format(caption, sizeof caption, "Spieler online (%d)", count);

    ShowPlayerDialog(playerid, DIALOG_TOP_LIST, DIALOG_STYLE_MSGBOX,
        caption, body, "Schliessen", "");
    return 1;
}

/*
 * Rangliste der Spieler, die gerade online sind. Eine serverweite Bestenliste
 * ueber alle Accounts wuerde bedeuten, bei jedem Aufruf saemtliche
 * Accountdateien zu lesen - das bleibt bewusst aussen vor.
 */
CMD:top(playerid, params[])
{
    #pragma unused params

    new ids[MAX_PLAYERS], count = 0;

    foreach_player(i)
    {
        if (IsLoggedIn(i)) ids[count++] = i;
    }

    if (count == 0)
    {
        SendInfo(playerid, "Gerade ist niemand eingeloggt.");
        return 1;
    }

    // Einfaches Insertion-Sort: bei maximal 1000 Eintraegen voellig ausreichend
    for (new i = 1; i < count; i++)
    {
        new current = ids[i], j = i - 1;

        while (j >= 0 && PlayerData[ids[j]][pScore] < PlayerData[current][pScore])
        {
            ids[j + 1] = ids[j];
            j--;
        }
        ids[j + 1] = current;
    }

    new body[1024], entry[128];
    new shown = (count < 10) ? count : 10;

    for (new i = 0; i < shown; i++)
    {
        new id = ids[i];
        format(entry, sizeof entry, "{F1C40F}%d. {FFFFFF}%s  {AFAFAF}%d Punkte, %d Kills, %d Stuntpunkte\n",
            i + 1, GetName(id), PlayerData[id][pScore],
            PlayerData[id][pKills], PlayerData[id][pStuntPoints]);
        strcat(body, entry);
    }

    ShowPlayerDialog(playerid, DIALOG_TOP_LIST, DIALOG_STYLE_MSGBOX,
        "Bestenliste (online)", body, "Schliessen", "");
    return 1;
}

// ---------------------------------------------------------------------------
// Kleinigkeiten
// ---------------------------------------------------------------------------
CMD:pos(playerid, params[])
{
    #pragma unused params

    new Float:x, Float:y, Float:z, msg[128];
    GetPlayerPos(playerid, x, y, z);
    format(msg, sizeof msg, "Position: %.2f, %.2f, %.2f | Interior %d | Welt %d",
        x, y, z, GetPlayerInterior(playerid), GetPlayerVirtualWorld(playerid));
    SendInfo(playerid, msg);
    return 1;
}

CMD:kill(playerid, params[])
{
    #pragma unused params
    require(PlayerData[playerid][pMinigame] == MINIGAME_NONE, "Waehrend eines Minispiels nicht moeglich - benutze /leave.");

    SetPlayerHealth(playerid, 0.0);
    SendInfo(playerid, "Du hast Selbstmord begangen.");
    return 1;
}

CMD:afk(playerid, params[])
{
    #pragma unused params

    if (PlayerData[playerid][pAfk])
    {
        MarkActivity(playerid);
        return 1;
    }

    PlayerData[playerid][pAfk] = true;

    new msg[96];
    format(msg, sizeof msg, "%s ist jetzt AFK.", GetName(playerid));
    SendClientMessageToAll(COL_GREY, msg);
    SendInfo(playerid, "Du bist als AFK markiert. Eine beliebige Aktion hebt das wieder auf.");
    return 1;
}

CMD:jailtime(playerid, params[])
{
    #pragma unused params
    require(IsPlayerJailed(playerid), "Du sitzt nicht im Jail.");

    new left[32], msg[96];
    FormatDuration(PlayerData[playerid][pJailUntil] - gettime(), left, sizeof left);
    format(msg, sizeof msg, "Verbleibende Jailzeit: %s", left);
    SendInfo(playerid, msg);
    return 1;
}

CMD:pay(playerid, params[])
{
    require(Params_Count(params) >= 2, "Benutzung: /pay [Spieler] [Betrag]");

    new arg[MAX_PLAYER_NAME + 1];
    Params_Get(params, 0, arg, sizeof arg);

    new target = ResolveTarget(playerid, arg);
    if (target == INVALID_PLAYER_ID) return 1;
    require(target != playerid, "Dir selbst Geld zu geben bringt wenig.");
    require(Params_IsInt(params, 1), "Der Betrag muss eine Zahl sein.");

    new amount = Params_Int(params, 1);
    require(amount > 0, "Der Betrag muss groesser als null sein.");
    require(GetPlayerMoney(playerid) >= amount, "So viel Geld hast du nicht.");
    require(GetPlayerDistanceToPlayer(playerid, target) < 10.0, "Der Spieler muss in deiner Naehe sein.");

    GivePlayerMoney(playerid, -amount);
    GivePlayerMoney(target, amount);

    new money[24], msg[144];
    FormatNumber(amount, money, sizeof money);

    format(msg, sizeof msg, "Du hast %s $%s gegeben.", GetName(target), money);
    SendSuccess(playerid, msg);

    format(msg, sizeof msg, "%s hat dir $%s gegeben.", GetName(playerid), money);
    SendSuccess(target, msg);

    format(msg, sizeof msg, "%s -> %s: $%s", GetName(playerid), GetName(target), money);
    Log(msg);
    return 1;
}

