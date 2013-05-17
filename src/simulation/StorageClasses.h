#ifndef STORAGECLASSES_H_
#define STORAGECLASSES_H_

#include <string>
#include "Elements.h"

class Renderer;
class Simulation;

/*struct part_type
{
	char *name;
	pixel pcolors;
	float advection;
	float airdrag;
	float airloss;
	float loss;
	float collision;
	float gravity;
	float diffusion;
	float hotair;
	int falldown;
	int flammable;
	int explosive;
	int meltable;
	int hardness;
	int menu;
	int enabled;
	int weight;
	int menusection;
	float heat;
	unsigned char hconduct;
	char *descs;
	char state;
	unsigned int properties;
	int (*update_func) (UPDATE_FUNC_ARGS);
	int (*graphics_func) (GRAPHICS_FUNC_ARGS);
};
typedef struct part_type part_type;*/

/*struct part_transition
{
	float plv; // transition occurs if pv is lower than this
	int plt;
	float phv; // transition occurs if pv is higher than this
	int pht;
	float tlv; // transition occurs if t is lower than this
	int tlt;
	float thv; // transition occurs if t is higher than this
	int tht;
};
typedef struct part_transition part_transition;*/

#endif
