/*
 * Jebiga-Gaming - Erfolge
 *
 * Zwanzig Ziele, die sich alle aus bereits vorhandenen Statistiken ableiten.
 * Genau darum steht die Pruefung an einer einzigen Stelle: statt jedes Modul
 * mit Erfolgs-Aufrufen zu durchsetzen, wertet Achievement_Check() regelmaessig
 * die Spielerdaten aus. Neue Erfolge brauchen dadurch nur einen Tabelleneintrag
 * und eine Zeile in der Auswertung.
 *
 * Gespeichert wird als Bitmaske in einer einzigen Accountzahl.
 */

#if defined _jebiga_achievement_included
    #endinput
#endif
#define _jebiga_achievement_included

#define ACHIEVEMENT_COUNT       (20)

enum E_ACHIEVEMENT
{
    acName[36],
    acDescription[64],
    acReward
};

static const gAchievements[ACHIEVEMENT_COUNT][E_ACHIEVEMENT] =
{
    {"Erster Schritt",      "Registriere dich und spiele deine erste Minute",   1000},
    {"Erster Abschuss",     "Schalte deinen ersten Gegner aus",                 2500},
    {"Scharfschuetze",      "Erreiche 100 Kills",                              25000},
    {"Legende",             "Erreiche 1000 Kills",                            150000},
    {"Stehaufmaennchen",    "Stirb 100 Mal",                                    5000},
    {"Erster Stunt",        "Sammle deinen ersten Stuntpunkt",                  1000},
    {"Luftakrobat",         "Sammle 1000 Stuntpunkte",                         20000},
    {"Schwerelos",          "Sammle 10000 Stuntpunkte",                       120000},
    {"Boostjunkie",         "Loese 500 Schuebe mit Taste 2 aus",                15000},
    {"Rennfahrer",          "Gewinne dein erstes Rennen",                       10000},
    {"Rennlegende",         "Gewinne 25 Rennen",                               100000},
    {"Blechschaden",        "Gewinne dein erstes Derby",                        10000},
    {"Schrottkoenig",       "Gewinne 25 Derbys",                               100000},
    {"Eigenheim",           "Kaufe dein erstes Haus",                           15000},
    {"Vereinsmeier",        "Tritt einer Crew bei oder gruende eine",            5000},
    {"Berufstaetig",        "Schliesse deine erste Jobroute ab",                 5000},
    {"Vielfahrer",          "Schliesse 50 Jobrouten ab",                        75000},
    {"Sparer",              "Habe 1000000 auf dem Bankkonto",                  100000},
    {"Stammgast",           "Spiele insgesamt 10 Stunden",                      50000},
    {"Urgestein",           "Spiele insgesamt 100 Stunden",                    300000}
};

stock bool:Achievement_Has(playerid, index)
{
    if (index < 0 || index >= ACHIEVEMENT_COUNT) return false;
    return ((PlayerData[playerid][pAchievements] & (1 << index)) != 0);
}

static Achievement_Grant(playerid, index)
{
    if (Achievement_Has(playerid, index)) return 0;

    PlayerData[playerid][pAchievements] |= (1 << index);
    GivePlayerMoney(playerid, gAchievements[index][acReward]);
    PlayerData[playerid][pScore] += 2;
    SetPlayerScore(playerid, PlayerData[playerid][pScore]);

    new money[24], msg[176];
    FormatNumber(gAchievements[index][acReward], money, sizeof money);

    format(msg, sizeof msg, "~y~Erfolg freigeschaltet~n~~w~%s", gAchievements[index][acName]);
    GameTextForPlayer(playerid, msg, 4000, 1);

    format(msg, sizeof msg, "Erfolg freigeschaltet: %s%s%s - $%s Belohnung",
        EC_WHITE, gAchievements[index][acName], EC_GREEN, money);
    SendSuccess(playerid, msg);

    format(msg, sizeof msg, "%s hat den Erfolg \"%s\" freigeschaltet.",
        GetName(playerid), gAchievements[index][acName]);
    SendClientMessageToAll(COL_GREY, msg);
    return 1;
}

/*
 * Wertet alle Bedingungen aus. Die Reihenfolge entspricht der Tabelle oben.
 */
stock Achievement_Check(playerid)
{
    if (!IsLoggedIn(playerid)) return 0;

    new playtime = GetTotalPlayTime(playerid);

    if (playtime >= 60)                                     Achievement_Grant(playerid,  0);
    if (PlayerData[playerid][pKills] >= 1)                  Achievement_Grant(playerid,  1);
    if (PlayerData[playerid][pKills] >= 100)                Achievement_Grant(playerid,  2);
    if (PlayerData[playerid][pKills] >= 1000)               Achievement_Grant(playerid,  3);
    if (PlayerData[playerid][pDeaths] >= 100)               Achievement_Grant(playerid,  4);
    if (PlayerData[playerid][pStuntPoints] >= 1)            Achievement_Grant(playerid,  5);
    if (PlayerData[playerid][pStuntPoints] >= 1000)         Achievement_Grant(playerid,  6);
    if (PlayerData[playerid][pStuntPoints] >= 10000)        Achievement_Grant(playerid,  7);
    if (PlayerData[playerid][pBoostCount] >= 500)           Achievement_Grant(playerid,  8);
    if (PlayerData[playerid][pRaceWins] >= 1)               Achievement_Grant(playerid,  9);
    if (PlayerData[playerid][pRaceWins] >= 25)              Achievement_Grant(playerid, 10);
    if (PlayerData[playerid][pDerbyWins] >= 1)              Achievement_Grant(playerid, 11);
    if (PlayerData[playerid][pDerbyWins] >= 25)             Achievement_Grant(playerid, 12);
    if (PlayerData[playerid][pHouse] != -1)                 Achievement_Grant(playerid, 13);
    if (PlayerData[playerid][pCrew] != -1)                  Achievement_Grant(playerid, 14);
    if (PlayerData[playerid][pJobsDone] >= 1)               Achievement_Grant(playerid, 15);
    if (PlayerData[playerid][pJobsDone] >= 50)              Achievement_Grant(playerid, 16);
    if (PlayerData[playerid][pBank] >= 1000000)             Achievement_Grant(playerid, 17);
    if (playtime >= 36000)                                  Achievement_Grant(playerid, 18);
    if (playtime >= 360000)                                 Achievement_Grant(playerid, 19);
    return 1;
}

stock Achievement_Count(playerid)
{
    new count = 0;
    for (new i = 0; i < ACHIEVEMENT_COUNT; i++)
    {
        if (Achievement_Has(playerid, i)) count++;
    }
    return count;
}

// ---------------------------------------------------------------------------
// Kommandos
// ---------------------------------------------------------------------------
CMD:achievements(playerid, params[])
{
    #pragma unused params

    new body[2048], entry[144], money[24];

    for (new i = 0; i < ACHIEVEMENT_COUNT; i++)
    {
        FormatNumber(gAchievements[i][acReward], money, sizeof money);

        if (Achievement_Has(playerid, i))
        {
            format(entry, sizeof entry, "{2ECC71}[x] %s {AFAFAF}- %s\n",
                gAchievements[i][acName], gAchievements[i][acDescription]);
        }
        else
        {
            format(entry, sizeof entry, "{AFAFAF}[ ] %s - %s {F1C40F}($%s)\n",
                gAchievements[i][acName], gAchievements[i][acDescription], money);
        }
        strcat(body, entry);
    }

    new caption[64];
    format(caption, sizeof caption, "Erfolge (%d von %d)", Achievement_Count(playerid), ACHIEVEMENT_COUNT);

    ShowPlayerDialog(playerid, DIALOG_ACHIEVEMENTS, DIALOG_STYLE_MSGBOX,
        caption, body, "Schliessen", "");
    return 1;
}
ALIAS:erfolge(achievements)
