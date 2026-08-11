/*
 * Jebiga-Gaming - Skinauswahl
 *
 * Zugriff auf alle 312 Skins von San Andreas, nach Geschlecht gefiltert und
 * seitenweise blaetterbar. Die Namen stammen aus src/data/skins.inc.
 *
 * Warum geblaettert wird: ein SA-MP-Dialog fasst gut 4000 Zeichen. 312
 * Eintraege mit Namen sprengen das deutlich, deshalb zeigt die Liste jeweils
 * DIALOG_PAGE_SIZE Eintraege und haengt Navigationszeilen an.
 */

#if defined _jebiga_skinshop_included
    #endinput
#endif
#define _jebiga_skinshop_included

#define SKIN_PRICE              (2500)

#define SKINFILTER_ALL          (0)
#define SKINFILTER_MALE         (1)
#define SKINFILTER_FEMALE       (2)

// Der Filter steckt in den oberen Bits des Dialogkontexts, die Seite in den
// unteren - so kommt die Auswahl ohne zusaetzliches Spielerfeld aus.
#define SKINCTX_PACK(%0,%1)     (((%0) << 16) | (%1))
#define SKINCTX_FILTER(%0)      ((%0) >> 16)
#define SKINCTX_PAGE(%0)        ((%0) & 0xFFFF)

static bool:Skin_MatchesFilter(skin, filter)
{
    switch (filter)
    {
        case SKINFILTER_MALE:   return !IsFemaleSkin(skin);
        case SKINFILTER_FEMALE: return IsFemaleSkin(skin);
    }
    return true;
}

static Skin_CountFiltered(filter)
{
    if (filter == SKINFILTER_ALL) return SKIN_COUNT;

    new count = 0;
    for (new i = 0; i < SKIN_COUNT; i++)
    {
        if (Skin_MatchesFilter(i, filter)) count++;
    }
    return count;
}

/*
 * Liefert die Skin-ID an Position <index> innerhalb des Filters.
 */
static Skin_ByFilterIndex(filter, index)
{
    new seen = 0;
    for (new i = 0; i < SKIN_COUNT; i++)
    {
        if (!Skin_MatchesFilter(i, filter)) continue;
        if (seen++ == index) return i;
    }
    return -1;
}

stock Skin_ShowCategories(playerid)
{
    new list[256], entry[80];

    format(entry, sizeof entry, "%sAlle Skins %s(%d)\n", EC_WHITE, EC_GREY, Skin_CountFiltered(SKINFILTER_ALL));
    strcat(list, entry);
    format(entry, sizeof entry, "%sMaennlich %s(%d)\n", EC_WHITE, EC_GREY, Skin_CountFiltered(SKINFILTER_MALE));
    strcat(list, entry);
    format(entry, sizeof entry, "%sWeiblich %s(%d)\n", EC_WHITE, EC_GREY, Skin_CountFiltered(SKINFILTER_FEMALE));
    strcat(list, entry);

    return ShowPlayerDialog(playerid, DIALOG_SKIN_CAT, DIALOG_STYLE_LIST,
        "Skinauswahl", list, "Weiter", "Abbrechen");
}

stock Skin_ShowPage(playerid, filter, page)
{
    new total = Skin_CountFiltered(filter);
    new pages = (total + DIALOG_PAGE_SIZE - 1) / DIALOG_PAGE_SIZE;
    if (pages < 1) pages = 1;

    if (page < 0)      page = pages - 1;
    if (page >= pages) page = 0;

    PlayerData[playerid][pDialogContext] = SKINCTX_PACK(filter, page);

    new list[2048], entry[80];
    new first = page * DIALOG_PAGE_SIZE;
    new last  = first + DIALOG_PAGE_SIZE;
    if (last > total) last = total;

    for (new i = first; i < last; i++)
    {
        new skin = Skin_ByFilterIndex(filter, i);
        if (skin == -1) break;

        format(entry, sizeof entry, "%s%d %s%s\n", EC_GREY, skin, EC_WHITE, GetSkinName(skin));
        strcat(list, entry);
    }

    // Navigationszeilen stehen am Ende und werden ueber ihren Listenindex erkannt
    strcat(list, EC_YELLOW "<< Vorherige Seite\n");
    strcat(list, EC_YELLOW ">> Naechste Seite\n");

    new caption[80], money[24];
    FormatNumber(SKIN_PRICE, money, sizeof money);
    format(caption, sizeof caption, "Skins - Seite %d von %d (je $%s)", page + 1, pages, money);

    return ShowPlayerDialog(playerid, DIALOG_SKIN_LIST, DIALOG_STYLE_LIST,
        caption, list, "Waehlen", "Zurueck");
}

stock Skin_Apply(playerid, skin)
{
    if (!IsValidSkin(skin))
    {
        SendError(playerid, "Diese Skin-ID gibt es nicht.");
        return 0;
    }
    if (PlayerData[playerid][pMinigame] != MINIGAME_NONE)
    {
        SendError(playerid, "Waehrend eines Minispiels kannst du den Skin nicht wechseln.");
        return 0;
    }
    if (GetPlayerMoney(playerid) < SKIN_PRICE)
    {
        new money[24], msg[112];
        FormatNumber(SKIN_PRICE, money, sizeof money);
        format(msg, sizeof msg, "Ein Skinwechsel kostet $%s.", money);
        SendError(playerid, msg);
        return 0;
    }

    GivePlayerMoney(playerid, -SKIN_PRICE);
    PlayerData[playerid][pSkin] = skin;
    SetPlayerSkin(playerid, skin);

    new msg[112];
    format(msg, sizeof msg, "Neuer Skin: %s%s%s (ID %d)", EC_WHITE, GetSkinName(skin), EC_GREEN, skin);
    SendSuccess(playerid, msg);
    return 1;
}

stock Skin_OnDialogResponse(playerid, dialogid, response, listitem)
{
    switch (dialogid)
    {
        case DIALOG_SKIN_CAT:
        {
            if (!response) return 1;
            Skin_ShowPage(playerid, listitem, 0);
            return 1;
        }
        case DIALOG_SKIN_LIST:
        {
            if (!response) { Skin_ShowCategories(playerid); return 1; }

            new context = PlayerData[playerid][pDialogContext];
            new filter = SKINCTX_FILTER(context);
            new page   = SKINCTX_PAGE(context);

            new total = Skin_CountFiltered(filter);
            new first = page * DIALOG_PAGE_SIZE;
            new last  = first + DIALOG_PAGE_SIZE;
            if (last > total) last = total;
            new shown = last - first;

            // Die beiden letzten Zeilen sind die Navigation
            if (listitem == shown)     { Skin_ShowPage(playerid, filter, page - 1); return 1; }
            if (listitem == shown + 1) { Skin_ShowPage(playerid, filter, page + 1); return 1; }

            new skin = Skin_ByFilterIndex(filter, first + listitem);
            if (skin != -1) Skin_Apply(playerid, skin);

            Skin_ShowPage(playerid, filter, page);
            return 1;
        }
    }
    return 0;
}

// ---------------------------------------------------------------------------
// Kommandos
// ---------------------------------------------------------------------------
CMD:skins(playerid, params[])
{
    // Ohne Argument das Menue, mit Argument die Namenssuche
    if (params[0] == EOS) return Skin_ShowCategories(playerid);

    if (Params_IsInt(params, 0)) return Skin_Apply(playerid, Params_Int(params, 0));

    new found = -1, matches = 0;
    for (new i = 0; i < SKIN_COUNT; i++)
    {
        if (strfind(gSkinInfo[i][skName], params, true) == -1) continue;
        found = i;
        matches++;
    }

    require(matches != 0, "Kein Skin mit diesem Namen gefunden. /skins zeigt alle.");

    if (matches > 1)
    {
        new msg[144], list[144], entry[48], shown = 0;
        for (new i = 0; i < SKIN_COUNT && shown < 6; i++)
        {
            if (strfind(gSkinInfo[i][skName], params, true) == -1) continue;
            format(entry, sizeof entry, "%d (%s)  ", i, gSkinInfo[i][skName]);
            strcat(list, entry);
            shown++;
        }
        format(msg, sizeof msg, "%d Treffer: %s", matches, list);
        SendInfo(playerid, msg);
        return 1;
    }

    Skin_Apply(playerid, found);
    return 1;
}
ALIAS:skinshop(skins)
