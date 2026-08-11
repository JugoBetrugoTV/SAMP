/*
 * UIF Freeroam - Tuning, Animationen und Spassbefehle
 *
 * Kleinteile, die auf einem Freeroam-Server erwartet werden: ein Tuningmenue
 * fuer das aktuelle Fahrzeug, ein paar Animationen und Befehle, die vor allem
 * unterhalten sollen.
 */

#if defined _uif_extras_included
    #endinput
#endif
#define _uif_extras_included

// ---------------------------------------------------------------------------
// Tuning
//
// Die Bauteil-IDs stammen aus dem Tuningsystem von GTA San Andreas. Nicht
// jedes Teil passt an jedes Fahrzeug; AddVehicleComponent ignoriert
// unpassende Kombinationen stillschweigend, deshalb genuegt hier eine
// Rueckmeldung ohne eigene Pruefung.
// ---------------------------------------------------------------------------
enum E_TUNING
{
    tuName[28],
    tuComponent,
    tuPrice
};

static const gTuning[][E_TUNING] =
{
    {"Nitro (2x)",          1009,  15000},
    {"Nitro (5x)",          1008,  25000},
    {"Nitro (10x)",         1010,  40000},
    {"Hydraulik",           1087,  20000},
    {"Rennfelgen 'Shadow'", 1075,   8000},
    {"Rennfelgen 'Mega'",   1080,   8000},
    {"Rennfelgen 'Grove'",  1085,   8000},
    {"Rennfelgen 'Twist'",  1084,   8000},
    {"Spoiler 'Alpha'",     1000,  12000},
    {"Spoiler 'Champ'",     1001,  12000},
    {"Auspuff 'Upswept'",   1129,   9000},
    {"Auspuff 'Twin'",      1132,   9000},
    {"Dachscheinwerfer",    1087,  10000}
};

stock Tuning_ShowMenu(playerid)
{
    new list[1024], entry[96], price[24];

    for (new i = 0; i < sizeof gTuning; i++)
    {
        FormatNumber(gTuning[i][tuPrice], price, sizeof price);
        format(entry, sizeof entry, "%s%s  %s$%s\n",
            EC_WHITE, gTuning[i][tuName], EC_YELLOW, price);
        strcat(list, entry);
    }

    return ShowPlayerDialog(playerid, DIALOG_TUNING, DIALOG_STYLE_LIST,
        "Tuning", list, "Einbauen", "Abbrechen");
}

static Tuning_Apply(playerid, index)
{
    if (index < 0 || index >= sizeof gTuning) return 0;

    if (!IsPlayerInAnyVehicleAsDriver(playerid))
    {
        SendError(playerid, "Du musst als Fahrer in einem Fahrzeug sitzen.");
        return 0;
    }
    if (GetPlayerMoney(playerid) < gTuning[index][tuPrice])
    {
        new price[24], msg[112];
        FormatNumber(gTuning[index][tuPrice], price, sizeof price);
        format(msg, sizeof msg, "Dieses Teil kostet $%s.", price);
        SendError(playerid, msg);
        return 0;
    }

    AddVehicleComponent(GetPlayerVehicleID(playerid), gTuning[index][tuComponent]);
    GivePlayerMoney(playerid, -gTuning[index][tuPrice]);

    new msg[112];
    format(msg, sizeof msg, "%s eingebaut. Passt das Teil nicht zum Modell, bleibt es wirkungslos.",
        gTuning[index][tuName]);
    SendSuccess(playerid, msg);
    return 1;
}

// ---------------------------------------------------------------------------
// Animationen
// ---------------------------------------------------------------------------
enum E_ANIM
{
    anName[24],
    anLibrary[20],
    anAnimation[28],
    bool:anLoop
};

static const gAnims[][E_ANIM] =
{
    {"Tanzen 1",     "DANCING",  "dnce_M_a",       true},
    {"Tanzen 2",     "DANCING",  "dnce_M_b",       true},
    {"Tanzen 3",     "DANCING",  "dnce_M_c",       true},
    {"Tanzen 4",     "DANCING",  "dnce_M_d",       true},
    {"Unterhalten",  "PED",      "IDLE_CHAT",      true},
    {"Sonnen",       "BEACH",    "bather",         true},
    {"Rappen",       "RAPPING",  "RAP_A_Loop",     true},
    {"Essen",        "FOOD",     "EAT_Burger",     false},
    {"Handy",        "PED",      "phone_talk",     true},
    {"Jubeln",       "RIOT",     "RIOT_ANGRY",     false},
    {"Hinsetzen",    "PED",      "SEAT_idle",      true},
    {"Aufgeben",     "PED",      "handsup",        true}
};

stock Anim_ShowList(playerid)
{
    new list[768], entry[64];

    for (new i = 0; i < sizeof gAnims; i++)
    {
        format(entry, sizeof entry, "%s%s\n", EC_WHITE, gAnims[i][anName]);
        strcat(list, entry);
    }

    return ShowPlayerDialog(playerid, DIALOG_ANIM_LIST, DIALOG_STYLE_LIST,
        "Animationen", list, "Abspielen", "Abbrechen");
}

static Anim_Play(playerid, index)
{
    if (index < 0 || index >= sizeof gAnims) return 0;

    if (IsPlayerInAnyVehicle(playerid))
    {
        SendError(playerid, "Im Fahrzeug geht das nicht.");
        return 0;
    }

    // Die Animationsbibliothek muss vor dem ersten Abspielen geladen werden
    ApplyAnimation(playerid, gAnims[index][anLibrary], gAnims[index][anAnimation],
        4.1, gAnims[index][anLoop], 0, 0, 0, 0, 1);
    ApplyAnimation(playerid, gAnims[index][anLibrary], gAnims[index][anAnimation],
        4.1, gAnims[index][anLoop], 0, 0, gAnims[index][anLoop] ? 1 : 0, 0, 1);

    new msg[80];
    format(msg, sizeof msg, "Animation: %s (beenden mit /stopanim)", gAnims[index][anName]);
    SendInfo(playerid, msg);
    return 1;
}

// ---------------------------------------------------------------------------
// Dialoge
// ---------------------------------------------------------------------------
stock Extras_OnDialogResponse(playerid, dialogid, response, listitem)
{
    switch (dialogid)
    {
        case DIALOG_TUNING:
        {
            if (!response) return 1;
            Tuning_Apply(playerid, listitem);
            return 1;
        }
        case DIALOG_ANIM_LIST:
        {
            if (!response) return 1;
            Anim_Play(playerid, listitem);
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

    return Tuning_ShowMenu(playerid);
}
ALIAS:tuning(tune)

CMD:anims(playerid, params[])
{
    #pragma unused params
    return Anim_ShowList(playerid);
}
ALIAS:anim(anims)

CMD:stopanim(playerid, params[])
{
    #pragma unused params
    ClearAnimations(playerid);
    SendSuccess(playerid, "Animation beendet.");
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
