#pragma once

#include "common/Vec2.h"

constexpr auto MAX_FIGHTERS = 100;
struct playerst
{
	char comm;           // command cell
	char pcomm;          // previous command
	int pdir;            // previous direction (-1 for left, 1 for right)
	int elem;            // element power
	Vec2<float> downVec{0, 1}; // down direction, used to draw the head properly
	Vec2<float> legs[4] = { {0, 0}, {0, 0}, {0, 0}, {0, 0}};  // legs' coordinates
	Vec2<float> legsp[4] = { {0, 0}, {0, 0}, {0, 0}, {0, 0}}; // previous legs' coordinates
	Vec2<float> accs[4] = { {0, 0}, {0, 0}, {0, 0}, {0, 0}};  // accelerations
	bool spwn;           // if stick man was spawned
	unsigned int frames; // frames since last particle spawn (for LIGH), and shield strength (for SHLD)
	bool rocketBoots;
	bool fan;
	int gaitState;       // current state of the gait cycle
	int gaitTimer;       // timer used to switch between gait states
	bool jumpAllowed;
	bool isJumping;
	bool dashAllowed;
	int swimCounter;
	bool isSwimming; // used in Renderer for underwater graphics
	bool legacy;
};

struct fighterst : playerst
{
	int frustration;
	int prevLife;
	float prevScore;
};
