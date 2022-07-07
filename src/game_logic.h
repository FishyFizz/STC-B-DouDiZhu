#ifndef _GAME_LOGIC_H_
#define _GAME_LOGIC_H_

#include "global.h"
#include "random.h"

#define _3 0
#define _4 1
#define _5 2
#define _6 3
#define _7 4
#define _8 5
#define _9 6
#define _10 7
#define _J 8
#define _Q 9
#define _K 10
#define _A 11
#define _2 12
#define _Jo 13 // black joker
#define _JO 14 // red joker
#define MAXCARD 15
#define NOCARD 15

#define SINGLE 0
#define PAIR 1
#define TRIPLE 2
#define BOMB 3
#define TRIPLE_A 4
#define TRIPLE_AA 5
#define SER_SINGLE 6
#define SER_PAIR 7
#define PLANE 8
#define PLANE_1 9
#define PLANE_2 10
#define BOMB_AB 11
#define BOMB_AA 12
#define BOMB_AABB 13
#define BOMB_BOMB 14
#define ROCKET 15
#define PASS 16
#define INVALID 17
#define LASTPLAYER ((current_player+2) %3)
#define NEXTPLAYER ((current_player+1) %3) 

#define PATTERN(type, max, length) ((((u32)type<<16) + ((u32)max<<8) + (u32)length))
#define TYPE(pattern) 		((u8)(pattern>>16))
#define MAX(pattern) 		((u8)((pattern & 0x0000ff00)>>8))
#define LENGTH(pattern) 	((u8)(pattern & 0x000000ff))
#define INVALID_PTRN 			PATTERN(INVALID,0,0)


extern XDATA u8 player_cards[3][20];
extern XDATA u8 reserved[4];
extern XDATA u8 player_cards_count[3];
extern XDATA u8 current_player;
extern XDATA u8 last_played_player;
extern XDATA u32 last_played_pattern;
extern XDATA u32 last_player_pattern;

void shuffle_send17();
void bubble_sort(u8 XDATA *arr, u8 len);
void set_dizhu(u8 p);

extern XDATA u8 play_select[20]; //external input
extern XDATA u8 card_playcount[15];
extern XDATA u8 play_binsort[5];
u32 check_pattern();
u8 play_legal_check(u32 pattern);

extern XDATA u8 cards_played[20];
u8 play();

#endif