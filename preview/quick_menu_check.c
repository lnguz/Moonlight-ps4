#include "../src/ui/quick_menu.h"
#include <assert.h>
#include <stdio.h>
int main(void){
    qm_open(1,0,20000);assert(qm_is_open());
    assert(qm_input(MENU_BTN_CROSS)==QM_ACT_TOGGLE_TOUCHPAD);
    qm_view_t view;qm_view(&view);assert(!view.touchpad);
    qm_input(MENU_BTN_DOWN);assert(qm_input(MENU_BTN_CROSS)==QM_ACT_TOGGLE_OVERLAY);
    qm_input(MENU_BTN_DOWN);qm_input(MENU_BTN_RIGHT);int br,changed;
    assert(qm_input(MENU_BTN_CIRCLE)==QM_ACT_CLOSE);qm_close(&br,&changed);assert(br==25000 && changed);
    assert(!qm_is_open());puts("Stream menu: toggles, bitrate and close passed");

    /* PC commands: none -> row does nothing; three -> list, wrap, run, back. */
    qm_set_server_cmds(NULL,0,0,1);qm_open(0,0,20000);
    for(int i=0;i<QM_ROW_COMMANDS;i++)qm_input(MENU_BTN_DOWN);
    assert(qm_input(MENU_BTN_CROSS)==QM_ACT_NONE);qm_view(&view);assert(!view.cmd_mode && view.sel==QM_ROW_COMMANDS);
    qm_close(NULL,NULL);
    char names[3][64]={"Sleep PC","Restart Steam","A long command name to test truncation on screen"};
    qm_set_server_cmds(&names[0][0],3,64,1);qm_open(0,0,20000);
    for(int i=0;i<QM_ROW_COMMANDS;i++)qm_input(MENU_BTN_DOWN);
    assert(qm_input(MENU_BTN_CROSS)==QM_ACT_NONE);qm_view(&view);assert(view.cmd_mode && view.cmd_count==3);
    qm_input(MENU_BTN_UP);qm_view(&view);assert(view.cmd_sel==2);
    assert(qm_input(MENU_BTN_CROSS)==QM_ACT_SERVER_CMD && qm_selected_server_cmd()==2);
    assert(qm_is_open());assert(qm_input(MENU_BTN_CIRCLE)==QM_ACT_NONE);qm_view(&view);assert(!view.cmd_mode);
    assert(qm_input(MENU_BTN_CIRCLE)==QM_ACT_CLOSE);qm_close(NULL,NULL);
    puts("Stream menu: PC commands passed");
}
