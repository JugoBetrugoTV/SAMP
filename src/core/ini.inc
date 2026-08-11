/*
 * Jebiga-Gaming - Minimaler INI-Speicher
 *
 * Schlanker key=value Dateispeicher auf Basis der Pawn-Dateifunktionen. Damit
 * kommt der Server ohne MySQL-Plugin aus; Accounts, Bans und Haeuser liegen
 * als lesbare Textdateien in scriptfiles/.
 *
 * Lesen:    INI_GetString(datei, "Key", buf); INI_GetInt(datei, "Key");
 * Schreiben: INI_Open(datei); INI_WriteInt("Key", 5); ... INI_Close();
 *            (schreibt die Datei jedes Mal komplett neu)
 */

#if defined _jebiga_ini_included
    #endinput
#endif
#define _jebiga_ini_included

#define INI_MAX_LINE            (192)

static File:gIniHandle = File:0;

// ---------------------------------------------------------------------------
// Schreiben
// ---------------------------------------------------------------------------
stock bool:INI_Open(const file[])
{
    if (gIniHandle) fclose(gIniHandle);

    gIniHandle = fopen(file, io_write);
    return (gIniHandle != File:0);
}

stock INI_WriteString(const key[], const value[])
{
    if (!gIniHandle) return 0;

    new line[INI_MAX_LINE];
    format(line, sizeof line, "%s=%s\r\n", key, value);
    return fwrite(gIniHandle, line);
}

stock INI_WriteInt(const key[], value)
{
    new buf[24];
    format(buf, sizeof buf, "%d", value);
    return INI_WriteString(key, buf);
}

stock INI_WriteFloat(const key[], Float:value)
{
    new buf[32];
    format(buf, sizeof buf, "%.4f", value);
    return INI_WriteString(key, buf);
}

stock INI_Close()
{
    if (!gIniHandle) return 0;

    fclose(gIniHandle);
    gIniHandle = File:0;
    return 1;
}

// ---------------------------------------------------------------------------
// Lesen
// ---------------------------------------------------------------------------

// Schneidet fuehrende/abschliessende Leerzeichen sowie CR/LF ab.
stock INI_Trim(string[])
{
    new len = strlen(string);

    while (len > 0 && (string[len - 1] == '\r' || string[len - 1] == '\n' ||
                       string[len - 1] == ' '  || string[len - 1] == '\t'))
    {
        string[--len] = EOS;
    }

    new start = 0;
    while (string[start] == ' ' || string[start] == '\t') start++;

    if (start > 0) strdel(string, 0, start);
    return strlen(string);
}

stock bool:INI_GetString(const file[], const key[], dest[], len = sizeof dest)
{
    dest[0] = EOS;

    new File:handle = fopen(file, io_read);
    if (!handle) return false;

    new line[INI_MAX_LINE], name[64], keyLen = strlen(key);
    new bool:found = false;

    while (fread(handle, line, sizeof line))
    {
        INI_Trim(line);
        if (line[0] == EOS || line[0] == ';' || line[0] == '#') continue;

        new sep = strfind(line, "=");
        if (sep == -1) continue;

        strmid(name, line, 0, (sep < sizeof name) ? sep : sizeof name - 1, sizeof name);
        INI_Trim(name);

        if (strlen(name) == keyLen && !strcmp(name, key, true))
        {
            strmid(dest, line, sep + 1, strlen(line), len);
            INI_Trim(dest);
            found = true;
            break;
        }
    }

    fclose(handle);
    return found;
}

stock INI_GetInt(const file[], const key[], defaultValue = 0)
{
    new buf[24];
    if (!INI_GetString(file, key, buf, sizeof buf)) return defaultValue;
    return strval(buf);
}

stock Float:INI_GetFloat(const file[], const key[], Float:defaultValue = 0.0)
{
    new buf[32];
    if (!INI_GetString(file, key, buf, sizeof buf)) return defaultValue;
    return floatstr(buf);
}

stock bool:INI_HasKey(const file[], const key[])
{
    new buf[8];
    return INI_GetString(file, key, buf, sizeof buf);
}
