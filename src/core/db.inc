/*
 * Jebiga-Gaming - Datenbankschicht
 *
 * Der Server laeuft wahlweise auf MySQL oder auf den INI-Dateien in
 * scriptfiles/. Die Wahl faellt beim Start anhand von scriptfiles/mysql.ini;
 * fehlt die Datei oder steht Enabled auf 0, bleibt alles beim Dateispeicher.
 * Scheitert der Verbindungsaufbau, faellt der Server ebenfalls auf Dateien
 * zurueck, statt den Start abzubrechen - ein Server ohne Datenbank ist besser
 * als gar kein Server.
 *
 * Das Schema legt der Server selbst an: DB_CreateSchema() fuehrt beim Start
 * CREATE TABLE IF NOT EXISTS fuer alle Tabellen aus. Eine leere, frisch
 * angelegte Datenbank genuegt also - Einspielen von Hand entfaellt.
 *
 * Zu synchron und asynchron
 * -------------------------
 * Schemaanlage und das Laden globaler Daten beim Start laufen synchron
 * (mysql_query). Das blockiert, passiert aber genau einmal beim Hochfahren,
 * bevor Spieler verbunden sind.
 *
 * Alles, was zur Laufzeit passiert - Account laden, speichern, Bans schreiben -
 * laeuft asynchron (mysql_tquery). Bei 700 Spielern wuerde eine blockierende
 * Abfrage sonst den gesamten Server anhalten.
 */

#if defined _jebiga_db_included
    #endinput
#endif
#define _jebiga_db_included

#define PATH_MYSQL_CONFIG       "mysql.ini"

#define DB_MAX_QUERY            (2048)
#define DB_MAX_HOST             (64)
#define DB_MAX_USER             (64)
#define DB_MAX_PASSWORD         (64)
#define DB_MAX_DATABASE         (64)

static MySQL:gDbHandle = MYSQL_INVALID_HANDLE;
static bool:gDbActive  = false;

stock bool:DB_IsActive()
{
    return gDbActive;
}

stock MySQL:DB_Handle()
{
    return gDbHandle;
}

/*
 * Legt eine Beispielkonfiguration an, wenn noch keine existiert. So sieht der
 * Betreiber beim ersten Start, welche Schluessel erwartet werden.
 */
static DB_WriteSampleConfig()
{
    if (fexist(PATH_MYSQL_CONFIG)) return 0;
    if (!INI_Open(PATH_MYSQL_CONFIG)) return 0;

    INI_WriteInt("Enabled", 0);
    INI_WriteString("Host", "127.0.0.1");
    INI_WriteString("User", "jebiga");
    INI_WriteString("Password", "");
    INI_WriteString("Database", "jebiga");
    INI_WriteInt("Port", 3306);
    INI_Close();

    Log("scriptfiles/mysql.ini angelegt. Enabled=1 setzen, um MySQL zu nutzen.");
    return 1;
}

/*
 * Fuehrt eine Abfrage synchron aus und meldet Fehler ins Log.
 * Rueckgabe: true bei Erfolg.
 */
stock bool:DB_QuerySync(const query[])
{
    if (!gDbActive) return false;

    new Cache:result = mysql_query(gDbHandle, query, false);

    new errorId = mysql_errno(gDbHandle);
    if (errorId != 0)
    {
        new error[192], msg[384];
        mysql_error(error, sizeof error, gDbHandle);
        format(msg, sizeof msg, "[MYSQL] Fehler %d: %s", errorId, error);
        Log(msg);

        if (result) cache_delete(result);
        return false;
    }

    if (result) cache_delete(result);
    return true;
}

/*
 * Legt alle Tabellen an, sofern sie fehlen.
 *
 * Die Spaltennamen entsprechen den Schluesseln der INI-Dateien, damit sich
 * beide Speicherwege ohne Umdenken vergleichen lassen. Namen werden ueber
 * einen eindeutigen Index gefuehrt - so kann derselbe Account nicht doppelt
 * entstehen, auch wenn zwei Abfragen gleichzeitig laufen.
 */
stock DB_CreateSchema()
{
    if (!gDbActive) return 0;

    new created = 0;

    // Pawn kann Stringliterale nicht ueber Zeilengrenzen fortsetzen, ohne dass
    // die Zeile selbst mit \ weitergefuehrt wird - daher die Fortsetzungen.
    if (DB_QuerySync(
        "CREATE TABLE IF NOT EXISTS `accounts` (" \
        "`id` INT UNSIGNED NOT NULL AUTO_INCREMENT," \
        "`name` VARCHAR(24) NOT NULL," \
        "`password` CHAR(64) NOT NULL," \
        "`salt` VARCHAR(16) NOT NULL," \
        "`regdate` VARCHAR(24) NOT NULL DEFAULT ''," \
        "`lastip` VARCHAR(16) NOT NULL DEFAULT ''," \
        "`adminlevel` TINYINT UNSIGNED NOT NULL DEFAULT 0," \
        "`viplevel` TINYINT UNSIGNED NOT NULL DEFAULT 0," \
        "`score` INT NOT NULL DEFAULT 0," \
        "`money` INT NOT NULL DEFAULT 0," \
        "`bank` INT NOT NULL DEFAULT 0," \
        "`kills` INT UNSIGNED NOT NULL DEFAULT 0," \
        "`deaths` INT UNSIGNED NOT NULL DEFAULT 0," \
        "`playtime` INT UNSIGNED NOT NULL DEFAULT 0," \
        "`skin` SMALLINT UNSIGNED NOT NULL DEFAULT 26," \
        "`stuntpoints` INT UNSIGNED NOT NULL DEFAULT 0," \
        "`racewins` INT UNSIGNED NOT NULL DEFAULT 0," \
        "`derbywins` INT UNSIGNED NOT NULL DEFAULT 0," \
        "`house` INT NOT NULL DEFAULT -1," \
        "`vehicledamage` TINYINT UNSIGNED NOT NULL DEFAULT 1," \
        "`jobsdone` INT UNSIGNED NOT NULL DEFAULT 0," \
        "`jobearnings` INT UNSIGNED NOT NULL DEFAULT 0," \
        "`achievements` INT UNSIGNED NOT NULL DEFAULT 0," \
        "`crew` VARCHAR(24) NOT NULL DEFAULT ''," \
        "PRIMARY KEY (`id`)," \
        "UNIQUE KEY `name` (`name`)" \
        ") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4")) created++;

    if (DB_QuerySync(
        "CREATE TABLE IF NOT EXISTS `bans` (" \
        "`id` INT UNSIGNED NOT NULL AUTO_INCREMENT," \
        "`name` VARCHAR(24) NOT NULL," \
        "`reason` VARCHAR(128) NOT NULL DEFAULT ''," \
        "`admin` VARCHAR(24) NOT NULL DEFAULT ''," \
        "`created` VARCHAR(24) NOT NULL DEFAULT ''," \
        "PRIMARY KEY (`id`)," \
        "UNIQUE KEY `name` (`name`)" \
        ") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4")) created++;

    if (DB_QuerySync(
        "CREATE TABLE IF NOT EXISTS `houses` (" \
        "`slot` INT UNSIGNED NOT NULL," \
        "`owner` VARCHAR(24) NOT NULL DEFAULT ''," \
        "PRIMARY KEY (`slot`)" \
        ") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4")) created++;

    if (DB_QuerySync(
        "CREATE TABLE IF NOT EXISTS `crews` (" \
        "`slot` INT UNSIGNED NOT NULL," \
        "`name` VARCHAR(24) NOT NULL," \
        "`tag` VARCHAR(6) NOT NULL DEFAULT ''," \
        "`leader` VARCHAR(24) NOT NULL DEFAULT ''," \
        "`score` INT NOT NULL DEFAULT 0," \
        "PRIMARY KEY (`slot`)," \
        "UNIQUE KEY `name` (`name`)" \
        ") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4")) created++;

    new msg[96];
    format(msg, sizeof msg, "[MYSQL] Schema geprueft: %d von 4 Tabellen bereit.", created);
    Log(msg);
    return created;
}

/*
 * Baut die Verbindung auf. Rueckgabe: true, wenn ab jetzt MySQL benutzt wird.
 */
stock bool:DB_Connect()
{
    gDbActive = false;
    gDbHandle = MYSQL_INVALID_HANDLE;

    DB_WriteSampleConfig();

    if (INI_GetInt(PATH_MYSQL_CONFIG, "Enabled") != 1)
    {
        Log("[MYSQL] Deaktiviert - der Server benutzt den Dateispeicher in scriptfiles/.");
        return false;
    }

    new host[DB_MAX_HOST], user[DB_MAX_USER];
    new password[DB_MAX_PASSWORD], database[DB_MAX_DATABASE];

    INI_GetString(PATH_MYSQL_CONFIG, "Host", host, sizeof host);
    INI_GetString(PATH_MYSQL_CONFIG, "User", user, sizeof user);
    INI_GetString(PATH_MYSQL_CONFIG, "Password", password, sizeof password);
    INI_GetString(PATH_MYSQL_CONFIG, "Database", database, sizeof database);

    new port = INI_GetInt(PATH_MYSQL_CONFIG, "Port", 3306);

    if (host[0] == EOS || user[0] == EOS || database[0] == EOS)
    {
        Log("[MYSQL] Konfiguration unvollstaendig (Host, User oder Database fehlt) - Dateispeicher wird benutzt.");
        return false;
    }

    // Eigener Optionssatz, damit Port und automatische Neuverbindung gesetzt
    // werden koennen, ohne die Standardoptionen zu veraendern.
    new MySQLOpt:options = mysql_init_options();
    mysql_set_option(options, SERVER_PORT, port);
    mysql_set_option(options, AUTO_RECONNECT, true);

    gDbHandle = mysql_connect(host, user, password, database, options);

    new errorId = mysql_errno(gDbHandle);
    if (gDbHandle == MYSQL_INVALID_HANDLE || errorId != 0)
    {
        new error[192], msg[320];
        mysql_error(error, sizeof error, gDbHandle);
        format(msg, sizeof msg, "[MYSQL] Verbindung zu %s fehlgeschlagen (Fehler %d: %s) - Dateispeicher wird benutzt.",
            host, errorId, error);
        Log(msg);

        gDbHandle = MYSQL_INVALID_HANDLE;
        return false;
    }

    gDbActive = true;
    mysql_set_charset("utf8mb4", gDbHandle);

    new msg[192];
    format(msg, sizeof msg, "[MYSQL] Verbunden mit %s@%s:%d, Datenbank %s.", user, host, port, database);
    Log(msg);

    DB_CreateSchema();
    return true;
}

stock DB_Close()
{
    if (!gDbActive) return 0;

    mysql_close(gDbHandle);
    gDbActive = false;
    gDbHandle = MYSQL_INVALID_HANDLE;
    Log("[MYSQL] Verbindung geschlossen.");
    return 1;
}

/*
 * Meldet Fehler fehlgeschlagener Abfragen. Das Plugin ruft den Callback von
 * sich aus auf, sobald eine Abfrage scheitert.
 */
forward OnQueryError(errorid, const error[], const callback[], const query[], MySQL:handle);
public OnQueryError(errorid, const error[], const callback[], const query[], MySQL:handle)
{
    #pragma unused handle

    new msg[512];
    format(msg, sizeof msg, "[MYSQL] Abfrage fehlgeschlagen (%d: %s) im Callback '%s'", errorid, error, callback);
    Log(msg);

    // Die Abfrage selbst getrennt loggen - sie kann sehr lang sein
    format(msg, sizeof msg, "[MYSQL] Betroffene Abfrage: %s", query);
    Log(msg);
    return 1;
}
