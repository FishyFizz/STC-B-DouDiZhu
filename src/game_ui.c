#include "game_ui.h"

XDATA u8 gameui_cards[2][3][8];
XDATA u8 gameui_select[3][8];
XDATA u8 current_bank = 0, current_page = 0, cursor_pos = 0, cursor_blink = 0;
XDATA u8 gameui_enable = 0;

CODE u8 cards_seg[16] = {
	0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f, //3~9
	0x5c, 0x0e, 0x67, 0x75, 0x77, 0x5b, //10~2
	0x40, 0x01 //Jo/ JO
}; 


void reset_gameui_disp()
{
	XDATA u8 i;
	for(i = 0;i<48;i++)
		gameui_cards[0][0][i] = 15; //NOCARD -> 15
	
	for(i=0;i<24;i++)
		gameui_select[0][i] = 0;
		
	current_bank = current_page = cursor_pos = 0;
}

void reset_gameui_select()
{
	XDATA u8 i;
	for(i=0;i<24;i++)
		gameui_select[0][i] = 0;
}

u8 is_empty_page()
{
	XDATA u8 i;
	for(i=0; i<8; i++)
		if(gameui_cards[current_bank][current_page][i] != 15)
			return 0;
	return 1;
}

u8 cursor_invalid()
{
	return (gameui_cards[current_bank][current_page][cursor_pos]==15);
}

void update_gameui()
{
	XDATA u8 i;
	
	if(!gameui_enable) return;
	
	//display content changed, update.
	if(is_empty_page())
		current_page = 0;
	if(cursor_invalid())
		cursor_pos = 0;
	for(i=0;i<8;i++)
		seg_display_content[i] = cards_seg[gameui_cards[current_bank][current_page][i]] + (gameui_select[current_page][i]?0x80:0);
	cursor_blink?(seg_display_content[cursor_pos] = 0):0;
}

void gameui_proc()
{
	reset_gameui_disp();
		
	//////////////////////
	
	while(1)
	{
		proc_wait_evts_timeout(
			EVT_NAV_U | EVT_NAV_D |
			EVT_NAV_L | EVT_NAV_R |
			EVT_NAV_PUSH| EVT_BTN3_DN |
			EVT_BTN2_DN | EVT_USER1,
			250
		);

		switch(MY_EVENTS)
		{
			case EVT_NAV_U:
				(current_page < 2)?(current_page++):0;
				break;
			case EVT_NAV_D:
				(current_page > 0)?(current_page--):0;
				break;
			case EVT_NAV_L:
				cursor_pos = (cursor_pos + 7)%8;
				break;
			case EVT_NAV_R:
				cursor_pos = (cursor_pos + 1)%8;
				break;
			case EVT_NAV_PUSH:
				gameui_select[current_page][cursor_pos] = ~gameui_select[current_page][cursor_pos];
				break;
			case EVT_TIMER:
				cursor_blink = ~cursor_blink;
				break;
			case EVT_BTN2_DN:
				current_bank = 0; reset_gameui_select();
				break;
			case EVT_BTN3_DN:
				current_bank = 1; reset_gameui_select();
				break;
			case EVT_USER1:
				break;
		}
		
		update_gameui();

	}
}