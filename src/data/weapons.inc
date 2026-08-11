/*
 * Jebiga-Gaming - Waffendaten
 *
 * ERZEUGTE DATEI - NICHT VON HAND BEARBEITEN.
 * Erzeugt von tools/gen_data.py aus der Referenzdokumentation von open.mp
 * (docs/scripting/resources/weaponids.md). Aenderungen gehoeren in den Generator.
 *
 * 47 Waffen (0 bis 46) mit Name und Inventarplatz.
 */

#if defined _jg_weapons_inc
    #endinput
#endif
#define _jg_weapons_inc


#define WEAPON_COUNT            (47)

enum E_WEAPON_INFO
{
    wiName[28],
    wiSlot
};

new const gWeaponInfo[WEAPON_COUNT][E_WEAPON_INFO] =
{
    {"Fist", 0},   // 0
    {"Brass Knuckles", 0},   // 1
    {"Golf Club", 1},   // 2
    {"Nightstick", 1},   // 3
    {"Knife", 1},   // 4
    {"Baseball Bat", 1},   // 5
    {"Shovel", 1},   // 6
    {"Pool Cue", 1},   // 7
    {"Katana", 1},   // 8
    {"Chainsaw", 1},   // 9
    {"Purple Dildo", 10},   // 10
    {"Dildo", 10},   // 11
    {"Vibrator", 10},   // 12
    {"Silver Vibrator", 10},   // 13
    {"Flowers", 10},   // 14
    {"Cane", 10},   // 15
    {"Grenade", 8},   // 16
    {"Tear Gas", 8},   // 17
    {"Molotov Cocktail", 8},   // 18
    {"Waffe 19", 0},   // 19
    {"Waffe 20", 0},   // 20
    {"Waffe 21", 0},   // 21
    {"9mm", 2},   // 22
    {"Silenced 9mm", 2},   // 23
    {"Desert Eagle", 2},   // 24
    {"Shotgun", 3},   // 25
    {"Sawnoff Shotgun", 3},   // 26
    {"Combat Shotgun", 3},   // 27
    {"Micro SMG/UZI", 4},   // 28
    {"MP5", 4},   // 29
    {"AK-47", 4},   // 30
    {"M4", 5},   // 31
    {"Tec-9", 4},   // 32
    {"Country Rifle", 6},   // 33
    {"Sniper Rifle", 6},   // 34
    {"RPG", 7},   // 35
    {"HS Rocket", 7},   // 36
    {"Flamethrower", 7},   // 37
    {"Minigun", 7},   // 38
    {"Satchel Charge", 8},   // 39
    {"Detonator", 12},   // 40
    {"Spraycan", 9},   // 41
    {"Fire Extinguisher", 9},   // 42
    {"Camera", 9},   // 43
    {"Night Vision Goggles", 11},   // 44
    {"Thermal Goggles", 11},   // 45
    {"Parachute", 11}   // 46
};

stock bool:IsValidWeapon(weaponid)
{
    return (weaponid >= 0 && weaponid < WEAPON_COUNT);
}

stock GetWeaponNameEx(weaponid)
{
    new name[28];
    if (!IsValidWeapon(weaponid)) name = "Unbekannt";
    else format(name, sizeof name, "%s", gWeaponInfo[weaponid][wiName]);
    return name;
}

stock GetWeaponSlot(weaponid)
{
    if (!IsValidWeapon(weaponid)) return -1;
    return gWeaponInfo[weaponid][wiSlot];
}

/*
 * Sucht eine Waffe anhand einer ID oder eines Namensfragments.
 * Rueckgabe: Waffen-ID, -1 = nichts gefunden, -2 = mehrdeutig.
 */
stock FindWeaponId(const input[])
{
    if (input[0] == EOS) return -1;

    new bool:numeric = true;
    for (new i = 0; input[i] != EOS; i++)
    {
        if (input[i] < '0' || input[i] > '9') { numeric = false; break; }
    }
    if (numeric)
    {
        new id = strval(input);
        return IsValidWeapon(id) ? id : -1;
    }

    new found = -1, matches = 0;
    for (new i = 0; i < WEAPON_COUNT; i++)
    {
        if (!strcmp(gWeaponInfo[i][wiName], input, true)) return i;
        if (strfind(gWeaponInfo[i][wiName], input, true) != -1)
        {
            found = i;
            matches++;
        }
    }
    return (matches > 1) ? -2 : found;
}
