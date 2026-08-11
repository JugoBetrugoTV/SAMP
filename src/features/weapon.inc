/*
 * UIF Freeroam - Waffensystem
 *
 * Vorgefertigte Waffensets statt einzelner Waffenkommandos. Manche Sets sind
 * VIPs oder dem Team vorbehalten.
 */

#if defined _uif_weapon_included
    #endinput
#endif
#define _uif_weapon_included

#define WEAPONS_PER_SET         (6)

enum E_WEAPON_SET
{
    wsName[32],
    wsWeapons[WEAPONS_PER_SET],
    wsAmmo,
    wsMinVip,
    wsMinAdmin
};

static const gWeaponSets[][E_WEAPON_SET] =
{
    //  Name                Waffen-IDs (0 = leer)                       Munition  VIP          Admin
    {"Nahkampf",           {  4,  0,  0,  0,  0,  0},                        1, VIP_NONE,   ADMIN_NONE},
    {"Pistolen",           { 22, 24, 23,  0,  0,  0},                      500, VIP_NONE,   ADMIN_NONE},
    {"Maschinenpistolen",  { 28, 32, 29,  0,  0,  0},                      800, VIP_NONE,   ADMIN_NONE},
    {"Sturmgewehre",       { 30, 31,  0,  0,  0,  0},                      800, VIP_NONE,   ADMIN_NONE},
    {"Schrotflinten",      { 25, 26, 27,  0,  0,  0},                      300, VIP_NONE,   ADMIN_NONE},
    {"Scharfschuetze",     { 34, 33, 24,  0,  0,  0},                      200, VIP_NONE,   ADMIN_NONE},
    {"Sprengstoff",        { 16, 18, 17,  0,  0,  0},                       50, VIP_NONE,   ADMIN_NONE},
    {"Standardausruestung",{ 24, 31, 25, 34,  4, 42},                      800, VIP_NONE,   ADMIN_NONE},
    {"Raketenwerfer",      { 35, 36,  0,  0,  0,  0},                       25, VIP_BRONZE, ADMIN_NONE},
    {"Rambo",              { 38, 35, 31, 24, 34, 16},                      999, VIP_SILVER, ADMIN_NONE},
    {"Jetpack & Fallschirm",{46,  0,  0,  0,  0,  0},                        1, VIP_BRONZE, ADMIN_NONE},
    {"Teamausruestung",    { 38, 36, 31, 24, 34, 18},                      999, VIP_NONE,   ADMIN_ADMIN}
};

stock Weapon_GiveSet(playerid, index)
{
    if (index < 0 || index >= sizeof gWeaponSets) return 0;

    if (VipLevel(playerid) < gWeaponSets[index][wsMinVip] && !IsAdmin(playerid, ADMIN_ADMIN))
    {
        new msg[128];
        format(msg, sizeof msg, "Dieses Set ist %s vorbehalten. Infos: /vip",
            GetVipRankName(gWeaponSets[index][wsMinVip]));
        SendError(playerid, msg);
        return 0;
    }
    if (AdminLevel(playerid) < gWeaponSets[index][wsMinAdmin])
    {
        SendError(playerid, "Dieses Set ist dem Team vorbehalten.");
        return 0;
    }
    if (PlayerData[playerid][pMinigame] != MINIGAME_NONE)
    {
        SendError(playerid, "Waehrend eines Minispiels gibt es keine Waffen.");
        return 0;
    }

    for (new i = 0; i < WEAPONS_PER_SET; i++)
    {
        new weapon = gWeaponSets[index][wsWeapons][i];
        if (weapon == 0) continue;

        // Jetpack ist keine Waffe, sondern ein spezieller Animationszustand
        if (weapon == 46) { SetPlayerSpecialAction(playerid, SPECIAL_ACTION_USEJETPACK); continue; }

        GivePlayerWeapon(playerid, weapon, gWeaponSets[index][wsAmmo]);
    }

    new msg[96];
    format(msg, sizeof msg, "Waffenset erhalten: %s%s", EC_WHITE, gWeaponSets[index][wsName]);
    SendSuccess(playerid, msg);
    return 1;
}

stock Weapon_ShowList(playerid)
{
    new list[1024], entry[128];

    for (new i = 0; i < sizeof gWeaponSets; i++)
    {
        new suffix[48];
        suffix[0] = EOS;

        if (gWeaponSets[i][wsMinAdmin] > ADMIN_NONE)
        {
            format(suffix, sizeof suffix, " %s[Team]", EC_ORANGE);
        }
        else if (gWeaponSets[i][wsMinVip] > VIP_NONE)
        {
            format(suffix, sizeof suffix, " %s[%s]", EC_GOLD, GetVipRankName(gWeaponSets[i][wsMinVip]));
        }

        format(entry, sizeof entry, "%s%s%s\n", EC_WHITE, gWeaponSets[i][wsName], suffix);
        strcat(list, entry);
    }

    return ShowPlayerDialog(playerid, DIALOG_WEAPON_LIST, DIALOG_STYLE_LIST,
        "Waffensets", list, "Nehmen", "Abbrechen");
}

stock Weapon_OnDialogResponse(playerid, dialogid, response, listitem)
{
    if (dialogid != DIALOG_WEAPON_LIST) return 0;
    if (!response) return 1;

    Weapon_GiveSet(playerid, listitem);
    return 1;
}

// ---------------------------------------------------------------------------
// Kommandos
// ---------------------------------------------------------------------------
CMD:weapons(playerid, params[])
{
    #pragma unused params
    return Weapon_ShowList(playerid);
}
ALIAS:w(weapons)
ALIAS:guns(weapons)
CMD:gun(playerid, params[])
{
    require(params[0] != EOS, "Benutzung: /gun [Waffen-ID 1-46] [Munition]");
    require(Params_IsInt(params, 0), "Die Waffen-ID muss eine Zahl sein.");

    new weapon = Params_Int(params, 0);
    require(weapon >= 1 && weapon <= 46, "Ungueltige Waffen-ID (erlaubt: 1-46).");
    require(PlayerData[playerid][pMinigame] == MINIGAME_NONE, "Waehrend eines Minispiels gibt es keine Waffen.");

    // Raketenwerfer und Minigun nur fuer VIP/Team
    if ((weapon == 35 || weapon == 36 || weapon == 38) &&
        !IsVip(playerid) && !IsAdmin(playerid, ADMIN_ADMIN))
    {
        SendError(playerid, "Diese Waffe ist VIPs vorbehalten. Infos: /vip");
        return 1;
    }

    new ammo = (Params_Count(params) >= 2) ? Params_Int(params, 1) : 500;
    if (ammo < 1)    ammo = 1;
    if (ammo > 9999) ammo = 9999;

    GivePlayerWeapon(playerid, weapon, ammo);

    new msg[96];
    format(msg, sizeof msg, "Waffe %d mit %d Schuss erhalten.", weapon, ammo);
    SendSuccess(playerid, msg);
    return 1;
}

CMD:disarm(playerid, params[])
{
    #pragma unused params
    ResetPlayerWeapons(playerid);
    SendSuccess(playerid, "Alle Waffen abgelegt.");
    return 1;
}
