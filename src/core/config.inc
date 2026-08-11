/*
 * Jebiga-Gaming - Zentrale Konfiguration
 *
 * Alle Server-weiten Konstanten, Farben und Dialog-IDs.
 * Wird als erstes Modul eingebunden, damit jedes andere Modul darauf zugreifen kann.
 */

#if defined _jebiga_config_included
    #endinput
#endif
#define _jebiga_config_included

// ---------------------------------------------------------------------------
// Server-Identitaet
// ---------------------------------------------------------------------------
#define SERVER_NAME             "Jebiga-Gaming"
#define SERVER_VERSION          "1.0.0"
#define SERVER_WEBSITE          "jebiga-gaming.net"
#define SERVER_DISCORD          "discord.gg/jebiga"

// ---------------------------------------------------------------------------
// Limits
// ---------------------------------------------------------------------------
#define MAX_ACCOUNT_NAME        (MAX_PLAYER_NAME)
#define MAX_PASSWORD_LEN        (65)    // SHA256 hex = 64 Zeichen + Nullterminator
#define MAX_SALT_LEN            (17)
#define MIN_PASSWORD_LEN        (4)
#define MAX_LOGIN_ATTEMPTS      (3)

#define SPAWN_PROTECTION_TIME   (5000)  // ms Spawnschutz nach dem Spawnen
#define AFK_THRESHOLD           (60)    // Sekunden ohne Update = AFK

// ---------------------------------------------------------------------------
// Admin-Level
// ---------------------------------------------------------------------------
#define ADMIN_NONE              (0)
#define ADMIN_MODERATOR         (1)
#define ADMIN_ADMIN             (2)
#define ADMIN_SENIOR            (3)
#define ADMIN_HEAD              (4)
#define ADMIN_OWNER             (5)

// VIP-Stufen
#define VIP_NONE                (0)
#define VIP_BRONZE              (1)
#define VIP_SILVER              (2)
#define VIP_GOLD                (3)

// ---------------------------------------------------------------------------
// Farben
// ---------------------------------------------------------------------------
#define COL_WHITE               0xFFFFFFFF
#define COL_GREY                0xAFAFAFFF
#define COL_RED                 0xE74C3CFF
#define COL_GREEN               0x2ECC71FF
#define COL_BLUE                0x3498DBFF
#define COL_YELLOW              0xF1C40FFF
#define COL_ORANGE              0xE67E22FF
#define COL_PURPLE              0x9B59B6FF
#define COL_CYAN                0x1ABC9CFF
#define COL_PINK                0xFF79C6FF
#define COL_LIME                0xA4E64BFF

#define COL_INFO                COL_CYAN
#define COL_ERROR               COL_RED
#define COL_SUCCESS             COL_GREEN
#define COL_ADMIN               0xFFA07AFF
#define COL_VIP                 0xFFD700FF
#define COL_ANNOUNCE            0x00D2FFFF

// Embedded-Color-Codes fuer Text innerhalb einer Nachricht
#define EC_WHITE                "{FFFFFF}"
#define EC_GREY                 "{AFAFAF}"
#define EC_RED                  "{E74C3C}"
#define EC_GREEN                "{2ECC71}"
#define EC_BLUE                 "{3498DB}"
#define EC_YELLOW               "{F1C40F}"
#define EC_ORANGE               "{E67E22}"
#define EC_CYAN                 "{1ABC9C}"
#define EC_GOLD                 "{FFD700}"
#define EC_PURPLE               "{9B59B6}"

// ---------------------------------------------------------------------------
// Dateipfade (relativ zu scriptfiles/)
// ---------------------------------------------------------------------------
#define PATH_ACCOUNTS           "accounts/%s.ini"
#define PATH_BANS               "bans.ini"
#define PATH_HOUSES             "houses.ini"
#define PATH_LOG                "logs/server.log"
#define PATH_CREWS              "crews.ini"

// ---------------------------------------------------------------------------
// Seitengroesse fuer lange Auswahllisten
//
// SA-MP begrenzt den Text eines Dialogs auf gut 4000 Zeichen. Listen mit
// hunderten Eintraegen - Skins, Animationen - muessen deshalb blaettern.
// ---------------------------------------------------------------------------
#define DIALOG_PAGE_SIZE        (40)

// ---------------------------------------------------------------------------
// Minispiel-Zustaende
// ---------------------------------------------------------------------------
#define MINIGAME_NONE           (0)
#define MINIGAME_RACE           (1)
#define MINIGAME_DERBY          (2)
#define MINIGAME_DM             (3)
#define MINIGAME_DUEL           (4)

// ---------------------------------------------------------------------------
// Virtuelle Welten
//
// Weltnummern sind serverweit eindeutig, damit sich Innenraeume, Arenen und
// Duelle nicht gegenseitig sehen.
// ---------------------------------------------------------------------------
#define WORLD_MAIN              (0)
#define WORLD_HOUSE_BASE        (100)   // 100 + Hausindex
#define WORLD_DERBY             (500)
#define WORLD_DUEL_BASE         (600)   // 600 + Duell-Slot
#define WORLD_JAIL              (7000)

// ---------------------------------------------------------------------------
// Standorte der Custom Map
//
// Die Bauwerke entstehen zur Laufzeit in map.inc. Die Koordinaten stehen hier,
// weil Teleporte, Stuntzonen und Minispiele sie ebenfalls brauchen.
// ---------------------------------------------------------------------------
#define MAP_PARK_X              (-950.0)
#define MAP_PARK_Y              (2000.0)
#define MAP_PARK_Z              (600.0)     // Oberkante der Plattform
#define MAP_LOUNGE_X            (2000.0)
#define MAP_LOUNGE_Y            (1500.0)
#define MAP_LOUNGE_Z            (500.0)
#define MAP_ISLAND_X            (-1000.0)
#define MAP_ISLAND_Y            (-3000.0)
#define MAP_ISLAND_Z            (10.0)

// ---------------------------------------------------------------------------
// Dialog-IDs
// ---------------------------------------------------------------------------
enum
{
    DIALOG_NONE = 0,
    DIALOG_REGISTER,
    DIALOG_LOGIN,
    DIALOG_TELEPORT_CAT,
    DIALOG_TELEPORT_LIST,
    DIALOG_WEAPON_LIST,
    DIALOG_STATS,
    DIALOG_HELP,
    DIALOG_RULES,
    DIALOG_VIP_MENU,
    DIALOG_ADMIN_MENU,
    DIALOG_HOUSE_BUY,
    DIALOG_HOUSE_MENU,
    DIALOG_RACE_LIST,
    DIALOG_DERBY_LIST,
    DIALOG_STUNT_LIST,
    DIALOG_TOP_LIST,
    DIALOG_DM_LIST,
    DIALOG_TUNING,
    DIALOG_ANIM_LIST,
    DIALOG_CREW_MENU,
    DIALOG_CREW_LIST,
    DIALOG_SKIN_CAT,
    DIALOG_SKIN_LIST,
    DIALOG_TUNE_CAT,
    DIALOG_TUNE_LIST,
    DIALOG_INTERIOR_CAT,
    DIALOG_INTERIOR_LIST,
    DIALOG_ANIM_LIBRARY,
    DIALOG_JOB_LIST,
    DIALOG_BANK,
    DIALOG_BANK_DEPOSIT,
    DIALOG_BANK_WITHDRAW,
    DIALOG_ACHIEVEMENTS,
    DIALOG_WEAPON_INFO
};
