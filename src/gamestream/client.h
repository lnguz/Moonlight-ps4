// GameStream/Sunshine API client: serverinfo, four-phase pairing,
// applist and launch/resume/cancel. mbedTLS port of libgamestream from
// moonlight-embedded (GPLv3).
#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <Limelight.h>

#include "certgen.h"
#include "mini_xml.h"

#define GS_UNIQUEID "0123456789ABCDEF"

/* Apollo (Sunshine fork) extensions published in HTTPS /serverinfo. */
#define GS_MAX_SERVER_CMDS 16
#define GS_SERVER_CMD_LEN  64
#define GS_DEVICE_NAME_LEN 32

typedef struct {
    SERVER_INFORMATION serverInfo;

    char address[128];
    unsigned short httpPort;
    unsigned short httpsPort;

    bool paired;
    int currentGame;
    int serverMajorVersion;
    int serverCodecModeSupport;
    bool isNvidiaSoftware;

    char appVersion[32];
    char gfeVersion[32];
    char serverCertPem[8192]; // server certificate saved during pairing
    char rtspSessionUrl[256];

    /* the streaming PC's MAC address as reported by Sunshine in
     * /serverinfo over HTTPS for PAIRED clients (upstream Sunshine fills
     * root.mac with the interface the request arrived on; plain-HTTP and
     * unpaired requests get the 00:00:00:00:00:00 placeholder, which is
     * never stored here). Used to auto-fill the Wake-on-LAN setting so
     * nobody types 12 hex digits by hand. Empty = host did not report. */
    char host_mac[24]; // "AA:BB:CC:DD:EE:FF"

    char hostUuid[64];      // <uniqueid>: stable PC identity (IP may change)
    char hostName[64];      // <hostname>

    /* Apollo: present only in the paired HTTPS serverinfo. */
    bool isApollo;          // host sent <Permission> (Apollo-only tag)
    bool vdisplayCapable;   // <VirtualDisplayCapable>true
    bool vdisplayReady;     // <VirtualDisplayDriverReady>true
    unsigned permission;    // Apollo PERM bitmask (0 when not Apollo)
    int serverCmdCount;     // entries in serverCmds (needs server_cmd perm)
    char serverCmds[GS_MAX_SERVER_CMDS][GS_SERVER_CMD_LEN];

    client_identity_t *id;
} gs_server_t;

/* Name this console reports when pairing (Apollo shows it in its client
 * list and keys permissions/virtual-display memory on the pairing).
 * Letters, digits, '-' and '_' only; anything else becomes '-'. Empty
 * restores the default "PS4". */
void gs_set_device_name(const char *name);
const char *gs_get_device_name(void);

/* Apollo launch options appended to /launch and /resume. virtual_display
 * asks Apollo to create a virtual monitor matching the stream mode;
 * scale_percent is the Windows scaling for it (100 = no scaling). Plain
 * Sunshine ignores both parameters. */
void gs_set_launch_options(bool virtual_display, int scale_percent);

/* Validate a <mac> XML field value and normalize it into out
 * ("AA:BB:CC:DD:EE:FF"). Returns 0 and fills out only for a real,
 * non-placeholder, non-broadcast MAC (Sunshine sends 00:00:00:00:00:00
 * on plain HTTP; GFE-era servers used nonsense MACs like 0123456789AB
 * that would wake nothing). Exported for the host unit tests. */
int gs_mac_from_field(const char *raw, char *out, size_t cap);

// Initialize identity (generating certificate if needed), query
// serverinfo and fill server state.
int gs_init(gs_server_t *server, client_identity_t *id, const char *address,
            unsigned short httpPort, const char *keydir);

// Four-phase pairing with 4-digit PIN.
int gs_pair(gs_server_t *server, const char *pin);
/* Same, with an explicit per-request timeout (ms) for every pairing
 * phase — the default PIN flow uses 120 s (see client.c). */
int gs_pair_ex(gs_server_t *server, const char *pin, unsigned timeout_ms);
int gs_unpair(gs_server_t *server);

int gs_applist(gs_server_t *server, app_entry_t **list);
int gs_appasset(gs_server_t *server, int app_id, unsigned char **bytes, size_t *length);

/* Race-free variants for background threads: operate on explicitly captured
 * host/port/identity instead of a gs_server_t that may be re-initialized
 * concurrently by the main thread. */
int gs_applist_ex(client_identity_t *id, const char *address,
                  unsigned short https_port, app_entry_t **list);
/* host_mac_out/cap (pass NULL to skip) receives the PC's MAC when
 * the paired HTTPS serverinfo reports one ("" when not). */
int gs_serverinfo_status(client_identity_t *id, const char *address,
                         unsigned short https_port, unsigned short http_port,
                         bool *paired, int *current_game,
                         char *host_mac_out, size_t host_mac_cap);

// Launch or resume an app and leave the server ready for LiStartConnection.
// Fills config->remoteInputAesKey/Iv and server->rtspSessionUrl.
int gs_start_app(gs_server_t *server, STREAM_CONFIGURATION *config, int appId,
                 bool sops, bool localaudio, int gamepad_mask);

int gs_quit_app(gs_server_t *server);

/* Refresh serverinfo (currentGame, paired, etc.) without re-init. */
int gs_refresh_status(gs_server_t *server);
