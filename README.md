# UIF - United Islands Freeroam

Ein vollständiger **Freeroam-Gamemode für SA-MP 0.3.7 / open.mp**, aufgebaut nach dem
Vorbild von [uifserver.net](https://uifserver.net) — Teleports quer über San Andreas,
Fahrzeug- und Waffensets, Stuntwertung, Rennen, Derby, Häuser, ein gestuftes
Adminsystem und VIP-Ränge.

Der Gamemode kommt **ohne Plugins und ohne externe Includes** aus. Kommandoparser,
Parameter-Parsing (sscanf-Ersatz) und Datenspeicher sind Teil des Projekts; benötigt
wird ausschließlich `a_samp.inc`.

---

## Inhalt

- [Schnellstart](#schnellstart)
- [Serverpaket besorgen](#serverpaket-besorgen)
- [Funktionsumfang](#funktionsumfang)
- [Kommandoübersicht](#kommandoübersicht)
- [Projektaufbau](#projektaufbau)
- [Architektur](#architektur)
- [Datenspeicher](#datenspeicher)
- [Konfiguration](#konfiguration)
- [Stand der Verifikation](#stand-der-verifikation)

---

## Schnellstart

```bash
./setup.sh      # SA-MP-Includes holen, Pawn-Compiler aus den Quellen bauen
./compile.sh    # Gamemode übersetzen -> gamemodes/uif.amx
./run.sh        # Server starten (Serverpaket erforderlich, siehe unten)
```

Alternativ über `make`:

```bash
make setup
make compile
make run
```

`setup.sh` braucht `git`, `cmake`, `make` und `gcc`. Es legt den Compiler unter
`.toolchain/` und die Includes unter `pawno/include/` ab — beides ist von Git
ignoriert, im Repository liegt ausschließlich eigener Quellcode.

### Ersten Administrator einrichten

Admin- und VIP-Ränge stehen in der jeweiligen Accountdatei. Nach der ersten
Registrierung im Spiel:

```bash
# Server stoppen, dann in scriptfiles/accounts/<name>.ini setzen:
AdminLevel=5
```

Danach vergibt dieser Account weitere Ränge in-game per `/setlevel` und `/setvip`.

---

## Serverpaket besorgen

Die eigentliche Serveranwendung (`samp03svr` bzw. `omp-server`) ist proprietär und
**nicht Teil dieses Repositories**. Sie muss einmalig danebengelegt werden:

- **open.mp** (empfohlen, aktiv gepflegt, abwärtskompatibel zu SA-MP 0.3.7):
  Serverpaket von [open.mp](https://open.mp) laden, `omp-server` ins Projektverzeichnis
  legen.
- **SA-MP 0.3.7 R2-1** (Original): Linux-Serverpaket entpacken und `samp03svr` ins
  Projektverzeichnis legen.

Erwartetes Ergebnis im Projektverzeichnis:

```
samp03svr        (oder omp-server)
server.cfg       <- liegt bereits im Repository
gamemodes/uif.amx
scriptfiles/
```

`run.sh` erkennt beide Varianten automatisch.

---

## Funktionsumfang

### Accounts
Registrierung und Login über Dialoge. Passwörter werden mit **SHA256 und einem
zufälligen 16-Zeichen-Salt pro Account** gehasht (`SHA256_PassHash`); das
Klartextpasswort wird nie gespeichert. Drei Fehlversuche führen zum Kick.
Gespeichert werden Rang, Geld, Punkte, Kills/Tode, Spielzeit, Skin, Stuntpunkte,
Renn- und Derbysiege sowie der Hausbesitz. Automatische Sicherung im Minutentakt.

### Teleports
Über 40 Ziele in fünf Kategorien (Städte, Stuntzonen, Fun & Party, Militär,
VIP-Bereiche), erreichbar über das Menü `/tp`, per Namenssuche (`/tp chiliad`)
oder über Direktkommandos wie `/ls`, `/sf`, `/lv`, `/a51`, `/zt`. Fahrzeuge werden
mitteleportiert, danach gilt drei Sekunden Spawnschutz.

### Fahrzeuge
40 statische Fahrzeuge an den wichtigen Punkten der Karte. `/veh` spawnt jedes
Modell von 400 bis 611 — per ID oder Name (`/veh infernus`). Dazu `/fix`, `/flip`,
`/nos`, `/color`, `/lock`, `/dv` und `/vd` zum Abschalten des sichtbaren
Fahrzeugschadens (Einstellung wird im Account gespeichert). Jeder Spieler hat
höchstens ein selbst gespawntes Fahrzeug.

### Waffen
Zwölf vorgefertigte Sets vom Nahkampf bis „Rambo“, gestaffelt nach VIP-Stufe und
Teamrang. Einzelwaffen über `/gun [ID] [Munition]`; Raketenwerfer und Minigun sind
VIPs vorbehalten.

### Stunts
Sprünge mit dem Fahrzeug werden automatisch bewertet — Punkte ergeben sich aus
Flugzeit, Weite und Höhe. SA-MP meldet keinen Bodenkontakt, deshalb leitet das
Modul die Flugphase aus der vertikalen Geschwindigkeit ab (Details als Kommentar
in `src/features/stunt.inc`). Flugzeuge und Helikopter sind ausgenommen, VIPs
erhalten 25 % Bonus. `/ramp` setzt eine eigene Sprungrampe.

### Minispiele
- **Rennen** (`/race`, `/join`): vier Strecken mit Checkpoints, Lobby, Countdown und
  Preisgeld nach Teilnehmerzahl.
- **Derby** (`/derby`): vier Arenen, letzter fahrbereiter Wagen gewinnt. Läuft in
  einer eigenen Virtual World; wer aussteigt, scheidet aus.

### Häuser
15 kaufbare Häuser mit begehbaren Innenräumen, 3D-Textlabels und Pickups am
Eingang. Ein Haus pro Spieler, Rückkauf zu 70 % des Kaufpreises. Besucher dürfen
hinein, solange der Besitzer online ist. Manche Objekte sind VIPs vorbehalten.

### Administration
Fünf Stufen — Moderator, Admin, Senior Admin, Head Admin, Owner. Jede Stufe schaltet
weitere Kommandos frei, gegen ranghöhere oder gleichrangige Teammitglieder kann
niemand vorgehen. Alle eingreifenden Aktionen landen im Adminchat und im Logfile.
Bans werden namensbasiert in `scriptfiles/bans.ini` geführt und beim Verbinden
geprüft.

### VIP
Drei Stufen (Bronze, Silber, Gold) mit gestaffelten Vorteilen: eigene Waffensets,
VIP-Chat und -Teleports, `/vheal`, freie Namensfarbe, Jetpack, exklusive Häuser.

### Chat und Schutzmechanismen
Rangfarben im Hauptchat, private Nachrichten mit `/pm` und `/r`, getrennte Kanäle
für Team (`/a`) und VIPs (`/v`). Dazu Floodschutz mit automatischem Mute, ein
Werbefilter gegen Server-IPs und Domains sowie ein bewusst konservativer
Anti-Cheat, der nur eindeutig unmögliche Zustände ahndet und alles Übrige an das
Team meldet statt selbst zu bestrafen.

---

## Kommandoübersicht

### Alle Spieler

| Bereich | Kommandos |
|---|---|
| Teleport | `/tp` `/ls` `/sf` `/lv` `/a51` `/chiliad` `/zt` `/stunt` `/tpto` |
| Fahrzeuge | `/veh` `/car` `/dv` `/fix` `/flip` `/nos` `/color` `/lock` `/unlock` `/vd` |
| Waffen | `/weapons` `/w` `/guns` `/gun` `/disarm` |
| Minispiele | `/race` `/join` `/derby` `/leave` `/ramp` `/delramp` |
| Häuser | `/houses` `/buyhouse` `/sellhouse` `/enter` `/exit` `/myhouse` |
| Chat | `/pm` `/r` `/report` `/v` (VIP) |
| Sonstiges | `/help` `/cmds` `/rules` `/credits` `/stats` `/players` `/top` `/pos` `/kill` `/afk` `/pay` `/skin` `/jailtime` `/vip` |
| VIP | `/vheal` `/vcolor` `/vjetpack` |

### Team

| Stufe | Kommandos |
|---|---|
| 1 Moderator | `/acmds` `/kick` `/mute` `/unmute` `/warn` `/goto` `/gethere` `/spec` `/specoff` `/freeze` `/unfreeze` `/slap` `/ip` `/a` `/aduty` |
| 2 Admin | `/ban` `/jail` `/unjail` `/sethealth` `/setarmour` `/akill` `/explode` `/god` `/setskin` |
| 3 Senior Admin | `/unban` `/setmoney` `/setscore` `/ann` `/setweather` `/settime` |
| 4 Head Admin | `/setvip` |
| 5 Owner | `/setlevel` `/gmx` |

---

## Projektaufbau

```
gamemodes/uif.pwn          Einstiegspunkt: alle SA-MP-Callbacks, Kommando-Dispatcher
src/
  core/
    config.inc             Serverkennung, Farben, Dialog-IDs, Konstanten
    macros.inc             CMD:/ALIAS:-Makros, Parameter-Parser (sscanf-Ersatz)
    ini.inc                Schlanker key=value-Dateispeicher
    util.inc               Nachrichten, Spielersuche, Formatierung, Logging
    player.inc             Spielerdatenstruktur und Zugriffshelfer
    account.inc            Registrierung, Login, Speichern/Laden
    chat.inc               Hauptchat, Floodschutz, Werbefilter, PM, Kanäle
    commands.inc           Hilfe, Statistiken, Spielerliste, Servertipps
  features/
    teleport.inc           Teleportziele, Kategoriemenü, Spawnpunkte
    weapon.inc             Waffensets
    vehicle.inc            Statische und eigene Fahrzeuge, Modellnamen 400-611
    stunt.inc              Stuntwertung, Stuntzonen, Rampen
    house.inc              Kaufbare Häuser mit Innenräumen
  admin/
    admin.inc              Teamstufen, Bans, Jail, Adminkommandos
    vip.inc                VIP-Stufen und Vorteile
    anticheat.inc          Heuristiken gegen offensichtliche Manipulation
  minigames/
    race.inc               Rennen mit Checkpoints
    derby.inc              Derby-Arenen
server.cfg                 Serverkonfiguration
setup.sh compile.sh run.sh Build- und Startskripte
Makefile                   Kurzbefehle
```

---

## Architektur

Pawn kennt keine echten Module: **jeder SA-MP-Callback darf pro Skript nur einmal
existieren.** Deshalb liegt in `gamemodes/uif.pwn` die einzige Definition aller
Callbacks, und jedes Modul stellt eigene Funktionen bereit, die von dort
aufgerufen werden:

```pawn
public OnPlayerSpawn(playerid)
{
    ...
    if (IsPlayerJailed(playerid))      { Admin_PutInJail(playerid); return 1; }
    if (Derby_IsParticipant(playerid)) { Derby_RespawnParticipant(playerid); return 1; }

    Teleport_SpawnPlayer(playerid);
    AntiCheat_OnPlayerSpawn(playerid);
    return 1;
}
```

Da Pawn in einem Durchgang übersetzt, muss jede Funktion vor ihrer Verwendung
definiert sein. Die Include-Reihenfolge im Hauptskript ist deshalb bedeutsam:
Kern vor Features vor Team vor Minispielen. Zustände, die mehrere Module abfragen,
liegen bewusst im Kern — `IsPlayerJailed()` steht etwa in `player.inc` und nicht im
Adminmodul, damit `teleport.inc` nicht auf ein später eingebundenes Modul zeigt.

### Kommandos ohne zcmd

`CMD:name(playerid, params[])` legt eine `public`-Funktion `cmd_name` an. Der
Dispatcher in `OnPlayerCommandText` normalisiert den eingegebenen Namen auf
Kleinbuchstaben und ruft sie über `CallLocalFunction` auf — gibt sie 0 zurück oder
existiert sie nicht, meldet der Server „unbekanntes Kommando“. Kommandos müssen
also 1 zurückgeben; das entspricht der zcmd-Konvention.

### Parameter ohne sscanf

`Params_Count`, `Params_Get`, `Params_Int`, `Params_Float`, `Params_Rest` und
`Params_IsInt` ersetzen das sscanf-Plugin. Das Makro `require(bedingung, "text")`
bricht ein Kommando mit einer Fehlermeldung ab:

```pawn
CMD:pay(playerid, params[])
{
    require(Params_Count(params) >= 2, "Benutzung: /pay [Spieler] [Betrag]");
    ...
}
```

Zwei Eigenheiten des Pawn-Präprozessors, die im Code sichtbar sind: ein
`require`-Aufruf muss **auf einer Zeile** stehen, und `ALIAS:` wird **ohne
abschließendes Semikolon** geschrieben.

---

## Datenspeicher

Kein MySQL-Plugin nötig — alles liegt als lesbare Textdatei unter `scriptfiles/`:

| Datei | Inhalt |
|---|---|
| `accounts/<name>.ini` | ein Account (Name kleingeschrieben) |
| `bans.ini` | `<name>=<grund> \| von <admin> \| <datum>` |
| `houses.ini` | `House<n>=<besitzer>` |
| `logs/server.log` | Adminaktionen, Anti-Cheat-Meldungen, Serverereignisse |

Beispiel einer Accountdatei:

```ini
Password=a3f5...        ; SHA256 aus Passwort + Salt
Salt=Xk29fPq1LmZ4vB0r
RegDate=11.08.2026 14:22:05
AdminLevel=0
VipLevel=0
Score=142
Money=87500
Kills=61
Deaths=44
PlayTime=18240
StuntPoints=930
```

Für sehr große Spielerzahlen ist ein Datenbank-Backend die bessere Wahl; die
INI-Schicht ist bewusst hinter `ini.inc` gekapselt, sodass sie ersetzbar bleibt.

---

## Konfiguration

Die wichtigsten Stellschrauben:

| Wo | Was |
|---|---|
| `server.cfg` | Hostname, Port, Slots, **rcon_password** |
| `src/core/config.inc` | Servername, Website, Farben, Spawnschutz, AFK-Schwelle |
| `src/core/chat.inc` | Flood-Intervall, Automute-Dauer, Werbefilter-Domains |
| `src/admin/anticheat.inc` | Schwellwerte und Verwarnungslimit |
| `src/features/stunt.inc` | Punkteformel und Absprung-/Landeschwellen |
| `src/minigames/race.inc` | Lobbyzeit, Countdown, Zeitlimit, Preisgeld |

> **Vor dem ersten Betrieb:** `rcon_password` in `server.cfg` ändern. Der
> Auslieferungswert ist ein Platzhalter, `run.sh` warnt beim Start davor.

---

## Stand der Verifikation

Was geprüft ist und was nicht — damit klar ist, worauf man sich verlassen kann:

- **Übersetzt fehlerfrei** mit Pawn 3.10.10 (pawn-lang Community-Compiler, Tag
  `v3.10.10`) gegen pawn-lang/samp-stdlib, Commit `8ffb055`, mit den strengen
  Optionen `-d3 -;+ -(+`: **0 Fehler, 0 Warnungen.** Ergebnis:
  `gamemodes/uif.amx`, rund 346 KB.
- Der Ablauf `setup.sh` → `compile.sh` wurde aus einem **komplett leeren Baum**
  durchgespielt (ohne Toolchain, ohne Includes) und lief fehlerfrei durch.
- Die Includes sind bewusst auf einen Commit des `master`-Zweigs gepinnt und
  nicht auf das Release `0.3.7-R2-2-1`. Das Release enthält die unveränderten
  Original-Includes von SA-MP; die deklarieren `print`/`printf` selbst — was mit
  der `console.inc` des Pawn-Compilers kollidiert — und führen `SHA256_PassHash`
  ohne `const`-Parameter. Beides verhindert die Übersetzung. Der `master`-Zweig
  stammt aus derselben Organisation wie der Compiler und ist darauf abgestimmt.
- Der Stackbedarf wurde geprüft: der Compiler schätzt den Spitzenwert auf 8843
  Zellen, deshalb setzt das Hauptskript `#pragma dynamic 16384` statt der
  Standardgröße von 4096 Zellen.
- **Nicht im Spiel getestet.** Die Serveranwendung ist proprietär und in der
  Entwicklungsumgebung dieses Projekts nicht verfügbar gewesen, ein Laufzeittest
  mit echten Clients steht also aus. Betroffen sind vor allem die
  erfahrungsabhängigen Werte: Stuntschwellen, Anti-Cheat-Grenzen sowie einige
  Teleport-, Haus- und Rennkoordinaten, die auf Kartenwissen beruhen und beim
  ersten Durchlauf feinjustiert werden sollten.
- Das Objektmodell der Sprungrampe (`STUNT_RAMP_OBJECT`, Modell 1655) ist eine
  Konstante an einer Stelle — falls die Rampe im Spiel nicht passt, genügt es,
  dort ein anderes Modell einzutragen.
