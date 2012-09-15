#include <cstdio>
#include <cstdlib>
#include <cmath>
#include "VirtualMachine.h"
#include "simulation/Simulation.h"
#include "graphics/Renderer.h"

namespace vm
{
	#define ARG(n) (Get(RP + ((2 + n) * sizeof(word))))

	#define TRAPDEF(f) int VirtualMachine::trap##f()

	TRAPDEF(sin)
	{
		Push<float4_t>(sin(ARG(0).float4));
	}

	TRAPDEF(cos)
	{
		Push<float4_t>(cos(ARG(0).float4));
	}

	TRAPDEF(atan2)
	{
		Push<float4_t>(atan2(ARG(0).float4, ARG(1).float4));
	}

	TRAPDEF(sqrt)
	{
		Push<float4_t>(sqrt(ARG(0).float4));
	}

	TRAPDEF(floor)
	{
		Push<float4_t>(floor(ARG(0).float4));
	}

	TRAPDEF(ceil)
	{
		Push<float4_t>(ceil(ARG(0).float4));
	}


	TRAPDEF(print)
	{
		char *text;
		text = (char*)(ram) + ARG(0).int4;
		printf("%s", text);
	}


	TRAPDEF(error)
	{
		char *msg;
		msg = (char*)(ram) + ARG(0).int4;
		printf("%s", msg);
		End();
	}


	TRAPDEF(partCreate)
	{
		Push<int4_t>(sim->create_part(ARG(0).int4, ARG(1).int4, ARG(2).int4, ARG(3).int4));
	}

	TRAPDEF(partChangeType)
	{
		sim->part_change_type(ARG(0).int4, ARG(1).int4, ARG(2).int4, ARG(3).int4);
	}

	TRAPDEF(pmapData)
	{
		Push<int4_t>(sim->pmap[ARG(1).int4][ARG(0).int4]);
	}

	TRAPDEF(deletePart)
	{
		sim->delete_part(ARG(0).int4, ARG(1).int4, ARG(2).int4);
	}

	TRAPDEF(killPart)
	{
		sim->kill_part(ARG(0).int4);
	}
}
