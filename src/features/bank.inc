/*
 * Jebiga-Gaming - Bank
 *
 * Zweites Konto neben dem Bargeld. Der Unterschied ist praktisch: Bargeld
 * verliert man beim Tod nicht, aber es liegt sichtbar herum - das Bankkonto
 * verzinst sich stattdessen.
 *
 * Die Zinsen laufen an der Spielzeit, nicht an der Echtzeit: nur wer online
 * ist, bekommt sie gutgeschrieben. Sonst waere Nichtstun die beste Strategie.
 */

#if defined _jebiga_bank_included
    #endinput
#endif
#define _jebiga_bank_included

#define BANK_INTEREST_PERCENT   (2)      // je Zinsintervall
#define BANK_INTEREST_MINUTES   (30)     // Spielminuten bis zur Gutschrift
#define BANK_INTEREST_CAP       (25000)  // Deckel je Gutschrift
#define BANK_TRANSFER_FEE       (2)      // Prozent, mindestens 1 Dollar

stock Bank_ShowMenu(playerid)
{
    new bank[24], cash[24], body[640];
    FormatNumber(PlayerData[playerid][pBank], bank, sizeof bank);
    FormatNumber(GetPlayerMoney(playerid), cash, sizeof cash);

    format(body, sizeof body,
        "%sKontostand:%s $%s\n\
        %sBargeld:%s $%s\n\n\
        %sZinsen:%s %d%%%% alle %d Spielminuten, hoechstens $%d je Gutschrift\n\
        %sUeberweisung:%s %d%%%% Gebuehr\n\n\
        %s/deposit [Betrag]%s - einzahlen\n\
        %s/withdraw [Betrag]%s - abheben\n\
        %s/transfer [Spieler] [Betrag]%s - ueberweisen",
        EC_GREY, EC_GREEN, bank,
        EC_GREY, EC_WHITE, cash,
        EC_GREY, EC_WHITE, BANK_INTEREST_PERCENT, BANK_INTEREST_MINUTES, BANK_INTEREST_CAP,
        EC_GREY, EC_WHITE, BANK_TRANSFER_FEE,
        EC_WHITE, EC_GREY, EC_WHITE, EC_GREY, EC_WHITE, EC_GREY);

    return ShowPlayerDialog(playerid, DIALOG_BANK, DIALOG_STYLE_MSGBOX,
        "Bank", body, "Schliessen", "");
}

/*
 * Wird im Minutentakt aufgerufen und schreibt faellige Zinsen gut.
 */
stock Bank_Tick()
{
    foreach_player(i)
    {
        if (!IsLoggedIn(i)) continue;
        if (PlayerData[i][pBank] <= 0) continue;

        PlayerData[i][pBankMinutes]++;
        if (PlayerData[i][pBankMinutes] < BANK_INTEREST_MINUTES) continue;

        PlayerData[i][pBankMinutes] = 0;

        new interest = (PlayerData[i][pBank] * BANK_INTEREST_PERCENT) / 100;
        if (interest > BANK_INTEREST_CAP) interest = BANK_INTEREST_CAP;
        if (interest <= 0) continue;

        PlayerData[i][pBank] += interest;

        new money[24], msg[128];
        FormatNumber(interest, money, sizeof money);
        format(msg, sizeof msg, "Zinsgutschrift: $%s auf dein Bankkonto.", money);
        SendInfo(i, msg);
    }
    return 1;
}

// ---------------------------------------------------------------------------
// Kommandos
// ---------------------------------------------------------------------------
CMD:bank(playerid, params[])
{
    #pragma unused params
    return Bank_ShowMenu(playerid);
}

CMD:deposit(playerid, params[])
{
    require(Params_IsInt(params, 0), "Benutzung: /deposit [Betrag]");

    new amount = Params_Int(params, 0);
    require(amount > 0, "Der Betrag muss groesser als null sein.");
    require(GetPlayerMoney(playerid) >= amount, "So viel Bargeld hast du nicht.");

    GivePlayerMoney(playerid, -amount);
    PlayerData[playerid][pBank] += amount;

    new money[24], total[24], msg[144];
    FormatNumber(amount, money, sizeof money);
    FormatNumber(PlayerData[playerid][pBank], total, sizeof total);
    format(msg, sizeof msg, "$%s eingezahlt. Neuer Kontostand: $%s", money, total);
    SendSuccess(playerid, msg);
    return 1;
}
ALIAS:einzahlen(deposit)

CMD:withdraw(playerid, params[])
{
    require(Params_IsInt(params, 0), "Benutzung: /withdraw [Betrag]");

    new amount = Params_Int(params, 0);
    require(amount > 0, "Der Betrag muss groesser als null sein.");
    require(PlayerData[playerid][pBank] >= amount, "So viel liegt nicht auf deinem Konto.");

    PlayerData[playerid][pBank] -= amount;
    GivePlayerMoney(playerid, amount);

    new money[24], total[24], msg[144];
    FormatNumber(amount, money, sizeof money);
    FormatNumber(PlayerData[playerid][pBank], total, sizeof total);
    format(msg, sizeof msg, "$%s abgehoben. Neuer Kontostand: $%s", money, total);
    SendSuccess(playerid, msg);
    return 1;
}
ALIAS:abheben(withdraw)

CMD:transfer(playerid, params[])
{
    require(Params_Count(params) >= 2, "Benutzung: /transfer [Spieler] [Betrag]");

    new arg[MAX_PLAYER_NAME + 1];
    Params_Get(params, 0, arg, sizeof arg);

    new target = ResolveTarget(playerid, arg);
    if (target == INVALID_PLAYER_ID) return 1;
    require(target != playerid, "An dich selbst zu ueberweisen bringt nichts.");
    require(Params_IsInt(params, 1), "Der Betrag muss eine Zahl sein.");

    new amount = Params_Int(params, 1);
    require(amount > 0, "Der Betrag muss groesser als null sein.");

    new fee = (amount * BANK_TRANSFER_FEE) / 100;
    if (fee < 1) fee = 1;

    require(PlayerData[playerid][pBank] >= amount + fee, "Kontostand reicht nicht (Betrag plus Gebuehr).");

    PlayerData[playerid][pBank] -= (amount + fee);
    PlayerData[target][pBank]   += amount;

    new money[24], feeText[24], msg[176];
    FormatNumber(amount, money, sizeof money);
    FormatNumber(fee, feeText, sizeof feeText);

    format(msg, sizeof msg, "$%s an %s ueberwiesen (Gebuehr $%s).", money, GetName(target), feeText);
    SendSuccess(playerid, msg);

    format(msg, sizeof msg, "%s hat dir $%s ueberwiesen.", GetName(playerid), money);
    SendSuccess(target, msg);

    format(msg, sizeof msg, "[BANK] %s -> %s: $%s", GetName(playerid), GetName(target), money);
    Log(msg);
    return 1;
}
ALIAS:ueberweisen(transfer)
