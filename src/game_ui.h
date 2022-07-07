#ifndef _GAME_UI_H_
#define _GAME_UI_H_

#include "global.h"
#include "events.h"
#include "seg_led.h"
#include "game_logic.h"

extern XDATA u8 gameui_cards[2][3][8];
extern XDATA u8 gameui_select[3][8];
extern XDATA u8 current_bank, current_page, cursor_pos, cursor_blink;
extern XDATA u8 gameui_enable;

void reset_gameui_disp();
void reset_gameui_select();
u8 is_empty_page();
u8 cursor_invalid();
void gameui_proc();

#endif