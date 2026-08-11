/*
 * Jebiga-Gaming - Animationen und Spassbefehle
 *
 * Der Animationsbrowser arbeitet auf src/data/animations.inc: 1716
 * Animationen aus 132 Bibliotheken. Ausgewaehlt wird in zwei Schritten -
 * erst die Bibliothek, dann die Animation - weil beides zusammen in keinen
 * SA-MP-Dialog passt.
 *
 * Die Tabelle ist nach Bibliothek sortiert. Dadurch bilden die Eintraege einer
 * Bibliothek einen zusammenhaengenden Bereich, den eine einzige Suche nach
 * Anfang und Ende findet - statt fuer jede Anzeige ueber alle 1716 Zeilen zu
 * laufen.
 */

#if defined _jebiga_extras_included
    #endinput
#endif
#define _jebiga_extras_included

// Ausgewaehlte Bibliothek steckt in den oberen Bits des Dialogkontexts
#define ANIMCTX_PACK(%0,%1)     (((%0) << 16) | (%1))
#define ANIMCTX_LIBRARY(%0)     ((%0) >> 16)
#define ANIMCTX_PAGE(%0)        ((%0) & 0xFFFF)

/*
 * Erster Eintrag einer Bibliothek, -1 wenn sie leer ist.
 */
static Anim_LibraryStart(library)
{
    for (new i = 0; i < ANIMATION_COUNT; i++)
    {
        if (!strcmp(gAnimationInfo[i][aiLibrary], gAnimationLibraries[library], true)) return i;
    }
    return -1;
}

static Anim_LibraryCount(library)
{
    new start = Anim_LibraryStart(library);
    if (start == -1) return 0;

    new count = 0;
    for (new i = start; i < ANIMATION_COUNT; i++)
    {
        if (strcmp(gAnimationInfo[i][aiLibrary], gAnimationLibraries[library], true) != 0) break;
        count++;
    }
    return count;
}

stock Anim_ShowLibraries(playerid)
{
    new list[2048], entry[48];

    for (new i = 0; i < ANIMATION_LIBRARIES; i++)
    {
        format(entry, sizeof entry, "%s%s %s(%d)\n",
            EC_WHITE, gAnimationLibraries[i], EC_GREY, Anim_LibraryCount(i));

        if (strlen(list) + strlen(entry) >= sizeof(list) - 8) break;
        strcat(list, entry);
    }

    return ShowPlayerDialog(playerid, DIALOG_ANIM_LIBRARY, DIALOG_STYLE_LIST,
        "Animationen - Bibliothek", list, "Weiter", "Abbrechen");
}

stock Anim_ShowPage(playerid, library, page)
{
    new total = Anim_LibraryCount(library);
    if (total == 0) return 0;

    new pages = (total + DIALOG_PAGE_SIZE - 1) / DIALOG_PAGE_SIZE;
    if (page < 0)      page = pages - 1;
    if (page >= pages) page = 0;

    PlayerData[playerid][pDialogContext] = ANIMCTX_PACK(library, page);

    new start = Anim_LibraryStart(library);
    new first = page * DIALOG_PAGE_SIZE;
    new last  = first + DIALOG_PAGE_SIZE;
    if (last > total) last = total;

    new list[2048], entry[48];
    for (new i = first; i < last; i++)
    {
        format(entry, sizeof entry, "%s%s\n", EC_WHITE, gAnimationInfo[start + i][aiName]);
        strcat(list, entry);
    }

    if (pages > 1)
    {
        strcat(list, EC_YELLOW "<< Vorherige Seite\n");
        strcat(list, EC_YELLOW ">> Naechste Seite\n");
    }

    new caption[80];
    format(caption, sizeof caption, "%s - Seite %d von %d",
        gAnimationLibraries[library], page + 1, pages);

    return ShowPlayerDialog(playerid, DIALOG_ANIM_LIST, DIALOG_STYLE_LIST,
        caption, list, "Abspielen", "Zurueck");
}

stock Anim_Play(playerid, index)
{
    if (index < 0 || index >= ANIMATION_COUNT) return 0;

    if (IsPlayerInAnyVehicle(playerid))
    {
        SendError(playerid, "Im Fahrzeug geht das nicht.");
        return 0;
    }

    /*
     * Der erste Aufruf laedt die Bibliothek beim Spieler nach und laeuft oft
     * noch ins Leere; der zweite spielt sie dann tatsaechlich ab. Das ist das
     * uebliche Vorgehen bei SA-MP.
     */
    ApplyAnimation(playerid, gAnimationInfo[index][aiLibrary], gAnimationInfo[index][aiName],
        4.1, 0, 0, 0, 0, 0, 1);
    ApplyAnimation(playerid, gAnimationInfo[index][aiLibrary], gAnimationInfo[index][aiName],
        4.1, 1, 0, 0, 1, 0, 1);

    new msg[112];
    format(msg, sizeof msg, "Animation: %s / %s  (beenden mit /stopanim)",
        gAnimationInfo[index][aiLibrary], gAnimationInfo[index][aiName]);
    SendInfo(playerid, msg);
    return 1;
}

stock Extras_OnDialogResponse(playerid, dialogid, response, listitem)
{
    switch (dialogid)
    {
        case DIALOG_ANIM_LIBRARY:
        {
            if (!response) return 1;
            Anim_ShowPage(playerid, listitem, 0);
            return 1;
        }
        case DIALOG_ANIM_LIST:
        {
            if (!response) { Anim_ShowLibraries(playerid); return 1; }

            new context = PlayerData[playerid][pDialogContext];
            new library = ANIMCTX_LIBRARY(context);
            new page    = ANIMCTX_PAGE(context);

            new total = Anim_LibraryCount(library);
            new first = page * DIALOG_PAGE_SIZE;
            new last  = first + DIALOG_PAGE_SIZE;
            if (last > total) last = total;
            new shown = last - first;

            if (listitem == shown)     { Anim_ShowPage(playerid, library, page - 1); return 1; }
            if (listitem == shown + 1) { Anim_ShowPage(playerid, library, page + 1); return 1; }

            Anim_Play(playerid, Anim_LibraryStart(library) + first + listitem);
            return 1;
        }
    }
    return 0;
}

// ---------------------------------------------------------------------------
// Kommandos
// ---------------------------------------------------------------------------
CMD:anims(playerid, params[])
{
    // Ohne Argument der Browser, mit Argument die Suche ueber alle Animationen
    if (params[0] == EOS) return Anim_ShowLibraries(playerid);

    new found = -1, matches = 0;
    for (new i = 0; i < ANIMATION_COUNT; i++)
    {
        if (strfind(gAnimationInfo[i][aiName], params, true) == -1) continue;
        found = i;
        matches++;
    }

    require(matches != 0, "Keine Animation mit diesem Namen. /anims zeigt alle.");

    if (matches > 1)
    {
        new msg[176], list[144], entry[40], shown = 0;
        for (new i = 0; i < ANIMATION_COUNT && shown < 5; i++)
        {
            if (strfind(gAnimationInfo[i][aiName], params, true) == -1) continue;
            format(entry, sizeof entry, "%s  ", gAnimationInfo[i][aiName]);
            strcat(list, entry);
            shown++;
        }
        format(msg, sizeof msg, "%d Treffer: %s", matches, list);
        SendInfo(playerid, msg);
        return 1;
    }

    Anim_Play(playerid, found);
    return 1;
}
ALIAS:anim(anims)

CMD:stopanim(playerid, params[])
{
    #pragma unused params
    ClearAnimations(playerid);
    SendSuccess(playerid, "Animation beendet.");
    return 1;
}

CMD:weaponinfo(playerid, params[])
{
    require(params[0] != EOS, "Benutzung: /weaponinfo [Waffen-ID oder Name]");

    new weapon = FindWeaponId(params);
    require(weapon != -2, "Mehrere Waffen passen - bitte genauer angeben.");
    require(weapon != -1, "Keine Waffe mit diesem Namen oder dieser ID.");

    new msg[144];
    format(msg, sizeof msg, "%s%s%s - ID %d, Inventarplatz %d",
        EC_WHITE, GetWeaponNameEx(weapon), EC_CYAN, weapon, GetWeaponSlot(weapon));
    SendInfo(playerid, msg);
    return 1;
}

/*
 * Schleudert den Spieler senkrecht nach oben - klassischer Freeroam-Unfug.
 * Der Fallschirm kommt gleich mit, sonst endet es jedes Mal toedlich.
 */
CMD:rocket(playerid, params[])
{
    #pragma unused params
    require(PlayerData[playerid][pMinigame] == MINIGAME_NONE, "Waehrend eines Minispiels nicht moeglich.");

    new Float:x, Float:y, Float:z;
    GetPlayerPos(playerid, x, y, z);
    SetPlayerPos(playerid, x, y, z + 60.0);
    GivePlayerWeapon(playerid, 46, 1);   // Fallschirm

    SendSuccess(playerid, "Viel Spass beim Fliegen - der Fallschirm liegt bereit.");
    return 1;
}

CMD:jetpack(playerid, params[])
{
    #pragma unused params
    require(IsVip(playerid) || IsAdmin(playerid, ADMIN_ADMIN), "Das Jetpack ist VIPs vorbehalten. Infos: /vip");
    require(PlayerData[playerid][pMinigame] == MINIGAME_NONE, "Waehrend eines Minispiels nicht moeglich.");

    SetPlayerSpecialAction(playerid, SPECIAL_ACTION_USEJETPACK);
    SendSuccess(playerid, "Jetpack aktiviert.");
    return 1;
}

CMD:parachute(playerid, params[])
{
    #pragma unused params
    GivePlayerWeapon(playerid, 46, 1);
    SendSuccess(playerid, "Fallschirm erhalten.");
    return 1;
}
ALIAS:fallschirm(parachute)

CMD:eject(playerid, params[])
{
    #pragma unused params
    require(IsPlayerInAnyVehicle(playerid), "Du sitzt in keinem Fahrzeug.");

    new Float:x, Float:y, Float:z;
    GetPlayerPos(playerid, x, y, z);
    RemovePlayerFromVehicle(playerid);
    SetPlayerPos(playerid, x, y, z + 5.0);
    GivePlayerWeapon(playerid, 46, 1);

    SendSuccess(playerid, "Schleudersitz ausgeloest.");
    return 1;
}
