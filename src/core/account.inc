/*
 * Jebiga-Gaming - Accountsystem
 *
 * Registrierung und Login ueber Dialoge. Passwoerter werden mit SHA256 und
 * einem zufaelligen Salt pro Account gehasht (SA-MP Native SHA256_PassHash),
 * das Klartextpasswort verlaesst den Dialog nie.
 *
 * Zwei Speicherwege
 * -----------------
 * Der Account liegt entweder in MySQL oder in scriptfiles/accounts/. Welcher
 * Weg gilt, entscheidet db.inc beim Start.
 *
 * Der Unterschied ist nicht nur das Ziel, sondern der Ablauf: Dateien liest
 * man sofort, MySQL antwortet erst spaeter. Damit der Rest des Gamemodes
 * davon nichts merken muss, ist das Laden in beiden Faellen *zweistufig*
 * modelliert - Account_BeginLoad() stoesst an, Account_AfterLoad() macht
 * weiter. Der Dateiweg ruft die zweite Stufe direkt auf, der MySQL-Weg aus
 * dem Abfrage-Callback heraus.
 */

#if defined _jebiga_account_included
    #endinput
#endif
#define _jebiga_account_included

/*
 * Wird nach erfolgreichem Login ausgeloest. Die Definition steht im
 * Hauptskript, damit auch Module reagieren koennen, die erst nach dem
 * Accountsystem eingebunden werden.
 */
forward OnPlayerLoggedIn(playerid);

stock Account_GetPath(playerid, dest[], len = sizeof dest)
{
    new name[MAX_PLAYER_NAME + 1];
    GetPlayerName(playerid, name, sizeof name);

    // Einheitlich klein schreiben, damit Gross-/Kleinschreibung keine
    // Zweitaccounts erzeugt.
    for (new i = 0; name[i] != EOS; i++)
    {
        if (name[i] >= 'A' && name[i] <= 'Z') name[i] += 32;
    }
    format(dest, len, PATH_ACCOUNTS, name);
    return strlen(dest);
}

static Account_GenerateSalt(dest[], len = sizeof dest)
{
    static const charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    new setLen = sizeof(charset) - 1;

    new i = 0;
    for (; i < len - 1; i++) dest[i] = charset[random(setLen)];
    dest[i] = EOS;
    return i;
}

stock Account_Hash(const password[], const salt[], dest[], len = sizeof dest)
{
    SHA256_PassHash(password, salt, dest, len);
    return strlen(dest);
}

// ---------------------------------------------------------------------------
// Dateispeicher
// ---------------------------------------------------------------------------
static bool:Account_LoadFromFile(playerid)
{
    new path[64];
    Account_GetPath(playerid, path, sizeof path);
    if (!fexist(path)) return false;

    INI_GetString(path, "Password", PlayerData[playerid][pPassword], MAX_PASSWORD_LEN);
    INI_GetString(path, "Salt",     PlayerData[playerid][pSalt],     MAX_SALT_LEN);
    INI_GetString(path, "RegDate",  PlayerData[playerid][pRegDate],  24);
    INI_GetString(path, "LastIP",   PlayerData[playerid][pLastIP],   16);
    INI_GetString(path, "Crew",     PlayerData[playerid][pCrewName], 24);

    PlayerData[playerid][pAdminLevel]    = INI_GetInt(path, "AdminLevel");
    PlayerData[playerid][pVipLevel]      = INI_GetInt(path, "VipLevel");
    PlayerData[playerid][pScore]         = INI_GetInt(path, "Score");
    PlayerData[playerid][pMoney]         = INI_GetInt(path, "Money");
    PlayerData[playerid][pKills]         = INI_GetInt(path, "Kills");
    PlayerData[playerid][pDeaths]        = INI_GetInt(path, "Deaths");
    PlayerData[playerid][pPlayTime]      = INI_GetInt(path, "PlayTime");
    PlayerData[playerid][pSkin]          = INI_GetInt(path, "Skin", 26);
    PlayerData[playerid][pStuntPoints]   = INI_GetInt(path, "StuntPoints");
    PlayerData[playerid][pRaceWins]      = INI_GetInt(path, "RaceWins");
    PlayerData[playerid][pDerbyWins]     = INI_GetInt(path, "DerbyWins");
    PlayerData[playerid][pHouse]         = INI_GetInt(path, "House", -1);
    PlayerData[playerid][pVehicleDamage] = bool:INI_GetInt(path, "VehicleDamage", 1);
    PlayerData[playerid][pBank]          = INI_GetInt(path, "Bank");
    PlayerData[playerid][pJobsDone]      = INI_GetInt(path, "JobsDone");
    PlayerData[playerid][pJobEarnings]   = INI_GetInt(path, "JobEarnings");
    PlayerData[playerid][pAchievements]  = INI_GetInt(path, "Achievements");
    return true;
}

static Account_SaveToFile(playerid)
{
    new path[64];
    Account_GetPath(playerid, path, sizeof path);

    if (!INI_Open(path))
    {
        new msg[96];
        format(msg, sizeof msg, "FEHLER: Account von %s konnte nicht gespeichert werden.", GetName(playerid));
        Log(msg);
        return 0;
    }

    INI_WriteString("Password",      PlayerData[playerid][pPassword]);
    INI_WriteString("Salt",          PlayerData[playerid][pSalt]);
    INI_WriteString("RegDate",       PlayerData[playerid][pRegDate]);
    INI_WriteString("LastIP",        PlayerData[playerid][pLastIP]);
    INI_WriteString("Crew",          PlayerData[playerid][pCrewName]);
    INI_WriteInt("AdminLevel",       PlayerData[playerid][pAdminLevel]);
    INI_WriteInt("VipLevel",         PlayerData[playerid][pVipLevel]);
    INI_WriteInt("Score",            PlayerData[playerid][pScore]);
    INI_WriteInt("Money",            PlayerData[playerid][pMoney]);
    INI_WriteInt("Kills",            PlayerData[playerid][pKills]);
    INI_WriteInt("Deaths",           PlayerData[playerid][pDeaths]);
    INI_WriteInt("PlayTime",         PlayerData[playerid][pPlayTime]);
    INI_WriteInt("Skin",             PlayerData[playerid][pSkin]);
    INI_WriteInt("StuntPoints",      PlayerData[playerid][pStuntPoints]);
    INI_WriteInt("RaceWins",         PlayerData[playerid][pRaceWins]);
    INI_WriteInt("DerbyWins",        PlayerData[playerid][pDerbyWins]);
    INI_WriteInt("House",            PlayerData[playerid][pHouse]);
    INI_WriteInt("VehicleDamage",    PlayerData[playerid][pVehicleDamage] ? 1 : 0);
    INI_WriteInt("Bank",             PlayerData[playerid][pBank]);
    INI_WriteInt("JobsDone",         PlayerData[playerid][pJobsDone]);
    INI_WriteInt("JobEarnings",      PlayerData[playerid][pJobEarnings]);
    INI_WriteInt("Achievements",     PlayerData[playerid][pAchievements]);
    INI_Close();
    return 1;
}

// ---------------------------------------------------------------------------
// Dialoge
// ---------------------------------------------------------------------------
stock Account_ShowRegister(playerid)
{
    new caption[64], body[512];
    format(caption, sizeof caption, "%s - Registrierung", SERVER_NAME);
    format(body, sizeof body,
        "%sWillkommen auf %s%s%s!\n\n\
        Der Name %s%s%s ist noch nicht registriert.\n\
        Waehle ein Passwort, um deinen Account anzulegen.\n\n\
        %sMindestens %d Zeichen.",
        EC_WHITE, EC_GOLD, SERVER_NAME, EC_WHITE,
        EC_CYAN, GetName(playerid), EC_WHITE,
        EC_GREY, MIN_PASSWORD_LEN);

    return ShowPlayerDialog(playerid, DIALOG_REGISTER, DIALOG_STYLE_PASSWORD,
        caption, body, "Registrieren", "Verlassen");
}

stock Account_ShowLogin(playerid, const extra[] = "")
{
    new caption[64], body[512];
    format(caption, sizeof caption, "%s - Login", SERVER_NAME);
    format(body, sizeof body,
        "%sWillkommen zurueck, %s%s%s!\n\n\
        Bitte gib dein Passwort ein.\n\
        Letzte IP: %s%s\n%s",
        EC_WHITE, EC_CYAN, GetName(playerid), EC_WHITE,
        EC_GREY, PlayerData[playerid][pLastIP], extra);

    return ShowPlayerDialog(playerid, DIALOG_LOGIN, DIALOG_STYLE_PASSWORD,
        caption, body, "Login", "Verlassen");
}

// ---------------------------------------------------------------------------
// Laden (zweistufig, siehe Kopfkommentar)
// ---------------------------------------------------------------------------

/*
 * Zweite Stufe: die Daten stehen (oder es gibt keine). Ab hier ist der Ablauf
 * fuer beide Speicherwege identisch.
 */
static Account_AfterLoad(playerid, bool:exists)
{
    if (!IsPlayerConnected(playerid)) return 0;

    PlayerData[playerid][pRegistered] = exists;

    if (exists) Account_ShowLogin(playerid);
    else        Account_ShowRegister(playerid);
    return 1;
}

/*
 * Ergebnis der MySQL-Abfrage. Das Plugin ruft den Callback auf, sobald die
 * Antwort da ist - der Spieler kann inzwischen wieder weg sein.
 */
forward OnAccountLoaded(playerid);
public OnAccountLoaded(playerid)
{
    if (!IsPlayerConnected(playerid)) return 0;

    new rows;
    cache_get_row_count(rows);

    if (rows == 0)
    {
        Account_AfterLoad(playerid, false);
        return 1;
    }

    cache_get_value_name(0, "password", PlayerData[playerid][pPassword], MAX_PASSWORD_LEN);
    cache_get_value_name(0, "salt",     PlayerData[playerid][pSalt],     MAX_SALT_LEN);
    cache_get_value_name(0, "regdate",  PlayerData[playerid][pRegDate],  24);
    cache_get_value_name(0, "lastip",   PlayerData[playerid][pLastIP],   16);
    cache_get_value_name(0, "crew",     PlayerData[playerid][pCrewName], 24);

    cache_get_value_name_int(0, "adminlevel",   PlayerData[playerid][pAdminLevel]);
    cache_get_value_name_int(0, "viplevel",     PlayerData[playerid][pVipLevel]);
    cache_get_value_name_int(0, "score",        PlayerData[playerid][pScore]);
    cache_get_value_name_int(0, "money",        PlayerData[playerid][pMoney]);
    cache_get_value_name_int(0, "bank",         PlayerData[playerid][pBank]);
    cache_get_value_name_int(0, "kills",        PlayerData[playerid][pKills]);
    cache_get_value_name_int(0, "deaths",       PlayerData[playerid][pDeaths]);
    cache_get_value_name_int(0, "playtime",     PlayerData[playerid][pPlayTime]);
    cache_get_value_name_int(0, "skin",         PlayerData[playerid][pSkin]);
    cache_get_value_name_int(0, "stuntpoints",  PlayerData[playerid][pStuntPoints]);
    cache_get_value_name_int(0, "racewins",     PlayerData[playerid][pRaceWins]);
    cache_get_value_name_int(0, "derbywins",    PlayerData[playerid][pDerbyWins]);
    cache_get_value_name_int(0, "house",        PlayerData[playerid][pHouse]);
    cache_get_value_name_int(0, "jobsdone",     PlayerData[playerid][pJobsDone]);
    cache_get_value_name_int(0, "jobearnings",  PlayerData[playerid][pJobEarnings]);
    cache_get_value_name_int(0, "achievements", PlayerData[playerid][pAchievements]);

    new damage = 1;
    cache_get_value_name_int(0, "vehicledamage", damage);
    PlayerData[playerid][pVehicleDamage] = (damage != 0);

    Account_AfterLoad(playerid, true);
    return 1;
}

/*
 * Erste Stufe: Ladevorgang anstossen.
 */
stock Account_BeginLoad(playerid)
{
    if (!DB_IsActive())
    {
        Account_AfterLoad(playerid, Account_LoadFromFile(playerid));
        return 1;
    }

    new query[DB_MAX_QUERY];
    mysql_format(DB_Handle(), query, sizeof query,
        "SELECT * FROM `accounts` WHERE `name` = '%e' LIMIT 1", GetName(playerid));
    mysql_tquery(DB_Handle(), query, "OnAccountLoaded", "d", playerid);
    return 1;
}

// ---------------------------------------------------------------------------
// Speichern
// ---------------------------------------------------------------------------
stock Account_Save(playerid)
{
    if (!PlayerData[playerid][pRegistered]) return 0;

    // Laufende Sitzung in die Gesamtspielzeit uebernehmen
    if (PlayerData[playerid][pLoggedIn])
    {
        PlayerData[playerid][pPlayTime] = GetTotalPlayTime(playerid);
        PlayerData[playerid][pSessionStart] = gettime();

        PlayerData[playerid][pMoney] = GetPlayerMoney(playerid);
        PlayerData[playerid][pScore] = GetPlayerScore(playerid);
    }

    if (!DB_IsActive()) return Account_SaveToFile(playerid);

    new query[DB_MAX_QUERY];
    mysql_format(DB_Handle(), query, sizeof query,
        "UPDATE `accounts` SET `lastip`='%e',`adminlevel`=%d,`viplevel`=%d,`score`=%d," \
        "`money`=%d,`bank`=%d,`kills`=%d,`deaths`=%d,`playtime`=%d,`skin`=%d," \
        "`stuntpoints`=%d,`racewins`=%d,`derbywins`=%d,`house`=%d,`vehicledamage`=%d," \
        "`jobsdone`=%d,`jobearnings`=%d,`achievements`=%d,`crew`='%e' WHERE `name`='%e'",
        PlayerData[playerid][pLastIP],
        PlayerData[playerid][pAdminLevel],
        PlayerData[playerid][pVipLevel],
        PlayerData[playerid][pScore],
        PlayerData[playerid][pMoney],
        PlayerData[playerid][pBank],
        PlayerData[playerid][pKills],
        PlayerData[playerid][pDeaths],
        PlayerData[playerid][pPlayTime],
        PlayerData[playerid][pSkin],
        PlayerData[playerid][pStuntPoints],
        PlayerData[playerid][pRaceWins],
        PlayerData[playerid][pDerbyWins],
        PlayerData[playerid][pHouse],
        PlayerData[playerid][pVehicleDamage] ? 1 : 0,
        PlayerData[playerid][pJobsDone],
        PlayerData[playerid][pJobEarnings],
        PlayerData[playerid][pAchievements],
        PlayerData[playerid][pCrewName],
        GetName(playerid));

    mysql_tquery(DB_Handle(), query);
    return 1;
}

// ---------------------------------------------------------------------------
// Ablauf
// ---------------------------------------------------------------------------
stock Account_OnConnect(playerid)
{
    GetPlayerIp(playerid, PlayerData[playerid][pLastIP], 16);
    Account_BeginLoad(playerid);
    return 1;
}

stock Account_Register(playerid, const password[])
{
    if (strlen(password) < MIN_PASSWORD_LEN)
    {
        new note[128];
        format(note, sizeof note, "\n%sDas Passwort braucht mindestens %d Zeichen.",
            EC_RED, MIN_PASSWORD_LEN);

        new caption[64], body[512];
        format(caption, sizeof caption, "%s - Registrierung", SERVER_NAME);
        format(body, sizeof body, "%sWaehle ein Passwort fuer %s%s%s.%s",
            EC_WHITE, EC_CYAN, GetName(playerid), EC_WHITE, note);

        ShowPlayerDialog(playerid, DIALOG_REGISTER, DIALOG_STYLE_PASSWORD,
            caption, body, "Registrieren", "Verlassen");
        return 0;
    }

    Account_GenerateSalt(PlayerData[playerid][pSalt], MAX_SALT_LEN);
    Account_Hash(password, PlayerData[playerid][pSalt],
        PlayerData[playerid][pPassword], MAX_PASSWORD_LEN);

    GetTimestamp(PlayerData[playerid][pRegDate], 24);
    GetPlayerIp(playerid, PlayerData[playerid][pLastIP], 16);

    PlayerData[playerid][pRegistered] = true;
    PlayerData[playerid][pMoney]      = 25000;
    PlayerData[playerid][pScore]      = 0;

    if (DB_IsActive())
    {
        // Der eindeutige Index auf `name` verhindert Doppelanlagen, falls
        // derselbe Name zweimal fast gleichzeitig registriert wird.
        new query[DB_MAX_QUERY];
        mysql_format(DB_Handle(), query, sizeof query,
            "INSERT IGNORE INTO `accounts` (`name`,`password`,`salt`,`regdate`,`lastip`,`money`,`skin`) " \
            "VALUES ('%e','%e','%e','%e','%e',%d,%d)",
            GetName(playerid),
            PlayerData[playerid][pPassword],
            PlayerData[playerid][pSalt],
            PlayerData[playerid][pRegDate],
            PlayerData[playerid][pLastIP],
            PlayerData[playerid][pMoney],
            PlayerData[playerid][pSkin]);
        mysql_tquery(DB_Handle(), query);
    }
    else
    {
        Account_SaveToFile(playerid);
    }

    new msg[128];
    format(msg, sizeof msg, "Account %s registriert (IP %s).",
        GetName(playerid), PlayerData[playerid][pLastIP]);
    Log(msg);

    SendSuccess(playerid, "Account erfolgreich angelegt. Viel Spass auf " SERVER_NAME "!");
    Account_CompleteLogin(playerid);
    return 1;
}

stock Account_TryLogin(playerid, const password[])
{
    new hash[MAX_PASSWORD_LEN];
    Account_Hash(password, PlayerData[playerid][pSalt], hash, sizeof hash);

    if (strcmp(hash, PlayerData[playerid][pPassword]) != 0)
    {
        PlayerData[playerid][pLoginTries]++;

        if (PlayerData[playerid][pLoginTries] >= MAX_LOGIN_ATTEMPTS)
        {
            new msg[128];
            format(msg, sizeof msg, "%s wurde nach %d Fehlversuchen gekickt.",
                GetName(playerid), MAX_LOGIN_ATTEMPTS);
            Log(msg);

            SendError(playerid, "Zu viele Fehlversuche. Du wirst vom Server getrennt.");
            KickDelayed(playerid);
            return 0;
        }

        new note[160];
        format(note, sizeof note, "\n%sFalsches Passwort. Versuch %d von %d.",
            EC_RED, PlayerData[playerid][pLoginTries], MAX_LOGIN_ATTEMPTS);
        Account_ShowLogin(playerid, note);
        return 0;
    }

    GetPlayerIp(playerid, PlayerData[playerid][pLastIP], 16);
    Account_CompleteLogin(playerid);
    return 1;
}

stock Account_CompleteLogin(playerid)
{
    PlayerData[playerid][pLoggedIn]     = true;
    PlayerData[playerid][pSessionStart] = gettime();
    PlayerData[playerid][pLoginTries]   = 0;

    ResetPlayerMoney(playerid);
    GivePlayerMoney(playerid, PlayerData[playerid][pMoney]);
    SetPlayerScore(playerid, PlayerData[playerid][pScore]);
    SetPlayerColor(playerid, GetPlayerRankColor(playerid));

    new msg[144], playtime[32];
    FormatDuration(PlayerData[playerid][pPlayTime], playtime, sizeof playtime);

    format(msg, sizeof msg, "Eingeloggt. Gesamtspielzeit: %s | Kills: %d | Tode: %d",
        playtime, PlayerData[playerid][pKills], PlayerData[playerid][pDeaths]);
    SendInfo(playerid, msg);
    SendInfo(playerid, "Tippe " EC_YELLOW "/help" EC_CYAN " fuer eine Uebersicht aller Kommandos.");

    if (AdminLevel(playerid) > ADMIN_NONE)
    {
        format(msg, sizeof msg, "Du bist als %s eingeloggt. " EC_YELLOW "/acmds" EC_CYAN " zeigt deine Kommandos.",
            GetAdminRankName(AdminLevel(playerid)));
        SendInfo(playerid, msg);
    }
    if (IsVip(playerid))
    {
        format(msg, sizeof msg, "%s-Status aktiv. " EC_YELLOW "/vip" EC_CYAN " zeigt deine Vorteile.",
            GetVipRankName(VipLevel(playerid)));
        SendInfo(playerid, msg);
    }

    format(msg, sizeof msg, "%s hat den Server betreten.", GetName(playerid));
    SendClientMessageToAll(COL_GREY, msg);

    OnPlayerLoggedIn(playerid);

    SetSpawnInfo(playerid, NO_TEAM, PlayerData[playerid][pSkin],
        1958.3783, 1343.1572, 15.3746, 270.1425, 0, 0, 0, 0, 0, 0);
    SpawnPlayer(playerid);
    return 1;
}

stock Account_OnDisconnect(playerid)
{
    if (PlayerData[playerid][pLoggedIn]) Account_Save(playerid);
    return 1;
}

/*
 * Dialogantworten des Accountsystems. Gibt 1 zurueck, wenn der Dialog
 * verarbeitet wurde.
 */
stock Account_OnDialogResponse(playerid, dialogid, response, const inputtext[])
{
    switch (dialogid)
    {
        case DIALOG_REGISTER:
        {
            if (!response) { KickDelayed(playerid); return 1; }
            Account_Register(playerid, inputtext);
            return 1;
        }
        case DIALOG_LOGIN:
        {
            if (!response) { KickDelayed(playerid); return 1; }
            Account_TryLogin(playerid, inputtext);
            return 1;
        }
    }
    return 0;
}
