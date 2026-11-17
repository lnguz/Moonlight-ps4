/* in-stream quick menu - portable state (see quick_menu.h). */
#include "quick_menu.h"
#include <pthread.h>
#include <stdio.h>
#include <string.h>
static pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER;

static qm_view_t s_qm;
static int s_seed_br; /* bitrate the session started with (change test) */

static int qm_clamp_br(int b) {
    if (b < QM_BITRATE_MIN)
        return QM_BITRATE_MIN;
    if (b > QM_BITRATE_MAX)
        return QM_BITRATE_MAX;
    return b;
}

static void qm_set_server_cmds_locked(const char *names, int count,
                                     int item_len, int is_apollo) {
    if (count < 0 || !names || item_len <= 0)
        count = 0;
    if (count > QM_MAX_CMDS)
        count = QM_MAX_CMDS;
    memset(s_qm.cmd_names, 0, sizeof(s_qm.cmd_names));
    for (int i = 0; i < count; i++)
        snprintf(s_qm.cmd_names[i], QM_CMD_LEN, "%.*s", item_len,
                 names + (size_t)i * (size_t)item_len);
    s_qm.cmd_count = count;
    s_qm.is_apollo = is_apollo ? 1 : 0;
}

static void qm_open_locked(int touchpad, int overlay, int bitrate) {
    s_qm.open = 1;
    s_qm.sel = 0;
    s_qm.cmd_mode = 0;
    s_qm.cmd_sel = 0;
    s_qm.cmd_ran = -1;
    s_qm.touchpad = touchpad ? 1 : 0;
    s_qm.overlay = overlay ? 1 : 0;
    s_qm.bitrate = qm_clamp_br(bitrate);
    s_seed_br = s_qm.bitrate;
    s_qm.bitrate_changed = 0;
}

static void qm_close_locked(int *bitrate_out, int *changed_out) {
    if (bitrate_out)
        *bitrate_out = s_qm.bitrate;
    if (changed_out)
        *changed_out = s_qm.open ? (s_qm.bitrate != s_seed_br) : 0;
    s_qm.open = 0;
}

static int qm_is_open_locked(void) {
    return s_qm.open;
}

static qm_action_t qm_input_locked(unsigned pressed) {
    if (!s_qm.open || !pressed)
        return QM_ACT_NONE;

    /* PC command list: UP/DOWN pick, CROSS runs, CIRCLE goes back. */
    if (s_qm.cmd_mode) {
        if (s_qm.cmd_count > 0) {
            if (pressed & MENU_BTN_UP)
                s_qm.cmd_sel = (s_qm.cmd_sel + s_qm.cmd_count - 1) % s_qm.cmd_count;
            if (pressed & MENU_BTN_DOWN)
                s_qm.cmd_sel = (s_qm.cmd_sel + 1) % s_qm.cmd_count;
        }
        if (pressed & MENU_BTN_CIRCLE) {
            s_qm.cmd_mode = 0;
            return QM_ACT_NONE;
        }
        if ((pressed & MENU_BTN_CROSS) && s_qm.cmd_count > 0) {
            s_qm.cmd_ran = s_qm.cmd_sel;
            return QM_ACT_SERVER_CMD;
        }
        return QM_ACT_NONE;
    }

    /* Navigation first: UP/DOWN always move, even on the bitrate row. */
    if (pressed & MENU_BTN_UP)
        s_qm.sel = (s_qm.sel + QM_ROW_COUNT - 1) % QM_ROW_COUNT;
    if (pressed & MENU_BTN_DOWN)
        s_qm.sel = (s_qm.sel + 1) % QM_ROW_COUNT;

    /* Bitrate row: LEFT/RIGHT step the value; CROSS also steps up so the
     * row is usable one-handed. Clamped to the Settings bounds; whether
     * the value actually moved is decided at close time (vs. the seed),
     * so stepping up and back down does not force a reconnect. */
    if (s_qm.sel == QM_ROW_BITRATE) {
        if (pressed & MENU_BTN_LEFT)
            s_qm.bitrate = qm_clamp_br(s_qm.bitrate - QM_BITRATE_STEP);
        if (pressed & MENU_BTN_RIGHT)
            s_qm.bitrate = qm_clamp_br(s_qm.bitrate + QM_BITRATE_STEP);
        if (pressed & MENU_BTN_CROSS)
            s_qm.bitrate = qm_clamp_br(s_qm.bitrate + QM_BITRATE_STEP);
    }

    if (pressed & MENU_BTN_CIRCLE)
        return QM_ACT_CLOSE;

    if (pressed & MENU_BTN_CROSS) {
        switch (s_qm.sel) {
        case QM_ROW_TOUCHPAD:
            s_qm.touchpad = !s_qm.touchpad;
            return QM_ACT_TOGGLE_TOUCHPAD;
        case QM_ROW_OVERLAY:
            s_qm.overlay = !s_qm.overlay;
            return QM_ACT_TOGGLE_OVERLAY;
        case QM_ROW_COMMANDS:
            if (s_qm.cmd_count > 0) {
                s_qm.cmd_mode = 1;
                if (s_qm.cmd_sel >= s_qm.cmd_count)
                    s_qm.cmd_sel = 0;
            }
            return QM_ACT_NONE;
        case QM_ROW_QUIT:
            return QM_ACT_QUIT;
        case QM_ROW_QUIT_APP:
            return QM_ACT_QUIT_APP;
        default:
            break; /* bitrate row handled above */
        }
    }

    return QM_ACT_NONE;
}

static void qm_view_locked(qm_view_t *out) {
    *out = s_qm;
    out->bitrate_changed = (s_qm.bitrate != s_seed_br);
}

void qm_open(int touchpad,int overlay,int bitrate){pthread_mutex_lock(&mutex);qm_open_locked(touchpad,overlay,bitrate);pthread_mutex_unlock(&mutex);}

void qm_close(int *bitrate_out,int *changed_out){pthread_mutex_lock(&mutex);qm_close_locked(bitrate_out,changed_out);pthread_mutex_unlock(&mutex);}

int qm_is_open(void){pthread_mutex_lock(&mutex);int result=qm_is_open_locked();pthread_mutex_unlock(&mutex);return result;}

qm_action_t qm_input(unsigned pressed){pthread_mutex_lock(&mutex);qm_action_t result=qm_input_locked(pressed);pthread_mutex_unlock(&mutex);return result;}

void qm_view(qm_view_t *out){pthread_mutex_lock(&mutex);qm_view_locked(out);pthread_mutex_unlock(&mutex);}

void qm_set_server_cmds(const char *names,int count,int item_len,int is_apollo){pthread_mutex_lock(&mutex);qm_set_server_cmds_locked(names,count,item_len,is_apollo);pthread_mutex_unlock(&mutex);}

int qm_selected_server_cmd(void){pthread_mutex_lock(&mutex);int result=s_qm.cmd_ran;pthread_mutex_unlock(&mutex);return result;}
