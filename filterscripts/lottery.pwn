/*
 * Jebiga-Gaming - Filterscript: Lotterie
 *
 * Alle 15 Minuten wird eine Zahl zwischen 1 und 100 gezogen. Wer vorher mit
 * /lotto eine Zahl gekauft hat und richtig liegt, bekommt den gesamten Topf;
 * liegt niemand richtig, waechst er in die naechste Runde hinein.
 *
 * Das Filterscript kommt ohne Zugriff auf den Gamemode aus: Einsatz und
 * Gewinn laufen ueber GivePlayerMoney, also ueber das serverseitige Geld,
 * das der Gamemode ohnehin speichert.
 */

#include <a_samp>

#define LOTTO_TICKET_PRICE      (5000)
#define LOTTO_MAX_NUMBER        (100)
#define LOTTO_ROUND_MINUTES     (15)
#define LOTTO_BASE_POT          (25000)
#define LOTTO_COLOR             0xF1C40FFF
#define LOTTO_DIALOG            (9200)

static gLottoNumber[MAX_PLAYERS];       // 0 = kein Los gekauft
static gLottoPot = LOTTO_BASE_POT;
static gLottoMinutesLeft = LOTTO_ROUND_MINUTES;

public OnFilterScriptInit()
{
    for (new i = 0; i < MAX_PLAYERS; i++) gLottoNumber[i] = 0;

    gLottoPot = LOTTO_BASE_POT;
    gLottoMinutesLeft = LOTTO_ROUND_MINUTES;

    SetTimer("Lotto_Minute", 60000, true);

    printf("[Lotterie] Bereit. Ziehung alle %d Minuten.", LOTTO_ROUND_MINUTES);
    return 1;
}

public OnPlayerConnect(playerid)
{
    gLottoNumber[playerid] = 0;
    return 1;
}

public OnPlayerDisconnect(playerid, reason)
{
    #pragma unused reason

    // Der Einsatz bleibt im Topf - sonst waere Verlassen kurz vor der
    // Ziehung eine kostenlose Ruecknahme.
    gLottoNumber[playerid] = 0;
    return 1;
}

/*
 * Fuehrt die Ziehung durch und verteilt den Topf.
 */
static Lotto_Draw()
{
    new drawn = 1 + random(LOTTO_MAX_NUMBER);
    new msg[144], winners = 0;

    format(msg, sizeof msg, "[LOTTERIE] Gezogene Zahl: %d", drawn);
    SendClientMessageToAll(LOTTO_COLOR, msg);

    new maxPlayers = GetPlayerPoolSize();
    for (new i = 0; i <= maxPlayers; i++)
    {
        if (!IsPlayerConnected(i) || IsPlayerNPC(i)) continue;
        if (gLottoNumber[i] != drawn) continue;
        winners++;
    }

    if (winners == 0)
    {
        format(msg, sizeof msg, "[LOTTERIE] Kein Gewinner. Der Topf waechst auf $%d.", gLottoPot);
        SendClientMessageToAll(LOTTO_COLOR, msg);
    }
    else
    {
        // Bei mehreren Gewinnern wird geteilt
        new share = gLottoPot / winners;

        for (new i = 0; i <= maxPlayers; i++)
        {
            if (!IsPlayerConnected(i) || IsPlayerNPC(i)) continue;
            if (gLottoNumber[i] != drawn) continue;

            GivePlayerMoney(i, share);

            new name[MAX_PLAYER_NAME + 1];
            GetPlayerName(i, name, sizeof name);
            format(msg, sizeof msg, "[LOTTERIE] %s hat die %d und gewinnt $%d!", name, drawn, share);
            SendClientMessageToAll(LOTTO_COLOR, msg);
        }
        gLottoPot = LOTTO_BASE_POT;
    }

    for (new i = 0; i < MAX_PLAYERS; i++) gLottoNumber[i] = 0;
    gLottoMinutesLeft = LOTTO_ROUND_MINUTES;
    return winners;
}

forward Lotto_Minute();
public Lotto_Minute()
{
    gLottoMinutesLeft--;

    if (gLottoMinutesLeft <= 0)
    {
        Lotto_Draw();
        return 1;
    }

    // Nur zu markanten Zeitpunkten erinnern, sonst wird es Spam
    if (gLottoMinutesLeft == 5 || gLottoMinutesLeft == 1)
    {
        new msg[144];
        format(msg, sizeof msg, "[LOTTERIE] Noch %d Minute%s bis zur Ziehung. Topf: $%d  -  /lotto",
            gLottoMinutesLeft, (gLottoMinutesLeft == 1) ? "" : "n", gLottoPot);
        SendClientMessageToAll(LOTTO_COLOR, msg);
    }
    return 1;
}

public OnPlayerCommandText(playerid, cmdtext[])
{
    if (strcmp(cmdtext, "/lotto", true, 6) != 0) return 0;

    // Ohne Argument nur den Stand anzeigen
    if (cmdtext[6] == EOS)
    {
        new body[512];
        format(body, sizeof body,
            "{FFFFFF}Aktueller Topf: {F1C40F}$%d\n\
            {FFFFFF}Naechste Ziehung in: {F1C40F}%d Minuten\n\
            {FFFFFF}Dein Los: {F1C40F}%s\n\n\
            {AFAFAF}Ein Los kostet $%d und gilt fuer eine Zahl von 1 bis %d.\n\
            {AFAFAF}Benutzung: /lotto [Zahl]",
            gLottoPot, gLottoMinutesLeft,
            (gLottoNumber[playerid] > 0) ? "gekauft" : "keins",
            LOTTO_TICKET_PRICE, LOTTO_MAX_NUMBER);

        ShowPlayerDialog(playerid, LOTTO_DIALOG, DIALOG_STYLE_MSGBOX, "Lotterie", body, "Schliessen", "");
        return 1;
    }

    new number = strval(cmdtext[7]);

    if (number < 1 || number > LOTTO_MAX_NUMBER)
    {
        new msg[96];
        format(msg, sizeof msg, "* Die Zahl muss zwischen 1 und %d liegen.", LOTTO_MAX_NUMBER);
        SendClientMessage(playerid, 0xE74C3CFF, msg);
        return 1;
    }
    if (gLottoNumber[playerid] != 0)
    {
        SendClientMessage(playerid, 0xE74C3CFF, "* Du hast fuer diese Runde bereits ein Los.");
        return 1;
    }
    if (GetPlayerMoney(playerid) < LOTTO_TICKET_PRICE)
    {
        new msg[96];
        format(msg, sizeof msg, "* Ein Los kostet $%d.", LOTTO_TICKET_PRICE);
        SendClientMessage(playerid, 0xE74C3CFF, msg);
        return 1;
    }

    GivePlayerMoney(playerid, -LOTTO_TICKET_PRICE);
    gLottoNumber[playerid] = number;
    gLottoPot += LOTTO_TICKET_PRICE;

    new msg[128];
    format(msg, sizeof msg, "* Los auf die %d gekauft. Topf jetzt $%d, Ziehung in %d Minuten.",
        number, gLottoPot, gLottoMinutesLeft);
    SendClientMessage(playerid, 0x2ECC71FF, msg);
    return 1;
}
