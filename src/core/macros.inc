/*
 * Jebiga-Gaming - Makros und Kommando-Dispatcher
 *
 * Ein eigener, abhaengigkeitsfreier Ersatz fuer zcmd + sscanf. Kommandos werden
 * als public-Funktionen mit dem Praefix "cmd_" angelegt und aus
 * OnPlayerCommandText per CallLocalFunction aufgerufen. Dadurch braucht der
 * Server weder Plugins noch externe Includes.
 */

#if defined _jebiga_macros_included
    #endinput
#endif
#define _jebiga_macros_included

// ---------------------------------------------------------------------------
// Kommando-Definition
//
//   CMD:heal(playerid, params[]) { ... return 1; }
//
// Ein Kommando muss 1 zurueckgeben. Gibt es 0 zurueck, behandelt der Dispatcher
// es wie ein unbekanntes Kommando (gleiche Konvention wie zcmd).
// ---------------------------------------------------------------------------
#define CMD:%1(%2) \
    forward cmd_%1(%2); \
    public cmd_%1(%2)

// Zweitname fuer ein bestehendes Kommando: ALIAS:tp(teleport)
// Ohne abschliessendes Semikolon - der Pawn-Praeprozessor kann ein solches
// Muster nicht zuverlaessig aufloesen.
#define ALIAS:%1(%2) \
    forward cmd_%1(playerid, params[]); \
    public cmd_%1(playerid, params[]) { return cmd_%2(playerid, params); }

// ---------------------------------------------------------------------------
// Schleifen- und Pruef-Kurzformen
// ---------------------------------------------------------------------------
#define foreach_player(%1) \
    for (new %1 = 0, __max = GetPlayerPoolSize(); %1 <= __max; %1++) \
        if (IsPlayerConnected(%1) && !IsPlayerNPC(%1))

// Frueher Ausstieg mit Fehlermeldung. Bewusst ohne Block, damit das
// abschliessende Semikolon an der Aufrufstelle das Statement beendet.
#define require(%1,%2) \
    if (!(%1)) return SendError(playerid, %2)

// ---------------------------------------------------------------------------
// Parameter-Parser (Ersatz fuer sscanf)
//
// Params_Count(params)          - Anzahl der durch Leerzeichen getrennten Worte
// Params_Get(params, i, d, len) - Wort i als String
// Params_Int(params, i)         - Wort i als Zahl (0 wenn keine Zahl)
// Params_Float(params, i)       - Wort i als Float
// Params_Rest(params, i, d,len) - Wort i bis Ende als String
// ---------------------------------------------------------------------------
stock Params_Count(const params[])
{
    new count = 0, i = 0, bool:inWord = false;
    while (params[i] != EOS)
    {
        if (params[i] == ' ')
        {
            inWord = false;
        }
        else if (!inWord)
        {
            inWord = true;
            count++;
        }
        i++;
    }
    return count;
}

stock Params_Get(const params[], index, dest[], len = sizeof dest)
{
    new i = 0, word = 0, pos;
    dest[0] = EOS;

    while (params[i] == ' ') i++;

    while (params[i] != EOS)
    {
        if (word == index)
        {
            pos = 0;
            while (params[i] != EOS && params[i] != ' ' && pos < len - 1)
            {
                dest[pos++] = params[i++];
            }
            dest[pos] = EOS;
            return pos;
        }
        // Aktuelles Wort ueberspringen
        while (params[i] != EOS && params[i] != ' ') i++;
        while (params[i] == ' ') i++;
        word++;
    }
    return 0;
}

stock Params_Rest(const params[], index, dest[], len = sizeof dest)
{
    new i = 0, word = 0, pos = 0;
    dest[0] = EOS;

    while (params[i] == ' ') i++;

    while (params[i] != EOS && word < index)
    {
        while (params[i] != EOS && params[i] != ' ') i++;
        while (params[i] == ' ') i++;
        word++;
    }
    while (params[i] != EOS && pos < len - 1)
    {
        dest[pos++] = params[i++];
    }
    dest[pos] = EOS;
    return pos;
}

stock Params_Int(const params[], index)
{
    new buf[24];
    if (!Params_Get(params, index, buf, sizeof buf)) return 0;
    return strval(buf);
}


stock Float:Params_Float(const params[], index)
{
    new buf[24];
    if (!Params_Get(params, index, buf, sizeof buf)) return 0.0;
    return floatstr(buf);
}

// Prueft, ob Wort <index> eine (auch negative) Ganzzahl ist
stock bool:Params_IsInt(const params[], index)
{
    new buf[24];
    if (!Params_Get(params, index, buf, sizeof buf)) return false;

    new i = (buf[0] == '-') ? 1 : 0;
    if (buf[i] == EOS) return false;

    for (; buf[i] != EOS; i++)
    {
        if (buf[i] < '0' || buf[i] > '9') return false;
    }
    return true;
}
