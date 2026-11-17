/* in-stream quick menu (hold L3 + R3 ~2.5 s). Pure UI state - no
 * Orbis calls, no drawing - so the logic is unit-testable on the host
 * (mock/quick_menu_test.c) and the renderer stays a dumb view.
 *
 * The stream loop writes; the video thread reads mutex-protected snapshots.
 */
#pragma once

#include "../input/input_pad.h" /* MENU_BTN_* masks */

typedef enum {
    QM_ACT_NONE = 0,
    QM_ACT_CLOSE,           /* CIRCLE: close; caller applies the bitrate */
    QM_ACT_TOGGLE_TOUCHPAD, /* X on the touchpad row */
    QM_ACT_TOGGLE_OVERLAY,  /* X on the overlay row */
    QM_ACT_QUIT,            /* X on "Quit stream" (game keeps running) */
    QM_ACT_QUIT_APP,        /* X on "Quit app on PC": close the app on the
                             * host (same /cancel as the app-list OPTIONS),
                             * then back to the client menu */
    QM_ACT_SERVER_CMD,      /* X on a PC command: run it (index from
                             * qm_selected_server_cmd()); menu stays open */
} qm_action_t;

/* Apollo server commands listed in the quick menu. */
#define QM_MAX_CMDS 16
#define QM_CMD_LEN  64

/* Bitrate bounds/step mirror the Settings row (SET_BITRATE) so both UIs
 * agree on the legal range. */
#define QM_BITRATE_MIN  5000
#define QM_BITRATE_MAX  50000
#define QM_BITRATE_STEP 5000

/* Row order = display order. */
#define QM_ROW_TOUCHPAD  0
#define QM_ROW_OVERLAY   1
#define QM_ROW_BITRATE   2
#define QM_ROW_COMMANDS  3 /* opens the Apollo PC command list */
#define QM_ROW_QUIT      4
#define QM_ROW_QUIT_APP  5
#define QM_ROW_COUNT     6

typedef struct {
    int open;            /* menu visible */
    int sel;             /* selected row, 0..QM_ROW_COUNT-1 */
    int touchpad;        /* current touchpad-mouse state (display + toggle) */
    int overlay;         /* current perf-overlay state (display + toggle) */
    int bitrate;         /* live value while open, QM_BITRATE_MIN..MAX */
    int bitrate_changed; /* bitrate != the value the session started with */
    int is_apollo;       /* host is Apollo (server commands possible) */
    int cmd_count;       /* commands Apollo published for this client */
    int cmd_mode;        /* the command list is showing instead of rows */
    int cmd_sel;         /* selected command in the list */
    int cmd_ran;         /* last command started, -1 = none yet */
    char cmd_names[QM_MAX_CMDS][QM_CMD_LEN];
} qm_view_t;

/* Commands shown under "PC commands" (copy; call before qm_open). names is
 * a flat array of count strings, item_len bytes apart. */
void qm_set_server_cmds(const char *names, int count, int item_len, int is_apollo);

/* Index of the command the last QM_ACT_SERVER_CMD refers to. */
int qm_selected_server_cmd(void);

/* Open with the session's current values. sel starts on the first row. */
void qm_open(int touchpad, int overlay, int bitrate);

/* Close. bitrate_out/changed_out may be NULL; *changed_out is 1 when the
 * live value differs from what qm_open() was given (caller restarts the
 * stream to apply it). Safe to call when already closed. */
void qm_close(int *bitrate_out, int *changed_out);

int qm_is_open(void);

/* Feed pressed edges (MENU_BTN_* from input_menu_poll). UP/DOWN move the
 * selection (wrap), LEFT/RIGHT and CROSS step the bitrate row down/up,
 * CROSS activates the selected row, CIRCLE requests close. Returns the
 * action for the caller to execute (the state update already happened
 * for toggle rows, so the caller applies the same value to the live
 * config it shows on screen). */
qm_action_t qm_input(unsigned pressed);

/* Snapshot for the renderer (video thread). */
void qm_view(qm_view_t *out);
