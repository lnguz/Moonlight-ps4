#pragma once

/* System UI state of this app (sceSystemServiceGetStatus).
 * Returns 1 when the PS4 home screen / system UI is in front of Moonlight
 * (PS button), 0 when Moonlight is in the foreground, -1 when unknown
 * (call failed or the reply did not look like the expected layout). */
int sys_status_backgrounded(void);
