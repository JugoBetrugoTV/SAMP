/*
 * Jebiga-Gaming - Fahrzeugtuning
 *
 * Vollstaendiges Tuningmenue auf Basis von src/data/components.inc: alle 194
 * Bauteile von San Andreas, nach Kategorie gruppiert und mit den
 * Originalpreisen aus dem Spiel.
 *
 * Nicht jedes Teil passt an jedes Fahrzeug. AddVehicleComponent ignoriert
 * unpassende Kombinationen stillschweigend, deshalb wird der Kaufpreis erst
 * nach einer Sichtpruefung ueber GetVehicleComponentInSlot verrechnet - wer
 * ein Teil bekommt, das nicht passt, zahlt auch nicht dafuer.
 */

#if defined _jebiga_tuning_included
    #endinput
#endif
#define _jebiga_tuning_included

#define TUNING_MAX_CATEGORIES   (24)

static gTuningCategories[TUNING_MAX_CATEGORIES][21];
static gTuningCategoryCount = 0;

/*
 * Sammelt die vorkommenden Kategorien einmalig beim Serverstart.
 */
stock Tuning_OnGameModeInit()
{
    gTuningCategoryCount = 0;

    for (new i = 0; i < COMPONENT_COUNT; i++)
    {
        new bool:known = false;
        for (new c = 0; c < gTuningCategoryCount; c++)
        {
            if (!strcmp(gTuningCategories[c], gComponentInfo[i][ciPart], true))
            {
                known = true;
                break;
            }
        }
        if (known) continue;
        if (gTuningCategoryCount >= TUNING_MAX_CATEGORIES) continue;

        format(gTuningCategories[gTuningCategoryCount], 21, "%s", gComponentInfo[i][ciPart]);
        gTuningCategoryCount++;
    }

    new msg[80];
    format(msg, sizeof msg, "%d Tuningbauteile in %d Kategorien geladen.",
        COMPONENT_COUNT, gTuningCategoryCount);
    Log(msg);
    return gTuningCategoryCount;
}

static Tuning_CountInCategory(category)
{
    new count = 0;
    for (new i = 0; i < COMPONENT_COUNT; i++)
    {
        if (!strcmp(gComponentInfo[i][ciPart], gTuningCategories[category], true)) count++;
    }
    return count;
}

static Tuning_ByCategoryIndex(category, index)
{
    new seen = 0;
    for (new i = 0; i < COMPONENT_COUNT; i++)
    {
        if (strcmp(gComponentInfo[i][ciPart], gTuningCategories[category], true) != 0) continue;
        if (seen++ == index) return i;
    }
    return -1;
}

stock Tuning_ShowCategories(playerid)
{
    new list[1024], entry[64];

    for (new c = 0; c < gTuningCategoryCount; c++)
    {
        format(entry, sizeof entry, "%s%s %s(%d)\n",
            EC_WHITE, gTuningCategories[c], EC_GREY, Tuning_CountInCategory(c));
        strcat(list, entry);
    }

    return ShowPlayerDialog(playerid, DIALOG_TUNE_CAT, DIALOG_STYLE_LIST,
        "Tuning - Kategorie", list, "Weiter", "Abbrechen");
}

stock Tuning_ShowCategory(playerid, category)
{
    if (category < 0 || category >= gTuningCategoryCount) return 0;

    PlayerData[playerid][pDialogContext] = category;

    new list[2048], entry[96], price[24];
    new count = Tuning_CountInCategory(category);

    for (new i = 0; i < count; i++)
    {
        new index = Tuning_ByCategoryIndex(category, i);
        if (index == -1) break;

        FormatNumber(gComponentInfo[index][ciPrice], price, sizeof price);
        format(entry, sizeof entry, "%s%s  %s$%s\n",
            EC_WHITE, gComponentInfo[index][ciType], EC_YELLOW, price);

        if (strlen(list) + strlen(entry) >= sizeof(list) - 8) break;
        strcat(list, entry);
    }

    new caption[64];
    format(caption, sizeof caption, "Tuning - %s", gTuningCategories[category]);

    return ShowPlayerDialog(playerid, DIALOG_TUNE_LIST, DIALOG_STYLE_LIST,
        caption, list, "Einbauen", "Zurueck");
}

stock Tuning_Install(playerid, index)
{
    if (index < 0 || index >= COMPONENT_COUNT) return 0;

    if (!IsPlayerInAnyVehicleAsDriver(playerid))
    {
        SendError(playerid, "Du musst als Fahrer in einem Fahrzeug sitzen.");
        return 0;
    }

    new component = gComponentInfo[index][ciId];
    new price     = gComponentInfo[index][ciPrice];

    if (GetPlayerMoney(playerid) < price)
    {
        new money[24], msg[112];
        FormatNumber(price, money, sizeof money);
        format(msg, sizeof msg, "Dieses Teil kostet $%s.", money);
        SendError(playerid, msg);
        return 0;
    }

    new vehicleid = GetPlayerVehicleID(playerid);
    new slot = GetVehicleComponentType(component);

    AddVehicleComponent(vehicleid, component);

    // Passte das Teil nicht, sitzt im Slot weiterhin etwas anderes
    if (GetVehicleComponentInSlot(vehicleid, slot) != component)
    {
        new msg[144];
        format(msg, sizeof msg, "%s passt nicht an %s - nichts berechnet.",
            gComponentInfo[index][ciType], Vehicle_GetModelName(GetVehicleModel(vehicleid)));
        SendError(playerid, msg);
        return 0;
    }

    GivePlayerMoney(playerid, -price);

    new money[24], msg[144];
    FormatNumber(price, money, sizeof money);
    format(msg, sizeof msg, "%s %s eingebaut fuer $%s.",
        gComponentInfo[index][ciPart], gComponentInfo[index][ciType], money);
    SendSuccess(playerid, msg);
    return 1;
}

stock Tuning_OnDialogResponse(playerid, dialogid, response, listitem)
{
    switch (dialogid)
    {
        case DIALOG_TUNE_CAT:
        {
            if (!response) return 1;
            Tuning_ShowCategory(playerid, listitem);
            return 1;
        }
        case DIALOG_TUNE_LIST:
        {
            if (!response) { Tuning_ShowCategories(playerid); return 1; }

            new category = PlayerData[playerid][pDialogContext];
            new index = Tuning_ByCategoryIndex(category, listitem);
            if (index != -1) Tuning_Install(playerid, index);

            Tuning_ShowCategory(playerid, category);
            return 1;
        }
    }
    return 0;
}

// ---------------------------------------------------------------------------
// Kommandos
// ---------------------------------------------------------------------------
CMD:tune(playerid, params[])
{
    #pragma unused params
    require(IsPlayerInAnyVehicleAsDriver(playerid), "Du musst als Fahrer in einem Fahrzeug sitzen.");

    return Tuning_ShowCategories(playerid);
}
ALIAS:tuning(tune)

CMD:untune(playerid, params[])
{
    #pragma unused params
    require(IsPlayerInAnyVehicleAsDriver(playerid), "Du musst als Fahrer in einem Fahrzeug sitzen.");

    new vehicleid = GetPlayerVehicleID(playerid);

    // Alle 17 Bauteilslots leeren
    for (new slot = 0; slot < 17; slot++)
    {
        new component = GetVehicleComponentInSlot(vehicleid, slot);
        if (component) RemoveVehicleComponent(vehicleid, component);
    }

    SendSuccess(playerid, "Alle Tuningteile ausgebaut.");
    return 1;
}
