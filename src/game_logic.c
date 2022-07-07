#include "game_logic.h"

XDATA u8 player_cards[3][20];
XDATA u8 reserved[4];
XDATA u8 player_cards_count[3];
XDATA u8 current_player;
XDATA u8 last_played_player;
XDATA u32 last_played_pattern;
XDATA u32 last_player_pattern;

void shuffle_send17()
{
	u8 i;
	u8 tmprnd, tmp;
	//clear cards.
	for (i = 0; i < 60; i++)
		player_cards[0][i] = NOCARD;

	//put the 4 identical cards into deck.
	for (i = _3; i <= _2; i++)
	{
		player_cards[0][i * 4] = i;
		player_cards[0][i * 4 + 1] = i;
		player_cards[0][i * 4 + 2] = i;
		player_cards[0][i * 4 + 3] = i;
	}

	//put Jokers at the 53th and  54th position
	player_cards[0][52] = _Jo;
	player_cards[0][53] = _JO;

	//Now we have a sorted deck.

	//Start shuffling
	i = 53;
	while (i > 0)
	{
		tmprnd = rand32() % (i + 1);
		tmp = player_cards[0][tmprnd];
		player_cards[0][tmprnd] = player_cards[0][i];
		player_cards[0][i] = tmp;
		i--;
	}
	//i = 0
	tmprnd = rand32() % (i + 1);
	tmp = player_cards[0][tmprnd];
	player_cards[0][tmprnd] = player_cards[0][i];
	player_cards[0][i] = tmp;

	//Now we reorder the array to 'send' cards to players.
	/*
		after shuffling, the position is
		P1: 20
		P2: 20
		P3: 14

		we take the last 3 cards from p1, move to p3.
		and take the last 3 cards from p2, reserve.
	*/

	player_cards[2][14] = player_cards[0][17];
	player_cards[2][15] = player_cards[0][18];
	player_cards[2][16] = player_cards[0][19];
	player_cards[0][17] = player_cards[0][18] = player_cards[0][19] = NOCARD;

	reserved[0] = player_cards[1][17];
	reserved[1] = player_cards[1][18];
	reserved[2] = player_cards[1][19];
	reserved[3] = NOCARD;

	player_cards[1][17] = player_cards[1][18] = player_cards[1][19] = NOCARD;

	player_cards_count[0] = player_cards_count[1] = player_cards_count[2] = 17;

	//sort cards
	//since NOCARD is bigger in value, we can directly sort with them
	//without considering the actual cards that player holds
	bubble_sort(player_cards[0], 20);
	bubble_sort(player_cards[1], 20);
	bubble_sort(player_cards[2], 20);
	bubble_sort(reserved, 3);
		
	return;
}

void bubble_sort(u8 XDATA *arr, u8 len)
{
	u8 i, j, temp;
	for (i = 0; i < len - 1; i++)
		for (j = 0; j < len - 1 - i; j++)
			if (arr[j] > arr[j + 1])
			{
				temp = arr[j];
				arr[j] = arr[j + 1];
				arr[j + 1] = temp;
			}

	return;
}

void set_dizhu(u8 p)
{
	XDATA u8 i;
	player_cards[p][17] = reserved[0];
	player_cards[p][18] = reserved[1];
	player_cards[p][19] = reserved[2];

	player_cards_count[0] = player_cards_count[1] = player_cards_count[2] = 17;
	player_cards_count[p] = 20;

	//since NOCARD is bigger in value, we can directly sort with them
	//without considering the actual cards that player holds
	bubble_sort(player_cards[0], 20);
	bubble_sort(player_cards[1], 20);
	bubble_sort(player_cards[2], 20);

	current_player = p;
	last_played_player = p;
	last_played_pattern = INVALID_PTRN;
	
	//clear cards_played array
	for(i=0;i<20;i++)
		cards_played[i] = NOCARD;
	return;
}


XDATA u8 play_select[20]; //external input
XDATA u8 card_playcount[15];
XDATA u8 play_binsort[5];
u32 check_pattern()
{
	XDATA char i, total, biggest_card, biggest_triple, smallest_card;

	//variable used by plane match
	XDATA u8 biggest_exact_triple = NOCARD;
	XDATA u8 smallest_exact_triple = NOCARD;
	XDATA u8 current_triple_start;
	XDATA u8 current_triple_length;
	XDATA u8 longest_triple_start;
	XDATA u8 longest_triple_length;
	XDATA u8 longest_triple_end;

	total = 0;
	//Reset helper arrays
	for (i = 0; i < 15; i++)
		card_playcount[i] = 0;
	for (i = 0; i < 5; i++)
		play_binsort[i] = 0;

	//Store the counts of each number card being selectd, and find the biggest card
	//For example, if player select 2 * _J to play, then card_playcount[_J] = 2
	for (i = 0; i < 20; i++)
	{
		if (play_select[i])
		{
			biggest_card = player_cards[current_player][i];
			card_playcount[player_cards[current_player][i]]++;
			total++;
		}
	}

	//find the smallest card selected
	for (i = _3; i <= _JO; i++)
	{
		if (card_playcount[i])
		{
			smallest_card = i;
			break;
		}
	}

	//Store the identical cards pattern
	//For example, if player select 2*_J 3*_K and 1*_A 1*_2
	//play_binsort[1] = 2 ---> (1*_A, 1*_2)
	//play_binsort[2] = 1 ---> (2*_J)
	//play_binsort[3] = 1 ---> (3*_K)
	for (i = 0; i < 14; i++)
		play_binsort[card_playcount[i]]++;

	//Match Pass
	if (!total)
		return PATTERN(PASS, 0, 0);

	//Match Single
	if (total == 1)
		return PATTERN(SINGLE, biggest_card, 1);

	//Match Pair (and joker pair)
	if (total == 2)
	{
		if (play_binsort[2] == 1)
			return PATTERN(PAIR, biggest_card, 2);
		if (card_playcount[_Jo] && card_playcount[_JO])
			return PATTERN(ROCKET, _JO, 2);
		return INVALID_PTRN;
	}

	//Match Triple
	if (total == 3)
	{
		if (play_binsort[3] == 1)
			return PATTERN(TRIPLE, biggest_card, 3);
		return INVALID_PTRN;
	}

	//Helper function, finds the biggest triple (including bomb)
	biggest_triple = NOCARD;
	for (i = _2; i >= _3; i--)
	{
		if (card_playcount[i] >= 3)
		{
			biggest_triple = i;
			break;
		}
	}

	//Match Bomb and 3+A
	if (total == 4)
	{
		if (play_binsort[4] == 1)
			return PATTERN(BOMB, biggest_card, 4);
		if (play_binsort[3] == 1)
			return PATTERN(TRIPLE_A, biggest_triple, 4);
		return INVALID_PTRN;
	}

	//total >= 5 check single series first
	if (total >= 5)
	{
		if (total == play_binsort[1]) //every cards are single
		{
			//check if range exeeds
			if (biggest_card >= _2)
				return INVALID_PTRN;

			//check if every card are played in sequence
			for (i = smallest_card; i < biggest_card; i++)
				if (!card_playcount[i]) //cards not continuous
					return INVALID_PTRN;

			return PATTERN(SER_SINGLE, biggest_card, total);
		}
		//not series, continue to check other possibilities
	}

	// 3+2
	if (total == 5)
	{
		if (play_binsort[3] == play_binsort[2] && play_binsort[2] == 1)
			return PATTERN(TRIPLE_AA, biggest_triple, 5);
		return INVALID_PTRN;
	}

	//total >= 6 check pair series first
	if (total >= 6)
	{
		if (total == play_binsort[2] * 2) //every cards are paired
		{
			//check if range exeeds
			if (biggest_card >= _2)
				return INVALID_PTRN;

			//check if every card are played in sequence
			for (i = smallest_card; i <= biggest_card; i++)
				if (!card_playcount[i]) //cards not continuous
					return INVALID_PTRN;

			return PATTERN(SER_PAIR, biggest_card, total);
		}
		//not series, continue to check other possibilities
	}

	//AAABBB or AAAABB or AAAABC
	if (total == 6)
	{
		if (play_binsort[4] == 1 && play_binsort[2] == 1)
			return PATTERN(BOMB_AA, biggest_triple, 6);
		if (play_binsort[4] == 1 && play_binsort[1] == 2)
			return PATTERN(BOMB_AB, biggest_triple, 6);
		if (play_binsort[3] == 2)
			return PATTERN(PLANE, biggest_triple, 6);
		return INVALID_PTRN;
	}

	//total == 7, only can be single series (checked before)
	if (total == 7)
		return INVALID_PTRN;

	//total == 8, AAAABBBB or AAAABBCC or plane((3+1)*2)
	if (total == 8)
	{
		if (play_binsort[4] == 2)
			return PATTERN(BOMB_BOMB, biggest_triple, 8);
		if (play_binsort[4] == 1 && play_binsort[2] == 2)
			return PATTERN(BOMB_AABB, biggest_triple, 8);
		//still can be plane. so do further check, not return
	}

	//plane check:

	//plane(AAA*n)
	if (total == 3 * play_binsort[3] && total >= 6)
	{
		//check if range exeeds
		if (biggest_card >= _2)
			goto NOT_3N_PLANE;

		//check if every card are played in sequence
		for (i = smallest_card; i < biggest_card; i++)
			if (!card_playcount[i]) //cards not continuous
				goto NOT_3N_PLANE;

		return PATTERN(PLANE, biggest_triple, total);
	}
NOT_3N_PLANE:;

	//plane(AAABB*n)
	if ((play_binsort[3] == play_binsort[2] + play_binsort[4] * 2) &&
		play_binsort[1] == 0 &&
		total >= 10)
	{
		//find the smallest exact triple

		for (i = _3; i <= _JO; i++)
		{
			if (card_playcount[i] == 3)
			{
				smallest_exact_triple = i;
				break;
			}
		}

		//find the biggest exact triple

		for (i = _JO; i >= _3; i--)
		{
			if (card_playcount[i] == 3)
			{
				biggest_exact_triple = i;
				break;
			}
		}

		//check if exeeded limit
		if (biggest_exact_triple >= _2)
			return INVALID_PTRN;

		//check if all cards between them are triples
		for (i = smallest_exact_triple; i <= biggest_exact_triple; i++)
		{
			if (card_playcount[i] != 3)
				return INVALID_PTRN;
		}

		//success
		return PATTERN(PLANE_2, biggest_exact_triple, total);
	}

	//1. Identify longest triple series
	current_triple_start = current_triple_length = 0;
	longest_triple_start = longest_triple_length = longest_triple_end = 0;

	for (i = _3; i <= _2; i++)
	{
		if (card_playcount[i] >= 3)
		{
			if (!current_triple_length)
				current_triple_start = i;
			current_triple_length++;
			if (current_triple_length >= longest_triple_length)
			{
				longest_triple_length = current_triple_length;
				longest_triple_start = current_triple_start;
				longest_triple_end = i;
			}
		}
		else
			current_triple_length = 0;
	}

	// plane (AAAB*n)
	// single = total - triples*3 
	// triples - x = single + 3x
	// triples - single = 4x [must be satisfied]
	// triples - x >= 2 [must be satisfied]
#define singles (total-longest_triple_length*3)
#define X ((longest_triple_length - singles)/4)
	if ((longest_triple_length - singles) % 4 == 0)
	{
		if ((longest_triple_length - X) >= 2)
			return PATTERN(PLANE_1, longest_triple_end, total);
	}

	return INVALID_PTRN; //not recognized
}

u8 play_legal_check(u32 pattern) //VCALL_UL
{
	// Not a legal pattern, always can't play
	if (TYPE(pattern) == INVALID)
		return(0);

	// opening, must play.
	if (TYPE(last_played_pattern) == INVALID)
		return(TYPE(pattern) != PASS);

	// you can always pass, as long as you're not the person last played.(all other player passes)
	if (TYPE(pattern) == PASS)
		return(last_played_player != current_player);

	// all other players passed, you play anything legal.
	if (current_player == last_played_player)
		return(1);

	// rocket is the biggest
	if (TYPE(pattern) == ROCKET)
		return(1);
	if (TYPE(last_played_pattern) == ROCKET)
		return(0);

	// bomb is second biggest
	if (TYPE(pattern) == BOMB)
	{
		// last player played bomb, compare
		if (TYPE(last_played_pattern) == BOMB)
			return(MAX(pattern) > MAX(last_played_pattern));

		// last player played not a bomb, you play.
		return(1);
	}

	// generally compare the type and max
	return 	(TYPE(pattern) == TYPE(last_played_pattern) &&
		LENGTH(pattern) == LENGTH(last_played_pattern) &&
		MAX(pattern) > MAX(last_played_pattern));
}



XDATA u8 cards_played[20];
u8 play()//ret = u8
{
	XDATA u8 i_kept, i_played, i;
	XDATA u32 pattern;

	pattern = check_pattern();
	i_kept = i_played = i = 0;


	if (play_legal_check(pattern) == 0) return(0);
	
	last_player_pattern = pattern;

	if (TYPE(pattern) == PASS)
		goto SKIP_PASS;

	for (i = 0; i < 20; i++)
		cards_played[i] = NOCARD;
	
	//split player_cards into cards_played and new player_cards according to play_select
	for (i = 0; i < 20; i++)
	{
		if (play_select[i])
			cards_played[i_played++] = player_cards[current_player][i];
		else
			player_cards[current_player][i_kept++] = player_cards[current_player][i];
	}
	
	//fill player's empty card slots
	for(;i_kept<20;i_kept++)
		player_cards[current_player][i_kept] = NOCARD;

	last_played_pattern = pattern;
	last_played_player = current_player;

	player_cards_count[current_player] -= LENGTH(pattern);

SKIP_PASS:;
	current_player = NEXTPLAYER;
	
	if(current_player == last_played_player)
	{
		//all other players passes, clear cards on-table.
		for (i = 0; i < 20; i++)
			cards_played[i] = NOCARD;
	}
	
	return(1);
}
/*

A full game will look like this:

shuffle_send17();       //players each get 17 random cards. 3 cards put in reserved array

set_dizhu(player_id);   //dizhu player receives the cards in reserved array. and is
						//set as current player. card count of each player initialized.
						//also, each players cards are getting sorted to make display better.

//loop until someone has no cards

while(	player_cards_count[0] &&
		player_cards_count[1]
		player_cards_count[2])
{
	play_select = {....};   //set what the player is going to play. flag play_select[i] correspond to player_cards[current_player][i]
	if(play())
		//move is legal. game state updated. do your processing.
	else
		//move is illegal. inertnal game state not changed. do your processing.
}

*/
