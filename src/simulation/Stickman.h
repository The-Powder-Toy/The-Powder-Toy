#pragma once

constexpr auto MAX_FIGHTERS = 100;
struct playerst
{
	char comm;           //command cell
	char pcomm;          //previous command
	int elem;            //element power
	float legs[16];      //legs' positions
	float accs[8];       //accelerations
	char spwn;           //if stick man was spawned
	unsigned int frames; //frames since last particle spawn - used when spawning LIGH
	bool rocketBoots;
	bool fan;
	int spawnID;         //id of the SPWN particle that spawns it
};


static const int Element_FIGH_moves_mask = 1; // 0 - unseen, 1 - seen
static const int Element_FIGH_square_head_mask = 2; // 0x0 - default, 0x2 - STKM-aligned, square head
static const int Element_FIGH_stk2_mask = 4; // 0x0 - default, 0x2 - STKM-aligned, square head
