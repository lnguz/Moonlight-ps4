/* Deliberately does not include <orbis/SystemService.h>: OpenOrbis declares
 * sceSystemServiceGetStatus() without parameters. Layout from shadPS4
 * (OrbisSystemServiceStatus). */
#include "sys_status.h"
#include "../log.h"

#include <stdint.h>
#include <string.h>

typedef struct {
    int32_t event_num;
    uint8_t is_system_ui_overlaid;
    uint8_t is_in_background_execution;
    uint8_t is_cpu_mode7_cpu_normal;
    uint8_t is_game_live_streaming_on_air;
    uint8_t is_out_of_vr_play_area;
    uint8_t reserved[125];
} ml_sys_status_t;

int sceSystemServiceGetStatus(ml_sys_status_t *status);

int sys_status_backgrounded(void) {
    static int s_last = -2;
    static int s_bad_logged;
    ml_sys_status_t st;
    memset(&st, 0, sizeof(st));
    int rc = sceSystemServiceGetStatus(&st);
    int v;
    if (rc != 0) {
        v = -1;
    } else if (st.is_system_ui_overlaid > 1 || st.is_in_background_execution > 1) {
        /* Not booleans: the layout guess is wrong, never act on it. */
        v = -1;
        if (!s_bad_logged++)
            LOGW("sys_status: unexpected reply %02x %02x %02x %02x; ignoring",
                 st.is_system_ui_overlaid, st.is_in_background_execution,
                 st.is_cpu_mode7_cpu_normal, st.is_game_live_streaming_on_air);
    } else {
        v = (st.is_system_ui_overlaid || st.is_in_background_execution) ? 1 : 0;
    }
    if (v != s_last) {
        LOGI("sys_status: rc=0x%08x overlaid=%u background=%u -> %d",
             (unsigned)rc, st.is_system_ui_overlaid,
             st.is_in_background_execution, v);
        s_last = v;
    }
    return v;
}
