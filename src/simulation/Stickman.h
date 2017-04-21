#ifndef STICKMAN_H_
#define STICKMAN_H_

#define MAX_FIGHTERS 100
struct playerst
{
	char comm;           //command cell
	char pcomm;          //previous command
	int elem;            //element power
	int pelem;           //previous element power
	float legs[16];      //legs' positions
	float accs[8];       //accelerations
	char spwn;           //if stick man was spawned
	int __flags;         //stick man's extra flags
	unsigned int frames; //frames since last particle spawn - used when spawning LIGH
	bool rocketBoots;
	int spawnID;         //id of the SPWN particle that spawns it
	int parentStickman;
	int firstChild;
	int prevStickman;
	int nextStickman;
	int lastChild;
	int self_ID;
};

#endif
