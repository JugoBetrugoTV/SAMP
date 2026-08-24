/*
 * Jebiga-Gaming - Filterscript: Tages- und Wetterzyklus
 *
 * Laesst die Uhrzeit realistisch weiterlaufen und wechselt das Wetter in
 * unregelmaessigen Abstaenden. Ohne das steht in San Andreas dauerhaft
 * dieselbe Mittagssonne.
 *
 * Die Uhrzeit folgt der echten Serverzeit, laeuft aber beschleunigt: eine
 * Spielstunde dauert TIME_MINUTES_PER_HOUR echte Minuten. So erlebt man
 * innerhalb einer Spielsitzung mehrere Tageswechsel.
 */

#include <a_samp>

#define TIME_MINUTES_PER_HOUR   (2)      // echte Minuten je Spielstunde
#define WEATHER_MIN_MINUTES     (8)      // fruehestens nach so vielen Minuten
#define WEATHER_MAX_MINUTES     (20)     // spaetestens nach so vielen
#define CYCLE_COLOR             0x3498DBFF

/*
 * Ausgewaehlte Wetter-IDs mit ihren Bezeichnungen. Bewusst nur freundliche
 * bis mittelschwere Lagen - dichter Nebel macht einen Freeroam-Server
 * unspielbar, weil man die Rampen nicht mehr sieht.
 */
enum E_WEATHER
{
    weId,
    weName[24]
};

static const gWeathers[][E_WEATHER] =
{
    { 0, "Wolkenlos"},
    { 1, "Sonnig"},
    { 2, "Leicht bewoelkt"},
    { 3, "Bedeckt"},
    { 5, "Klar"},
    { 7, "Bewoelkt"},
    { 8, "Regnerisch"},
    {10, "Diesig"},
    {11, "Sonnendunst"},
    {12, "Grau"},
    {17, "Hitzeflimmern"},
    {19, "Sandsturm"}
};

static gGameHour = 12;
static gWeatherIndex = 1;
static gMinutesUntilChange = WEATHER_MIN_MINUTES;

public OnFilterScriptInit()
{
    // Startzeit an der echten Uhrzeit ausrichten, damit der Server nicht
    // jedes Mal um Punkt zwoelf beginnt.
    new hour, minute, second;
    gettime(hour, minute, second);
    gGameHour = hour;

    SetWorldTime(gGameHour);

    gWeatherIndex = random(sizeof gWeathers);
    SetWeather(gWeathers[gWeatherIndex][weId]);

    gMinutesUntilChange = WEATHER_MIN_MINUTES + random(WEATHER_MAX_MINUTES - WEATHER_MIN_MINUTES);

    SetTimer("Cycle_Tick", 60000, true);

    printf("[Zyklus] Start um %d Uhr, Wetter: %s.", gGameHour, gWeathers[gWeatherIndex][weName]);
    return 1;
}

/*
 * Laeuft jede echte Minute.
 */
forward Cycle_Tick();
public Cycle_Tick()
{
    static realMinutes = 0;

    realMinutes++;
    if (realMinutes >= TIME_MINUTES_PER_HOUR)
    {
        realMinutes = 0;
        gGameHour = (gGameHour + 1) % 24;
        SetWorldTime(gGameHour);

        // Nur die markanten Tageszeiten ansagen
        new msg[96];
        switch (gGameHour)
        {
            case 6:  { format(msg, sizeof msg, "[ZEIT] Die Sonne geht auf."); SendClientMessageToAll(CYCLE_COLOR, msg); }
            case 12: { format(msg, sizeof msg, "[ZEIT] Mittag."); SendClientMessageToAll(CYCLE_COLOR, msg); }
            case 20: { format(msg, sizeof msg, "[ZEIT] Es wird dunkel."); SendClientMessageToAll(CYCLE_COLOR, msg); }
            case 0:  { format(msg, sizeof msg, "[ZEIT] Mitternacht."); SendClientMessageToAll(CYCLE_COLOR, msg); }
        }
    }

    gMinutesUntilChange--;
    if (gMinutesUntilChange > 0) return 1;

    // Neues Wetter waehlen, aber nicht dasselbe noch einmal
    new next = gWeatherIndex;
    while (next == gWeatherIndex && sizeof gWeathers > 1)
    {
        next = random(sizeof gWeathers);
    }
    gWeatherIndex = next;

    SetWeather(gWeathers[gWeatherIndex][weId]);
    gMinutesUntilChange = WEATHER_MIN_MINUTES + random(WEATHER_MAX_MINUTES - WEATHER_MIN_MINUTES);

    new msg[96];
    format(msg, sizeof msg, "[WETTER] Es wird %s.", gWeathers[gWeatherIndex][weName]);
    SendClientMessageToAll(CYCLE_COLOR, msg);
    return 1;
}

public OnPlayerCommandText(playerid, cmdtext[])
{
    if (strcmp(cmdtext, "/zeit", true) != 0 && strcmp(cmdtext, "/wetter", true) != 0) return 0;

    new msg[144];
    format(msg, sizeof msg, "* Es ist %d Uhr, das Wetter ist %s. Naechster Wechsel in %d Minuten.",
        gGameHour, gWeathers[gWeatherIndex][weName], gMinutesUntilChange);
    SendClientMessage(playerid, CYCLE_COLOR, msg);
    return 1;
}
