/*
 * Jebiga-Gaming - Datenbankschicht
 *
 * Der Server laeuft wahlweise auf MySQL oder auf den INI-Dateien in
 * scriptfiles/. Die Wahl faellt beim Start anhand von scriptfiles/mysql.ini:
 * steht dort ein Benutzername, wird MySQL benutzt, sonst der Dateispeicher.
 * Scheitert der Verbindungsaufbau, faellt der Server ebenfalls auf Dateien
 * zurueck, statt den Start abzubrechen - ein Server ohne Datenbank ist besser
 * als gar kein Server.
 *
 * Einzurichten ist nichts weiter als Benutzer und Passwort. Datenbank und
 * Tabellen legt der Server beim Start selbst an:
 *
 *   1. Verbindung mit der eingetragenen Datenbank versuchen.
 *   2. Scheitert das, ohne Datenbank verbinden und CREATE DATABASE ausfuehren.
 *   3. Danach neu verbinden, diesmal mit der Datenbank.
 *   4. CREATE TABLE IF NOT EXISTS fuer alle Tabellen.
 *
 * Schritt 3 ist kein Umweg: ein blosses USE wuerde bei eingeschaltetem
 * AUTO_RECONNECT verloren gehen, weil der Client nach einem Verbindungsabriss
 * den Zustand vom Verbindungsaufbau wiederherstellt und nicht den, der spaeter
 * per USE gesetzt wurde. Die Datenbank muss also an der Verbindung selbst
 * haengen.
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
#define DB_MAX_ERROR            (224)

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
 * Legt eine Beispielkonfiguration an, wenn noch keine existiert.
 *
 * Benutzer und Passwort stehen bewusst ganz oben und leer da - das sind die
 * einzigen zwei Angaben, die der Betreiber machen muss. Alles darunter ist
 * fuer eine Datenbank auf demselben Rechner bereits richtig eingestellt.
 */
static DB_WriteSampleConfig()
{
    if (fexist(PATH_MYSQL_CONFIG)) return 0;
    if (!INI_Open(PATH_MYSQL_CONFIG)) return 0;

    INI_WriteRaw("; Jebiga-Gaming - Zugang zur Datenbank");
    INI_WriteRaw(";");
    INI_WriteRaw("; Hier nur Benutzer und Passwort eintragen. Die Datenbank");
    INI_WriteRaw("; und alle Tabellen legt der Server beim Start selbst an.");
    INI_WriteRaw(";");
    INI_WriteRaw("; Bleibt User leer, laeuft der Server ohne MySQL und");
    INI_WriteRaw("; speichert wie bisher in Textdateien unter scriptfiles/.");
    INI_WriteRaw("");

    INI_WriteString("User", "");
    INI_WriteString("Password", "");

    INI_WriteRaw("");
    INI_WriteRaw("; Ab hier nur aendern, wenn der Datenbankserver nicht auf");
    INI_WriteRaw("; demselben Rechner laeuft oder anders heissen soll.");

    INI_WriteString("Host", "127.0.0.1");
    INI_WriteInt("Port", 3306);
    INI_WriteString("Database", "jebiga");

    INI_Close();

    Log("scriptfiles/mysql.ini angelegt - User und Passwort eintragen, um MySQL zu nutzen.");
    return 1;
}

/*
 * Prueft, ob ein Name gefahrlos in einen Bezeichner eingesetzt werden kann.
 *
 * Der Datenbankname kommt aus einer Datei und landet in CREATE DATABASE.
 * Anders als Werte lassen sich Bezeichner nicht als Parameter uebergeben und
 * auch nicht mit mysql_format maskieren - sie muessen in die Abfrage
 * geschrieben werden. Also wird der Name vorher auf Buchstaben, Ziffern und
 * Unterstrich eingegrenzt; alles andere wird abgelehnt statt maskiert.
 */
static bool:DB_IsSafeIdentifier(const name[])
{
    if (name[0] == EOS) return false;

    for (new i = 0; name[i] != EOS; i++)
    {
        new c = name[i];

        if ((c >= 'a' && c <= 'z') ||
            (c >= 'A' && c <= 'Z') ||
            (c >= '0' && c <= '9') ||
            c == '_') continue;

        return false;
    }
    return true;
}

/*
 * Fuehrt eine Abfrage synchron aus und meldet Fehler ins Log.
 * Rueckgabe: true bei Erfolg.
 */
stock bool:DB_QuerySync(const query[])
{
    // Bewusst am Handle und nicht an gDbActive gemessen: waehrend des
    // Verbindungsaufbaus steht die Verbindung schon, gilt aber noch nicht als
    // einsatzbereit - CREATE DATABASE laeuft genau in diesem Fenster.
    if (gDbHandle == MYSQL_INVALID_HANDLE) return false;

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
 * Oeffnet eine Verbindung und legt sie in gDbHandle ab.
 *
 * Ein leerer Datenbankname verbindet ohne Datenbankauswahl - genau das wird
 * gebraucht, um die Datenbank ueberhaupt erst anlegen zu koennen.
 * Rueckgabe: true bei Erfolg, sonst steht der Grund in error.
 */
static bool:DB_Open(const host[], const user[], const password[],
                    const database[], port, error[], errorLen = sizeof error)
{
    error[0] = EOS;

    // Eigener Optionssatz, damit Port und automatische Neuverbindung gesetzt
    // werden koennen, ohne die Standardoptionen zu veraendern.
    new MySQLOpt:options = mysql_init_options();
    mysql_set_option(options, SERVER_PORT, port);
    mysql_set_option(options, AUTO_RECONNECT, true);

    new MySQL:handle = mysql_connect(host, user, password, database, options);

    if (handle == MYSQL_INVALID_HANDLE)
    {
        strcat(error, "Verbindung konnte nicht angelegt werden", errorLen);
        return false;
    }

    new errorId = mysql_errno(handle);
    if (errorId != 0)
    {
        new detail[160];
        mysql_error(detail, sizeof detail, handle);
        format(error, errorLen, "Fehler %d: %s", errorId, detail);

        mysql_close(handle);
        return false;
    }

    gDbHandle = handle;
    return true;
}

// Schliesst eine offene Verbindung und vergisst sie.
static DB_Drop()
{
    if (gDbHandle != MYSQL_INVALID_HANDLE) mysql_close(gDbHandle);
    gDbHandle = MYSQL_INVALID_HANDLE;
    return 1;
}

/*
 * Baut die Verbindung auf und legt Datenbank und Schema bei Bedarf an.
 * Rueckgabe: true, wenn ab jetzt MySQL benutzt wird.
 */
stock bool:DB_Connect()
{
    gDbActive = false;
    gDbHandle = MYSQL_INVALID_HANDLE;

    DB_WriteSampleConfig();

    new host[DB_MAX_HOST], user[DB_MAX_USER];
    new password[DB_MAX_PASSWORD], database[DB_MAX_DATABASE];

    INI_GetString(PATH_MYSQL_CONFIG, "Host", host, sizeof host);
    INI_GetString(PATH_MYSQL_CONFIG, "User", user, sizeof user);
    INI_GetString(PATH_MYSQL_CONFIG, "Password", password, sizeof password);
    INI_GetString(PATH_MYSQL_CONFIG, "Database", database, sizeof database);

    new port = INI_GetInt(PATH_MYSQL_CONFIG, "Port", 3306);

    // Ein leerer Benutzername ist der Ausschalter. Wer MySQL nicht benutzen
    // will, laesst die Zeile stehen wie sie ist - es gibt keinen zweiten
    // Schalter, der zusaetzlich umgelegt werden muesste.
    if (user[0] == EOS)
    {
        Log("[MYSQL] Kein Benutzer in scriptfiles/mysql.ini eingetragen - der Server benutzt den Dateispeicher in scriptfiles/.");
        return false;
    }

    // Fehlende Angaben mit den ueblichen Werten fuellen, statt abzubrechen.
    if (host[0] == EOS) host = "127.0.0.1";
    if (database[0] == EOS) database = "jebiga";
    if (port <= 0 || port > 65535) port = 3306;

    new msg[352];

    if (!DB_IsSafeIdentifier(database))
    {
        format(msg, sizeof msg, "[MYSQL] Datenbankname '%s' enthaelt unzulaessige Zeichen (erlaubt: A-Z, a-z, 0-9, _) - Dateispeicher wird benutzt.", database);
        Log(msg);
        return false;
    }

    new error[DB_MAX_ERROR];

    // Erster Versuch: direkt mit der Datenbank verbinden. Im Normalbetrieb ist
    // das der einzige Schritt, alles Weitere laeuft nur beim allerersten Start.
    if (!DB_Open(host, user, password, database, port, error))
    {
        format(msg, sizeof msg, "[MYSQL] Datenbank '%s' noch nicht vorhanden oder nicht erreichbar (%s) - sie wird jetzt angelegt.", database, error);
        Log(msg);

        // Zweiter Versuch: ohne Datenbankauswahl anmelden. Klappt auch das
        // nicht, liegt es an Zugangsdaten oder Erreichbarkeit, nicht an der
        // fehlenden Datenbank.
        if (!DB_Open(host, user, password, "", port, error))
        {
            format(msg, sizeof msg, "[MYSQL] Anmeldung als %s@%s:%d fehlgeschlagen (%s) - Dateispeicher wird benutzt.", user, host, port, error);
            Log(msg);
            return false;
        }

        new query[DB_MAX_QUERY];
        format(query, sizeof query,
            "CREATE DATABASE IF NOT EXISTS `%s` CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci",
            database);

        if (!DB_QuerySync(query))
        {
            format(msg, sizeof msg, "[MYSQL] '%s' konnte nicht angelegt werden - fehlt dem Benutzer %s das Recht CREATE? Dateispeicher wird benutzt.", database, user);
            Log(msg);
            DB_Drop();
            return false;
        }

        // Neu verbinden, damit die Datenbank an der Verbindung haengt und eine
        // automatische Neuverbindung sie nicht wieder verliert.
        DB_Drop();

        if (!DB_Open(host, user, password, database, port, error))
        {
            format(msg, sizeof msg, "[MYSQL] '%s' wurde angelegt, die Verbindung dorthin scheiterte aber (%s) - Dateispeicher wird benutzt.", database, error);
            Log(msg);
            return false;
        }

        format(msg, sizeof msg, "[MYSQL] Datenbank '%s' angelegt.", database);
        Log(msg);
    }

    gDbActive = true;
    mysql_set_charset("utf8mb4", gDbHandle);

    format(msg, sizeof msg, "[MYSQL] Verbunden mit %s@%s:%d, Datenbank %s.", user, host, port, database);
    Log(msg);

    DB_CreateSchema();
    return true;
}

stock DB_Close()
{
    if (!gDbActive) return 0;

    DB_Drop();
    gDbActive = false;
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
