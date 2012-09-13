#include "simulation/Elements.h"
#include "virtualmachine/VirtualMachine.h"
//#TPT-Directive ElementClass Element_TEST PT_TEST 255
static vm::VirtualMachine * vMachine;
Element_TEST::Element_TEST()
{
	Identifier = "DEFAULT_PT_TEST";
	Name = "TEST";
	Colour = PIXPACK(0x407020);
	MenuVisible = 1;
	MenuSection = SC_POWDERS;
	Enabled = 1;
	
	Advection = 0.4f;
	AirDrag = 0.01f * CFDS;
	AirLoss = 0.99f;
	Loss = 0.95f;
	Collision = 0.0f;
	Gravity = 0.4f;
	Diffusion = 0.00f;
	HotAir = 0.000f * CFDS;
	Falldown = 1;
	
	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 0;
	
	Weight = 90;
	
	Temperature = R_TEMP+4.0f   +273.15f;
	HeatConduct = 251;
	Description = "Heavy particles. Fissile. Generates neutrons under pressure.";
	
	State = ST_SOLID;
	Properties = TYPE_PART|PROP_NEUTPENETRATE|PROP_RADIOACTIVE;
	
	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;
	

	vMachine = new vm::VirtualMachine(2);
	vMachine->LoadProgram("test.qvm");
	//if(vMachine->Compile())
	/*{
		Update = &Element_TEST::updateCompiled;
	}
	else*/
	{
		Update = &Element_TEST::updateInterpreted;
	}
}

//#TPT-Directive ElementHeader Element_TEST static int updateCompiled(UPDATE_FUNC_ARGS)
int Element_TEST::updateCompiled(UPDATE_FUNC_ARGS)
{
	//vMachine->CallCompiled(0);
	return 0;
}

//#TPT-Directive ElementHeader Element_TEST static int updateInterpreted(UPDATE_FUNC_ARGS)
int Element_TEST::updateInterpreted(UPDATE_FUNC_ARGS)
{
	vm::word w;
	int argAddr = 0, argCount = 5;
	vMachine->sim = sim;

	/* Set up call. */
	vMachine->OpPUSH(w);  //Pointless null in stack
	w.int4 = (argCount + 2) * sizeof(vm::word);
	vMachine->OpENTER(w);
	argAddr = 8;

	//Arguments
	w.int4 = i; vMachine->Marshal(argAddr, w); argAddr += 4;
	w.int4 = x; vMachine->Marshal(argAddr, w); argAddr += 4;
	w.int4 = y; vMachine->Marshal(argAddr, w); argAddr += 4;
	w.int4 = nt; vMachine->Marshal(argAddr, w); argAddr += 4;
	w.int4 = surround_space; vMachine->Marshal(argAddr, w); argAddr += 4;

	w.int4 = 0;
	vMachine->Push(w);

	vMachine->OpCALL(w);
	vMachine->Run();
	w.int4 = (argCount + 2) * sizeof(vm::word);
	vMachine->OpLEAVE(w);
	vMachine->OpPOP(w);   //Pop pointless null
	vMachine->End();
	return 0;
}


Element_TEST::~Element_TEST() {}