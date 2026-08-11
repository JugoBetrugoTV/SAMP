/*
 * UIF Freeroam - Zentrale Konfiguration
 *
 * Alle Server-weiten Konstanten, Farben und Dialog-IDs.
 * Wird als erstes Modul eingebunden, damit jedes andere Modul darauf zugreifen kann.
 */

#if defined _uif_config_included
    #endinput
#endif
#define _uif_config_included

// ---------------------------------------------------------------------------
// Server-Identitaet
// ---------------------------------------------------------------------------
#define SERVER_NAME             "UIF - United Islands Freeroam"
#define SERVER_VERSION          "1.0.0"
#define SERVER_WEBSITE          "uifserver.net"
#define SERVER_DISCORD          "discord.gg/uif"

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

// ---------------------------------------------------------------------------
// Dateipfade (relativ zu scriptfiles/)
// ---------------------------------------------------------------------------
#define PATH_ACCOUNTS           "accounts/%s.ini"
#define PATH_BANS               "bans.ini"
#define PATH_HOUSES             "houses.ini"
#define PATH_LOG                "logs/server.log"

// ---------------------------------------------------------------------------
// Minispiel-Zustaende
// ---------------------------------------------------------------------------
#define MINIGAME_NONE           (0)
#define MINIGAME_RACE           (1)
#define MINIGAME_DERBY          (2)

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
    DIALOG_TOP_LIST
};
