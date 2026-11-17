#include "../src/input/guide_chord.h"
#include <assert.h>
#include <stdio.h>
int main(void){
    const int chord=LB_FLAG|RB_FLAG|PLAY_FLAG;
    guide_chord_t g={0};uint64_t t=0;
    assert(guide_chord_filter(&g,A_FLAG,t)==A_FLAG);
    /* Press: one neutral frame, then Guide. */
    assert(guide_chord_filter(&g,chord,t)==0);
    assert(guide_chord_filter(&g,chord,t+=4000)==SPECIAL_FLAG);
    /* Long press: Guide stays down for as long as the chord is held. */
    for(int i=0;i<500;i++)assert(guide_chord_filter(&g,chord,t+=4000)==SPECIAL_FLAG);
    /* Release one button: Guide goes up, the rest of the chord is swallowed. */
    assert(guide_chord_filter(&g,LB_FLAG|RB_FLAG,t+=4000)==0);
    assert(guide_chord_filter(&g,LB_FLAG,t+=4000)==0);
    assert(guide_chord_filter(&g,0,t+=4000)==0);
    assert(guide_chord_filter(&g,A_FLAG,t+=4000)==A_FLAG);
    /* Quick tap: chord released right away, Guide still lasts >= 80 ms. */
    assert(guide_chord_filter(&g,chord,t+=4000)==0);
    uint64_t start=t+=4000;assert(guide_chord_filter(&g,chord,start)==SPECIAL_FLAG);
    assert(guide_chord_filter(&g,0,start+40000)==SPECIAL_FLAG);
    assert(guide_chord_filter(&g,0,start+81000)==0);
    assert(guide_chord_filter(&g,0,start+85000)==0);
    assert(guide_chord_filter(&g,B_FLAG,start+90000)==B_FLAG);
    puts("Guide chord: long press holds Guide, tap lasts 80 ms, chord never leaks passed");
}
