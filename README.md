# Jebiga-Gaming

Ein vollständiger **Freeroam-Gamemode für SA-MP 0.3.7 / open.mp**, aufgebaut nach dem
Vorbild von [jebiga-gaming.net](https://jebiga-gaming.net) — Teleports quer über San Andreas,
Fahrzeug- und Waffensets, Stuntwertung, Rennen, Derby, Häuser, ein gestuftes
Adminsystem und VIP-Ränge.

**Windows ist die Hauptplattform** — `setup.ps1` richtet alles mit einem Befehl
ein. Linux läuft ebenso, dort werden die Plugins aus den Quellen gebaut statt
geladen.

Wahlweise auf **MySQL** (das Schema legt der Server beim Start selbst an) oder
auf Textdateien. Kommandoparser und Parameter-Parsing sind Teil des Projekts.

---

## Inhalt

- [Schnellstart (Windows)](#schnellstart-windows)
- [Linux](#linux)
- [MySQL](#mysql)
- [Plugins](#plugins)
- [Filterscripts](#filterscripts)
- [Serverpaket besorgen](#serverpaket-besorgen)
- [Speedboost auf Taste 2](#speedboost-auf-taste-2)
- [Custom Map](#custom-map)
- [Referenzdaten](#referenzdaten)
- [Funktionsumfang](#funktionsumfang)
- [Kommandoübersicht](#kommandoübersicht)
- [Projektaufbau](#projektaufbau)
- [Architektur](#architektur)
- [Datenspeicher](#datenspeicher)
- [Konfiguration](#konfiguration)
- [Stand der Verifikation](#stand-der-verifikation)

---

## Schnellstart (Windows)

```powershell
powershell -ExecutionPolicy Bypass -File .\setup.ps1
.\compile.bat
.\run.bat
```

`setup.ps1` holt alles: Pawn-Compiler, SA-MP-Includes, die Plugins samt
passender Include-Dateien, `a_mysql.inc` und die Laufzeitverzeichnisse.
Voraussetzung ist nur **git für Windows** und PowerShell — die mitgelieferte
Version 5.1 genügt, das Skript meidet bewusst alles Neuere.

Danach fehlen noch zwei Dinge:

1. **Serverpaket** danebenlegen (`samp-server.exe` bzw. `omp-server.exe`) —
   siehe [Serverpaket besorgen](#serverpaket-besorgen).
2. **`rcon_password`** in `server.cfg` ändern.

### Wenn ein Download nicht klappt

Das Skript bricht dann **nicht** ab. Es merkt sich den Ausfall, macht weiter
und sagt am Ende, welche Datei fehlt, wohin sie gehört und auf welcher
Release-Seite sie liegt. Ein halb eingerichteter Baum ohne Hinweis wäre
schlimmer als ein klarer Abbruch.

| Datei | Rolle |
|---|---|
| `streamer.dll` | **Pflicht** — ohne den Streamer fehlt die Custom Map |
| `crashdetect.dll` | optional, aber beim Einfahren sehr zu empfehlen |
| `sscanf.dll` | optional; das Gamemode braucht es nicht |
| `mysql.dll` | nur bei `Enabled=1` in `scriptfiles\mysql.ini` |

Fehlt ein optionales Plugin dauerhaft, muss es aus der `plugins`-Zeile in
`server.cfg` gestrichen werden — sonst verweigert der Server den Start.

Aus jedem Plugin-Archiv übernimmt das Skript **auch die mitgelieferte
`.inc`-Datei**, nicht nur die DLL. So passen Include und Binary immer
zusammen; ein gemischtes Paar wäre die Art Fehler, die erst zur Laufzeit
auffällt.

---

## Linux

```bash
./setup.sh      # Includes holen, Compiler und Plugins aus den Quellen bauen
./compile.sh    # Gamemode und Filterscripts übersetzen
./run.sh        # Server starten
```

Oder über `make setup`, `make compile`, `make run`.

Unter Linux werden die Plugins **aus den Quellen gebaut** statt geladen. Dafür
braucht `setup.sh` neben `git`, `cmake`, `make` und `gcc` eine
**32-Bit-C++-Toolchain** — SA-MP lädt nur 32-Bit-Plugins, auch auf
64-Bit-Systemen:

```bash
# Debian / Ubuntu
apt-get install g++-multilib lib32stdc++-14-dev
# Fedora
dnf install glibc-devel.i686 libstdc++-devel.i686
```

`setup.sh` stellt außerdem die Plugin-Endungen in `server.cfg` von `.dll` auf
`.so` um — ausgeliefert wird die Windows-Fassung.

### Warum die Versionen sich unterscheiden

Unter Windows kommt der Streamer als Release **v2.9.6**, unter Linux baut
`setup.sh` den **master**-Zweig. Grund: v2.9.6 zieht seine Abhängigkeiten aus
`Zeex/cmake-modules`, und dieses Repository ist nicht mehr erreichbar; der
master-Zweig nutzt gepflegte Forks und lässt sich bauen.

Das ist unkritisch, weil jede Seite ihre **eigene passende `streamer.inc`**
bekommt und das Gamemode nur seit Jahren stabile Natives benutzt
(`CreateDynamicObject`, `CreateDynamic3DTextLabel`, `CreateDynamicMapIcon`).

### Ersten Administrator einrichten

Nach der ersten Registrierung im Spiel — je nach Speicherweg:

```ini
; Dateispeicher: Server stoppen, dann in scriptfiles/accounts/<name>.ini
AdminLevel=5
```

```sql
-- MySQL: geht im laufenden Betrieb, wirkt beim naechsten Login
UPDATE accounts SET adminlevel = 5 WHERE name = 'DeinName';
```

Danach vergibt dieser Account weitere Ränge in-game per `/setlevel` und
`/setvip`.

---

## MySQL

Der Server läuft wahlweise auf MySQL oder auf den Textdateien in `scriptfiles/`.
Beim ersten Start legt er `scriptfiles/mysql.ini` an:

```ini
Enabled=0
Host=127.0.0.1
User=jebiga
Password=
Database=jebiga
Port=3306
```

`Enabled=1` setzen, Zugangsdaten eintragen, fertig — **eine leere Datenbank
genügt.** Die Tabellen legt der Server beim Start selbst an
(`CREATE TABLE IF NOT EXISTS`), von Hand einzuspielen ist nichts:

| Tabelle | Inhalt |
|---|---|
| `accounts` | ein Datensatz je Spieler, `name` mit eindeutigem Index |
| `bans` | namensbasierte Sperren mit Grund, Admin und Datum |
| `houses` | Besitzer je Hausplatz |
| `crews` | Name, Kürzel, Leitung, Punkte |

Scheitert die Verbindung, **bricht der Start nicht ab** — der Server schreibt
den Grund ins Log und läuft auf Dateien weiter. Ein Server ohne Datenbank ist
besser als kein Server.

### Synchron und asynchron

Schemaanlage und das Laden von Häusern und Crews laufen **synchron**. Das
blockiert, passiert aber genau einmal beim Hochfahren, bevor Spieler verbunden
sind.

Alles zur Laufzeit — Account laden, speichern, Bans schreiben — läuft
**asynchron**. Bei 700 Spielern würde eine blockierende Abfrage sonst den
gesamten Server anhalten.

Der Unterschied ist nicht nur das Ziel, sondern der Ablauf: Dateien liest man
sofort, MySQL antwortet erst später. Damit der Rest des Gamemodes davon nichts
merkt, ist das Laden in beiden Fällen **zweistufig** modelliert —
`Account_BeginLoad()` stößt an, `Account_AfterLoad()` macht weiter. Der
Dateiweg ruft die zweite Stufe direkt auf, der MySQL-Weg aus dem
Abfrage-Callback heraus. Die Ban-Prüfung beim Verbinden funktioniert genauso
und stößt danach das Laden des Accounts an.

---

## Plugins

| Plugin | Rolle | Windows | Linux |
|---|---|---|---|
| **streamer** (Incognito) | Objekte, Textlabels und Map-Icons jenseits der SA-MP-Grenzen — trägt die Custom Map | Download v2.9.6 | Build aus master |
| **crashdetect** (Zeex, Fork Y-Less) | zeigt bei Laufzeitfehlern Datei und Zeilennummer statt nur einer Speicheradresse | Download v4.22 | Build aus Fork |
| **sscanf2** (maddinat0r) | Parameterzerlegung; das Gamemode braucht sie nicht, eigene Filterscripts profitieren davon | Download v2.13.8 | Build v2.13.8 |
| **mysql** (pBlueG, R41-4) | optionaler Datenspeicher | Download | nicht baubar, siehe unten |

Unter Linux entstehen die Plugins als **32-Bit-Shared-Objects** — SA-MP lädt nur
32-Bit-Binaries, auch auf 64-Bit-Systemen.

Zwei Eigenheiten, die `setup.sh` beim Bauen unter Linux automatisch behandelt:

- **subhook**: sscanf2 und crashdetect hängen davon ab. Das Originalrepository
  von Zeex ist nicht mehr erreichbar, deshalb zeigt `setup.sh` die Submodule auf
  den gepflegten Fork von Y-Less um.
- **crashdetect und moderne Compiler**: der mitgelieferte AMX-Interpreter nutzt
  „labels as values". Seit GCC 12 verlangt der Compiler dort einen echten
  Zeigertyp, sonst bricht der Build mit *computed goto must be pointer type* ab.
  `setup.sh` setzt den nötigen Cast.

**Das MySQL-Plugin baut `setup.sh` nicht.** Es hängt an `log-core`, das
wiederum `Zeex/cmake-modules` braucht — dasselbe unerreichbare Repository wie
oben, hier ohne verfügbaren Fork. `setup.sh` erzeugt nur `a_mysql.inc`, damit
das Gamemode übersetzt. Das Plugin selbst gibt es fertig gebaut für Linux und
Windows beim Projekt pBlueG/SA-MP-MySQL; die Datei gehört nach `plugins/` und
in die `plugins`-Zeile der `server.cfg`. Ohne das Plugin läuft der Server auf
Dateien weiter.

---

## Filterscripts

Vier eigenständige Skripte, unabhängig vom Gamemode. Sie lassen sich im
laufenden Betrieb mit `/rcon reloadfs <name>` neu laden, ohne dass Spieler
herausfliegen:

| Filterscript | Was es tut |
|---|---|
| `gates` | Fünf automatische Tore, die sich öffnen, sobald jemand näher kommt. `/gates` zeigt den Zustand |
| `lottery` | Ziehung alle 15 Minuten, `/lotto [Zahl]` kauft ein Los. Ohne Gewinner wächst der Topf weiter |
| `weathercycle` | Uhrzeit läuft beschleunigt weiter, Wetter wechselt in unregelmäßigen Abständen. `/zeit` |
| `speedcam` | Acht Radarfallen mit Kulanzgrenze, Sperrzeit und gedeckeltem Bußgeld. `/blitzer` |

Eingetragen sind sie in `server.cfg` unter `filterscripts`.

---

### Ersten Administrator einrichten

Nach der ersten Registrierung im Spiel — je nach Speicherweg:

```bash
# Dateispeicher: Server stoppen, dann in scriptfiles/accounts/<name>.ini setzen
AdminLevel=5
```

```sql
-- MySQL: geht im laufenden Betrieb, wirkt beim naechsten Login
UPDATE accounts SET adminlevel = 5 WHERE name = 'DeinName';
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
gamemodes/jebiga.amx
scriptfiles/
```

`run.sh` erkennt beide Varianten automatisch.

---

## Speedboost auf Taste 2

Das Markenzeichen von Jebiga: **im Fahrzeug die Taste 2 spammen gibt Schub.** Jeder
Tastendruck setzt einen Impuls in Fahrtrichtung; schnelles Drücken verkettet die
Impulse zu einer Beschleunigung.

Technisch liegt das auf `KEY_SUBMISSION` — die Taste, auf der GTA San Andreas
standardmäßig die „Sub-Mission" hat, also die 2 in der Zahlenreihe. Da
`OnPlayerKeyStateChange` pro Tastendruck feuert, entsteht das Spam-Verhalten von
selbst.

Damit daraus kein Dauerflug wird, hängt der Schub an einem Energievorrat:

| Größe | Wert | VIP |
|---|---|---|
| Vorrat | 100 | 100 |
| Kosten je Schub | 8 | 8 |
| Aufladung | 12/Sekunde | 18/Sekunde |
| Schubkraft | 0,28 | 0,34 |
| Mindestabstand | 110 ms | 110 ms |
| Höchstgeschwindigkeit | ~305 km/h | ~305 km/h |

Ergebnis: ein kurzer, heftiger Burst von rund zwölf Schüben, danach lädt der
Vorrat nach — genau der Rhythmus, den man vom Original kennt. Der Boostbalken
sitzt im HUD unter dem Tacho. Fluggeräte, Schienenfahrzeuge und ferngesteuerte
Modelle sind ausgenommen, im Rennen ist der Boost deaktiviert (im Derby nicht —
dort hat ihn jeder). `/boost` erklärt alles im Spiel.

---

## Custom Map

Vier eigene Bauwerke, komplett über den Streamer erzeugt:

| Ort | Inhalt |
|---|---|
| **Jebiga Stuntpark** (`/stuntpark`) | Schwebende Plattform von 240 × 180 m mit großem Looping, zweitem kleinen Looping, Sprungschanze mit Landerampe, Röhren- und Baumstammrampen, Schanzen im Karree |
| **VIP-Lounge** | Plattform mit umlaufendem Geländer und Ausfahrschanzen |
| **Gold-Insel** | Plattform über dem Meer, Gold-VIPs vorbehalten |
| **Derby-Arena** | Quadratische Umrandung samt Hindernissen — nur in der Derby-Welt sichtbar, im Freeroam steht sie niemandem im Weg |

### Wie die Objekte ausgewählt wurden

Modell-IDs aus dem Gedächtnis zu raten führt zu unsichtbaren oder falschen
Objekten. Stattdessen wurden die Modelle gegen die Objektdatenbank von GTA San
Andreas geprüft — inklusive ihrer tatsächlichen Abmessungen und, wichtiger, der
Lage des Modellursprungs.

Beispiel `loopbig`: Die Geometrie reicht auf der Z-Achse von −11,57 bis +11,59,
der Ursprung sitzt also mittig. Ein Looping, der auf dem Boden stehen soll, muss
folglich um 11,57 angehoben werden. Genau dafür stehen die `LIFT_*`-Konstanten in
`src/features/map.inc` — jedes Objekt sitzt damit sauber auf, statt halb im Boden
zu stecken.

Als Untergrund dienen **Straßenstücke** statt Bodenplatten: Straßen sind
garantiert kollidierbar. Eine reine Bodenmarkierung mit null Höhe sähe zwar
sauberer aus, ließe die Spieler aber womöglich hindurchfallen.

Die Umrandungen sind **quadratisch, nicht rund**. Ein Kreis aus Absperrungen
bräuchte pro Segment eine gedrehte Ausrichtung; da sich die Drehkonvention hier
nicht im Spiel prüfen ließ, hätte ein Vorzeichenfehler die Wand zerlegt. Vier
gerade Wände mit 0° und 90° sind eindeutig.

---

## Referenzdaten

Fünf Tabellen unter `src/data/` liefern die Grundlage für Skinauswahl, Tuning,
Innenraum-Reisen, Animationen und Waffeninfos:

| Datei | Inhalt |
|---|---|
| `skins.inc` | 312 Skins mit Namen und Geschlecht, lückenlos von 0 bis 311 |
| `weapons.inc` | 47 Waffen mit Namen und Inventarplatz |
| `components.inc` | 194 Tuningbauteile mit Kategorie, Bezeichnung und Originalpreis |
| `interiors.inc` | 127 Innenräume mit Interior-ID und Koordinaten |
| `animations.inc` | 1716 Animationen aus 132 Bibliotheken |

**Diese Dateien sind erzeugt, nicht getippt.** `tools/gen_data.py` liest die
Referenzdokumentation von open.mp und schreibt daraus die Pawn-Tabellen:

```bash
git clone --depth 1 https://github.com/openmultiplayer/wiki /tmp/ompwiki
python3 tools/gen_data.py /tmp/ompwiki
```

Der Grund ist schlicht: bei 312 Skinnamen und 1716 Animationen wäre Abtippen aus
dem Gedächtnis eine Fehlerquelle, die niemand bemerkt — ein falscher
Animationsname wirft keinen Fehler, die Animation spielt einfach nicht. Der
Generator prüft außerdem mit, ob die Skinliste Lücken hat, und bricht dann ab.

Die erzeugten Dateien sind eingecheckt; das Skript muss nur laufen, wenn die
Referenzdaten aktualisiert werden sollen.

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

### Deathmatch und Duelle
Fünf dauerhaft offene DM-Arenen (Steinbruch, Area 51, Chiliad, LS Stadion,
Bayside), jede mit eigenem Waffenset und eigener virtueller Welt. Beitritt
jederzeit über `/dm`, Tod führt zum Respawn in derselben Arena. Dazu 1-gegen-1-
**Duelle** auf Einladung (`/duel`, `/accept`) mit frei wählbarer Waffe, eigenem
Duellplatz und Preisgeld für den Sieger.

### Crews
Spielergruppen mit Kürzel, das in jeder Chatzeile erscheint, eigenem Crewchat
(`/c`), Einladungen und Mitgliederliste. Gespeichert wird im Account der Crew*name*
und nicht ihr Index — Indizes verschieben sich, wenn eine Crew aufgelöst wird, der
Name bleibt stabil.

### HUD
Zwei Textdraws am unteren Bildschirmrand, sichtbar sobald man in einem Fahrzeug
sitzt: Modellname und Geschwindigkeit in km/h, darunter der Boostvorrat als
Balken. Aktualisiert wird nur, wer tatsächlich fährt.

### Animationen und Spaßbefehle
Browser über alle 1716 Animationen (`/anims`), erst Bibliothek, dann Animation,
dazu Suche über den Namen. Außerdem `/rocket`, `/eject`, `/parachute`,
`/jetpack` und `/weaponinfo`.

### Skinauswahl
Alle 312 Skins, nach Geschlecht gefiltert und seitenweise blätterbar (`/skins`),
dazu Suche über den Namen — `/skins ballas` findet, was gemeint ist. Ein Wechsel
kostet Geld, die Wahl wird im Account gespeichert.

### Tuning
Alle 194 Bauteile nach Kategorie sortiert, mit den Originalpreisen aus dem Spiel.
Nicht jedes Teil passt an jedes Fahrzeug — deshalb wird nach dem Einbau über
`GetVehicleComponentInSlot` geprüft, ob es wirklich sitzt, und nur dann bezahlt.
`/untune` räumt alle 17 Bauteilslots wieder leer.

### Innenräume
127 Innenräume von San Andreas über `/interiors`, nach Kategorie gruppiert
(24/7-Läden, Clubs, Restaurants, Ämter, Tuningwerkstätten …). Die Position
draußen wird gemerkt, `/exitint` führt exakt dorthin zurück.

### Jobs
Sechs bezahlte Routen — zwei Fernfahrten, Kurierdienst, Müllabfuhr, Buslinie und
ein Frachtflug. Jeder Job stellt das passende Fahrzeug, setzt die Halte als
Checkpoints und zahlt pro erreichtem Punkt plus Abschlussprämie. Wer sich mehr
als 400 Meter vom Arbeitsfahrzeug entfernt, verliert den Auftrag — sonst ließe
sich die Route zu Fuß oder per Teleport abkürzen.

### Bank
Zweites Konto neben dem Bargeld, mit Zinsen alle 30 **Spiel**minuten (nicht
Echtzeitminuten — sonst wäre Nichtstun die beste Strategie). Überweisungen
zwischen Spielern gegen Gebühr.

### Erfolge
20 Ziele mit Geldprämien, von „Erster Abschuss" bis „100 Stunden gespielt".
Die Prüfung sitzt an einer einzigen Stelle und wertet die vorhandenen
Statistiken aus, statt jedes Modul mit Erfolgs-Aufrufen zu durchsetzen — ein
neuer Erfolg braucht dadurch einen Tabelleneintrag und eine Zeile Auswertung.

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
| Fahrzeuge | `/veh` `/car` `/dv` `/fix` `/flip` `/nos` `/color` `/lock` `/unlock` `/vd` `/tune` `/untune` |
| Waffen | `/weapons` `/w` `/guns` `/gun` `/disarm` |
| Boost | **Taste 2 im Fahrzeug** · `/boost` `/nitro` |
| Minispiele | `/race` `/join` `/derby` `/leave` `/ramp` `/delramp` `/stuntpark` |
| Deathmatch | `/dm` `/arena` `/leavedm` `/duel [Spieler]` `/accept` |
| Crew | `/crew` `/crews` `/createcrew` `/invitecrew` `/joincrew` `/leavecrew` `/c` |
| Häuser | `/houses` `/buyhouse` `/sellhouse` `/enter` `/exit` `/myhouse` |
| Chat | `/pm` `/r` `/report` `/v` (VIP) |
| Geld & Arbeit | `/bank` `/deposit` `/withdraw` `/transfer` `/jobs` `/quitjob` `/pay` |
| Aussehen & Orte | `/skins` `/interiors` `/int` `/exitint` |
| Sonstiges | `/help` `/cmds` `/rules` `/credits` `/stats` `/players` `/top` `/pos` `/kill` `/afk` `/jailtime` `/vip` `/achievements` |
| Spaß | `/anims` `/stopanim` `/weaponinfo` `/rocket` `/eject` `/parachute` `/jetpack` |
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
gamemodes/jebiga.pwn          Einstiegspunkt: alle SA-MP-Callbacks, Kommando-Dispatcher
src/
  core/
    config.inc             Serverkennung, Farben, Dialog-IDs, Welten, Mapkoordinaten
    db.inc                 MySQL-Verbindung, Schemaanlage, Rückfall auf Dateien
    macros.inc             CMD:/ALIAS:-Makros, Parameter-Parser (sscanf-Ersatz)
    ini.inc                Schlanker key=value-Dateispeicher
    util.inc               Nachrichten, Spielersuche, Formatierung, Logging
    player.inc             Spielerdatenstruktur und Zugriffshelfer
    account.inc            Registrierung, Login, Speichern/Laden
    chat.inc               Hauptchat, Floodschutz, Werbefilter, PM, Kanäle
    hud.inc                Tacho und Boostbalken als Player-Textdraws
    commands.inc           Hilfe, Statistiken, Spielerliste, Servertipps
  data/                    ERZEUGT von tools/gen_data.py - nicht von Hand ändern
    skins.inc              312 Skins
    weapons.inc            47 Waffen
    components.inc         194 Tuningbauteile
    interiors.inc          127 Innenräume
    animations.inc         1716 Animationen
  features/
    teleport.inc           Teleportziele, Kategoriemenü, Spawnpunkte
    boost.inc              Speedboost auf Taste 2
    weapon.inc             Waffensets
    vehicle.inc            Statische und eigene Fahrzeuge, Modellnamen 400-611
    stunt.inc              Stuntwertung, Stuntzonen, Rampen
    house.inc              Kaufbare Häuser mit Innenräumen
    map.inc                Custom Map: Stuntpark, Lounge, Insel, Arenawände
    crew.inc               Crews mit Kürzel, Chat und Einladungen
    skinshop.inc           Skinauswahl mit Filter und Seitenblättern
    tuning.inc             Tuningmenü über alle Bauteile
    interior.inc           Reisen in die Innenräume von San Andreas
    job.inc                Sechs bezahlte Routen
    bank.inc               Konto, Zinsen, Überweisungen
    achievement.inc        20 Erfolge mit Prämien
    extras.inc             Animationsbrowser und Spaßbefehle
  admin/
    admin.inc              Teamstufen, Bans, Jail, Adminkommandos
    vip.inc                VIP-Stufen und Vorteile
    anticheat.inc          Heuristiken gegen offensichtliche Manipulation
  minigames/
    race.inc               Rennen mit Checkpoints
    derby.inc              Derby-Arenen
    arena.inc              Deathmatch-Arenen und 1-gegen-1-Duelle
filterscripts/
  gates.pwn                Automatische Tore
  lottery.pwn              Lotterie mit wachsendem Topf
  weathercycle.pwn         Tages- und Wetterzyklus
  speedcam.pwn             Radarfallen
tools/gen_data.py          Generator für die Referenzdaten
setup.ps1 compile.bat run.bat   Windows (Hauptplattform)
setup.sh compile.sh run.sh      Linux
server.cfg                 Serverkonfiguration (Windows-Fassung, .dll)
Makefile                   Kurzbefehle unter Linux
```

---

## Architektur

Pawn kennt keine echten Module: **jeder SA-MP-Callback darf pro Skript nur einmal
existieren.** Deshalb liegt in `gamemodes/jebiga.pwn` die einzige Definition aller
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

### Einstiegspunkt beim Login

Manche Module müssen beim Login etwas tun, werden aber erst nach dem
Accountsystem eingebunden — das Crewmodul etwa löst dann den gespeicherten
Crewnamen in seinen Laufzeitindex auf. Dafür gibt es einen eigenen Haken:
`account.inc` deklariert `forward OnPlayerLoggedIn(playerid)` und ruft ihn nach
erfolgreichem Login auf, definiert wird er im Hauptskript. So kann jedes Modul
reagieren, ohne dass die Include-Reihenfolge umgestellt werden muss.

Aus demselben Grund steht `crew.inc` **vor** `chat.inc`: die Chatzeile zeigt das
Crewkürzel, also muss das Crewmodul zu diesem Zeitpunkt bereits bekannt sein.

---

## Datenspeicher

Kein MySQL-Plugin nötig — alles liegt als lesbare Textdatei unter `scriptfiles/`:

Ohne MySQL liegt alles als lesbare Textdatei unter `scriptfiles/`:

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
| `src/features/crew.inc` | Gründungskosten, maximale Crewzahl |
| `src/admin/anticheat.inc` | Schwellwerte und Verwarnungslimit |
| `src/features/stunt.inc` | Punkteformel und Absprung-/Landeschwellen |
| `src/minigames/race.inc` | Lobbyzeit, Countdown, Zeitlimit, Preisgeld |
| `src/features/boost.inc` | Schubkraft, Energiekosten, Aufladung, Tempolimit |
| `src/features/map.inc` | Modell-IDs und Aufsetzhöhen der Custom Map |
| `src/core/hud.inc` | Position und Aktualisierungsrate der Anzeigen |
| `src/features/bank.inc` | Zinssatz, Intervall, Deckel, Überweisungsgebühr |
| `src/features/job.inc` | Routen, Lohn je Halt, Abschlussprämie |
| `src/features/achievement.inc` | Ziele und Prämien |
| `src/features/skinshop.inc` | Preis eines Skinwechsels |
| `scriptfiles/mysql.ini` | Datenbankzugang, `Enabled` schaltet um |
| `filterscripts/*.pwn` | Tore, Lottopreise, Wetterintervalle, Tempolimit |

> **Vor dem ersten Betrieb:** `rcon_password` in `server.cfg` ändern. Der
> Auslieferungswert ist ein Platzhalter, `run.sh` warnt beim Start davor.

---

## Stand der Verifikation

### Geprüft

- **Übersetzt fehlerfrei** mit Pawn 3.10.10 gegen pawn-lang/samp-stdlib, Commit
  `8ffb055`, mit den strengen Optionen `-d3 -;+ -(+`: **0 Fehler, 0 Warnungen** —
  Gamemode und alle vier Filterscripts.
- Der Linux-Ablauf `setup.sh` → `compile.sh` wurde aus einem **komplett leeren
  Baum** durchgespielt: Includes geholt, Compiler gebaut, drei Plugins gebaut,
  `server.cfg` von `.dll` auf `.so` umgestellt, alles übersetzt.
- Die drei unter Linux gebauten Plugins sind **ELF-32-Bit-Shared-Objects** —
  geprüft, nicht angenommen.
- **Keine Namenskollisionen** unter den 138 Kommandos.
- Die Versionsnummern der Plugins stammen aus den **echten Release-Tags** der
  jeweiligen Repositories, nicht aus dem Gedächtnis.
- Die Modell-IDs der Custom Map sind gegen die Objektdatenbank von GTA San
  Andreas geprüft, inklusive Abmessungen und Ursprungslage.
- Die Referenzdaten sind aus der open.mp-Dokumentation erzeugt, nicht abgetippt.
  Der Generator ist deterministisch und bricht bei Lücken ab.
- Der Stackbedarf wurde geprüft: Spitzenwert 9733 Zellen, deshalb
  `#pragma dynamic 16384` statt der Standardgröße von 4096.

### Nicht geprüft

**Der Server wurde nie gestartet** und **die Windows-Skripte nie ausgeführt.**
Die Entwicklung lief auf Linux; die Serveranwendung ist proprietär und war über
keinen erreichbaren Mirror zu bekommen.

| Bereich | Was zu prüfen ist |
|---|---|
| **Plugin-Downloads in `setup.ps1`** | Die Adressen der Release-Archive ließen sich aus der Entwicklungsumgebung nicht abrufen — GitHub-Releases waren dort gesperrt. Die Versionen stimmen, die Dateinamen der Archive folgen der üblichen Benennung, sind aber ungeprüft. Läuft ein Download ins Leere, nennt das Skript die Release-Seite und macht weiter |
| **MySQL** | Der gesamte Datenbankweg ist ungetestet — kein Server, kein Plugin, keine Datenbank. Schema und Abfragen sind gegen die API von R41-4 geschrieben, aber nie ausgeführt. **Zuerst mit einer Wegwerfdatenbank ausprobieren** |
| `setup.ps1`, `compile.bat`, `run.bat` | unter Linux geschrieben, dort nicht ausführbar. Die Batchdatei quotet `-;+` und `-(+`, weil `;` und `(` für die Eingabeaufforderung Trennzeichen sind; die PowerShell-Fassung meidet alles, was es erst ab Version 6 gibt |
| Boost | Schubkraft und Aufladung — ob sich das Spammen richtig anfühlt |
| Custom Map | Ob die Plattformen tragen und die Rampen sauber aufsitzen |
| Filterscripts | Torpositionen und -richtungen, Blitzerstandorte |
| Stunts, Anti-Cheat | Schwellenwerte |
| Jobrouten | Ob die Wegpunkte mit den vorgesehenen Fahrzeugen erreichbar sind |
| Animationen | Die Dokumentation weist selbst darauf hin, dass nicht jede gelistete Animation in SA-MP funktioniert |

### Bewusst nicht eingebaut

- **Das MySQL-Plugin-Binary für Linux** — die Abhängigkeitskette führt über
  `log-core` zu `Zeex/cmake-modules`, das nicht erreichbar ist. Für Windows
  lädt `setup.ps1` das fertige Archiv.
- **Ortsanzeige im HUD** hätte die rund 360 Zonengrenzen von San Andreas
  gebraucht. Die stehen in keiner der geprüften Quellen, und geschätzte
  Rechtecke hätten dauerhaft falsche Ortsnamen angezeigt.
