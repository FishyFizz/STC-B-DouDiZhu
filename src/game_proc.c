#include "game_proc.h"

XDATA u8 next_usable_playerid = 0;
XDATA u8 my_playerid = INVALID_PLAYERID; //start with an invalid id
XDATA u8 i;
XDATA u8 landlord_playerid;

void game_proc() large reentrant
{
//====================================================
	PHASE_WAIT_PLAYERS:;
	{
		proc_wait_evts(EVT_BTN1_DN | EVT_UART2_RECV);
		if(MY_EVENTS & EVT_BTN1_DN)
		{
			//Don't assign player id again
			if(my_playerid!=INVALID_PLAYERID) goto PHASE_WAIT_PLAYERS;
			
			//Local machine selects player id
			my_playerid = next_usable_playerid;
			led_display_content = 1<<my_playerid;
			next_usable_playerid ++;
			
			//Notify remote machines
			rs485_buf[0] = my_playerid;
			rs485_write(rs485_buf, 1);
		}
		else
		{
			//Remote machine selects player id
			next_usable_playerid = rs485_buf[0] + 1;
		}
	}
	if(next_usable_playerid == 3)//all players ready
		goto PHASE_SEND_17;
	else
		goto PHASE_WAIT_PLAYERS;
		
//====================================================
	PHASE_SEND_17:;
	{
		seg_set_str("READY   ");
		proc_sleep(1000);
		
		if(my_playerid == 0) //server
		{
			proc_sleep(100);//make sure all clients has done preparations and started waiting for card information
			
			srand();
			
			shuffle_send17();
			
			for(i=0;i<60;i++)
				rs485_buf[i] = player_cards[0][i];
			for(i=0;i<3;i++)
				rs485_buf[i+60] = reserved[i];
			
			rs485_write(rs485_buf, 63);
		}
		else //client
		{
			proc_wait_evts(EVT_UART2_RECV);
			for(i=0;i<60;i++)
			  player_cards[0][i] = rs485_buf[i];
			for(i=0;i<3;i++)
				reserved[i] = rs485_buf[i+60];
		}
		
		for(i=0;i<20;i++)
			//fill gameui buffer bank 0 with local player cards
			gameui_cards[0][0][i] = player_cards[my_playerid][i];
		
		//enable gameui and send refresh event
		gameui_enable = 1;
		deliver_event(1, EVT_USER1);
	}
	
//====================================================
	PHASE_CALL:;
	{
		proc_wait_evts(EVT_BTN1_DN | EVT_UART2_RECV);
		if(MY_EVENTS & EVT_BTN1_DN)
		{
			//Local player becomes Landlord
			rs485_buf[0] = my_playerid;
			rs485_write(rs485_buf, 1);
			
			landlord_playerid = my_playerid;
		}
		else
		{
			//Remote player becomed Landlord
			landlord_playerid = rs485_buf[0];
		}
		
		//notify game logic module about dizhu player info, and display
		set_dizhu(landlord_playerid);
		led_display_content += 1<<(landlord_playerid+5);
		
		//update local player cards to gameui display buffer
		for(i=0;i<20;i++)
			gameui_cards[0][0][i] = player_cards[my_playerid][i];
			
		//disable gameui and temporarily display dizhu player.
		gameui_enable = 0;
		deliver_event(1, EVT_USER1);
		
		seg_set_str("DIZHU P ");
		seg_display_content[7] = seg_decoder[landlord_playerid+1];
		
		proc_sleep(1300);
		gameui_enable = 1;
		deliver_event(1, EVT_USER1);
	}
	
//====================================================
	PHASE_PLAY:
	{
		//local player's turn, display a message
		if(current_player == my_playerid)
		{
			gameui_enable = 0;
			deliver_event(1, EVT_USER1);
			
			seg_set_str("YOU PLAY");
			
			proc_sleep(800);
			gameui_enable = 1;
			deliver_event(1, EVT_USER1);
		}
		
		//clear gameui selection
		reset_gameui_select();
	
		//copy local player cards to gameui display buffer bank0
		for(i=0;i<20;i++)
			gameui_cards[0][0][i] = player_cards[my_playerid][i];
		
		//copy last played cards (on table) to gameui display buffer bank1
		for(i=0;i<20;i++)
			gameui_cards[1][0][i] = cards_played[i];
			
		//send gameui update event
		deliver_event(1, EVT_USER1);
		
		proc_wait_evts(EVT_BTN1_DN | EVT_UART2_RECV);
		if(MY_EVENTS & EVT_BTN1_DN)
		{
			//local player tries to play.
			
			//check turn first
			if(current_player != my_playerid) goto PHASE_PLAY;
			
			//copy gameui selection flags to game logic module input
			for(i=0;i<20;i++)
				play_select[i] = gameui_select[0][i];
				
			if(play())
			{
				//pattern valid, notify all machines
				rs485_write(play_select, 20);
			}
			else
			{
				goto PHASE_PLAY;
			}
		}
		else
		{
			//remote machine plays.
			
			//copy received selection flags to game logic module input
			for(i=0;i<20;i++)
				play_select[i] = rs485_buf[i];
			play();
		}
		
		//if played(passed), display message for a short while
		{
			gameui_enable = 0;
			deliver_event(1, EVT_USER1);
			
			if(TYPE(last_player_pattern) == PASS)
				seg_set_str("P   PASS");
			else
				seg_set_str("P  PLAYS");
			seg_display_content[1] = seg_decoder[(current_player+2)%3 + 1];
			
			proc_sleep(800);
			gameui_enable = 1;
			deliver_event(1, EVT_USER1);
		}
		
		//if anyone wins
		if (!(player_cards_count[0] && player_cards_count[1] && player_cards_count[2]))
		{
			// i = win player id
			if(player_cards_count[0] == 0)
				i = 0;
			else if(player_cards_count[1] == 0)
				i = 1;
			else if(player_cards_count[2] == 0)
				i = 2;
			
			if((i != landlord_playerid) && (my_playerid != landlord_playerid))
				{goto PLAYER_WIN;} //win as farmer
			else if(i == my_playerid)
				{goto PLAYER_WIN;} //win as landlord
			else
				{goto PLAYER_LOSE;}
				
			PLAYER_WIN:
			{	
				gameui_enable = 0;
				deliver_event(1, EVT_USER1);
				seg_set_str("YOU WIN ");
				while(1);
			}
			PLAYER_LOSE:
			{
				gameui_enable = 0;
				deliver_event(1, EVT_USER1);
				seg_set_str("YOU LOSE");
				while(1);
			}
		}
	}
	goto PHASE_PLAY;
	
	while(1);
}