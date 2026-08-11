/*
 * Jebiga-Gaming - Innenraeume
 *
 * Zugang zu 127 Innenraeumen von San Andreas - Laeden, Clubs, Restaurants,
 * Amtsgebaeude - nach Kategorie sortiert. Koordinaten und Interior-IDs
 * stammen aus src/data/interiors.inc.
 *
 * Beim Betreten wird die Position ausserhalb gemerkt, damit /exitint wieder
 * genau dorthin zurueckfuehrt.
 */

#if defined _jebiga_interior_included
    #endinput
#endif
#define _jebiga_interior_included

#define INTERIOR_MAX_CATEGORIES (24)

static gInteriorCategories[INTERIOR_MAX_CATEGORIES][25];
static gInteriorCategoryCount = 0;

stock Interior_OnGameModeInit()
{
    gInteriorCategoryCount = 0;

    for (new i = 0; i < INTERIOR_COUNT; i++)
    {
        new bool:known = false;
        for (new c = 0; c < gInteriorCategoryCount; c++)
        {
            if (!strcmp(gInteriorCategories[c], gInteriorInfo[i][itCategory], true))
            {
                known = true;
                break;
            }
        }
        if (known) continue;
        if (gInteriorCategoryCount >= INTERIOR_MAX_CATEGORIES) continue;

        format(gInteriorCategories[gInteriorCategoryCount], 25, "%s", gInteriorInfo[i][itCategory]);
        gInteriorCategoryCount++;
    }

    new msg[80];
    format(msg, sizeof msg, "%d Innenraeume in %d Kategorien geladen.",
        INTERIOR_COUNT, gInteriorCategoryCount);
    Log(msg);
    return gInteriorCategoryCount;
}

static Interior_CountInCategory(category)
{
    new count = 0;
    for (new i = 0; i < INTERIOR_COUNT; i++)
    {
        if (!strcmp(gInteriorInfo[i][itCategory], gInteriorCategories[category], true)) count++;
    }
    return count;
}

static Interior_ByCategoryIndex(category, index)
{
    new seen = 0;
    for (new i = 0; i < INTERIOR_COUNT; i++)
    {
        if (strcmp(gInteriorInfo[i][itCategory], gInteriorCategories[category], true) != 0) continue;
        if (seen++ == index) return i;
    }
    return -1;
}

stock Interior_ShowCategories(playerid)
{
    new list[1024], entry[80];

    for (new c = 0; c < gInteriorCategoryCount; c++)
    {
        format(entry, sizeof entry, "%s%s %s(%d)\n",
            EC_WHITE, gInteriorCategories[c], EC_GREY, Interior_CountInCategory(c));
        strcat(list, entry);
    }

    return ShowPlayerDialog(playerid, DIALOG_INTERIOR_CAT, DIALOG_STYLE_LIST,
        "Innenraeume", list, "Weiter", "Abbrechen");
}

stock Interior_ShowCategory(playerid, category)
{
    if (category < 0 || category >= gInteriorCategoryCount) return 0;

    PlayerData[playerid][pDialogContext] = category;

    new list[2048], entry[80];
    new count = Interior_CountInCategory(category);

    for (new i = 0; i < count; i++)
    {
        new index = Interior_ByCategoryIndex(category, i);
        if (index == -1) break;

        format(entry, sizeof entry, "%s%s\n", EC_WHITE, gInteriorInfo[index][itName]);
        if (strlen(list) + strlen(entry) >= sizeof(list) - 8) break;
        strcat(list, entry);
    }

    new caption[64];
    format(caption, sizeof caption, "Innenraeume - %s", gInteriorCategories[category]);

    return ShowPlayerDialog(playerid, DIALOG_INTERIOR_LIST, DIALOG_STYLE_LIST,
        caption, list, "Betreten", "Zurueck");
}

stock Interior_Enter(playerid, index)
{
    if (index < 0 || index >= INTERIOR_COUNT) return 0;
    if (!Teleport_CanUse(playerid)) return 0;

    // Rueckweg merken - aber nur, wenn wir gerade draussen sind
    if (!PlayerData[playerid][pInInterior])
    {
        GetPlayerPos(playerid,
            PlayerData[playerid][pIntReturnPos][0],
            PlayerData[playerid][pIntReturnPos][1],
            PlayerData[playerid][pIntReturnPos][2]);
    }

    SetPlayerPos(playerid, gInteriorInfo[index][itX], gInteriorInfo[index][itY], gInteriorInfo[index][itZ]);
    SetPlayerInterior(playerid, gInteriorInfo[index][itInterior]);
    SetPlayerVirtualWorld(playerid, WORLD_MAIN);
    SetCameraBehindPlayer(playerid);
    GivePlayerSpawnProtection(playerid, 3000);

    PlayerData[playerid][pInInterior] = true;

    new msg[144];
    format(msg, sizeof msg, "%s%s%s betreten. Zurueck mit %s/exitint",
        EC_WHITE, gInteriorInfo[index][itName], EC_CYAN, EC_WHITE);
    SendInfo(playerid, msg);
    return 1;
}

stock Interior_OnDialogResponse(playerid, dialogid, response, listitem)
{
    switch (dialogid)
    {
        case DIALOG_INTERIOR_CAT:
        {
            if (!response) return 1;
            Interior_ShowCategory(playerid, listitem);
            return 1;
        }
        case DIALOG_INTERIOR_LIST:
        {
            if (!response) { Interior_ShowCategories(playerid); return 1; }

            new index = Interior_ByCategoryIndex(PlayerData[playerid][pDialogContext], listitem);
            if (index != -1) Interior_Enter(playerid, index);
            return 1;
        }
    }
    return 0;
}

// ---------------------------------------------------------------------------
// Kommandos
// ---------------------------------------------------------------------------
CMD:interiors(playerid, params[])
{
    // Ohne Argument das Menue, mit Argument die Namenssuche
    if (params[0] == EOS) return Interior_ShowCategories(playerid);

    new found = -1, matches = 0;
    for (new i = 0; i < INTERIOR_COUNT; i++)
    {
        if (strfind(gInteriorInfo[i][itName], params, true) == -1) continue;
        found = i;
        matches++;
    }

    require(matches != 0, "Kein Innenraum mit diesem Namen. /interiors zeigt alle.");
    require(matches == 1, "Mehrere Innenraeume passen - bitte genauer angeben.");

    Interior_Enter(playerid, found);
    return 1;
}
ALIAS:int(interiors)

CMD:exitint(playerid, params[])
{
    #pragma unused params
    require(PlayerData[playerid][pInInterior], "Du bist in keinem Innenraum.");

    SetPlayerPos(playerid,
        PlayerData[playerid][pIntReturnPos][0],
        PlayerData[playerid][pIntReturnPos][1],
        PlayerData[playerid][pIntReturnPos][2]);
    SetPlayerInterior(playerid, 0);
    SetPlayerVirtualWorld(playerid, WORLD_MAIN);
    SetCameraBehindPlayer(playerid);

    PlayerData[playerid][pInInterior] = false;
    SendSuccess(playerid, "Zurueck nach draussen.");
    return 1;
}
