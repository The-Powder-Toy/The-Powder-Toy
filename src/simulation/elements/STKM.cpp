#include "simulation/ElementCommon.h"
#include "STKM.h"
#include "common/RasterGeometry.h"

static int update(UPDATE_FUNC_ARGS);
static bool createAllowed(ELEMENT_CREATE_ALLOWED_FUNC_ARGS);
static void changeType(ELEMENT_CHANGETYPE_FUNC_ARGS);

void Element::Element_STKM()
{
	Identifier = "DEFAULT_PT_STKM";
	Name = "STKM";
	Colour = 0xFFE0A0_rgb;
	MenuVisible = 1;
	MenuSection = SC_SPECIAL;
	Enabled = 1;

	Advection = 0.5f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 0.2f;
	Loss = 1.0f;
	Collision = 0.0f;
	Gravity = 0.0f;
	NewtonianGravity = 0.0f;
	Diffusion = 0.0f;
	HotAir = 0.00f	* CFDS;
	Falldown = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 0;

	Weight = 50;

	DefaultProperties.temp = R_TEMP + 14.6f + 273.15f;
	HeatConduct = 0;
	Description = "Stickman. Don't kill him! Control with the arrow keys.";

	Properties = TYPE_PART | PROP_NOCTYPEDRAW;
	CarriesTypeIn = 1U << FIELD_CTYPE;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 620.0f;
	HighTemperatureTransition = PT_FIRE; //@ STKM -> FIRE

	DefaultProperties.life = 100;
	DefaultProperties.ctype = PT_NONE; // This isn't PT_DUST deliberately

	Update = &update;
	Graphics = &Element_STKM_graphics;
	CreateAllowed = &createAllowed;
	ChangeType = &changeType;
}

// Field .tmp is used by FIGH

// Normal gait (target 0.8f)
float paramsBest[13] = {10.99, 0.707861, 6.46488, 0.57078, -0.775594, 0.756983, 7.89624, 0.208891, 0.356459, 0.366157, 0.129568, 0.953543, -0.919775};
// Fast gait (target 1.2f)
float paramsFast[13] = {10.9053, 0.696392, 6.45738, 0.684548, -0.8, 0.8, 7.66924, 0.386053, 0.537031, 0.0526862, 0.176239, 0.980492, -1.01767};
// Slow gait (target 0.4f)
float paramsSlow[13] = {10.99, 0.9, 6.50731, 0.384986, -0.713069, 0.740817, 8.01324, 0.262807, 0.477572, 0.25135, 0.188304, 0.732634, -0.798867};

static int update(UPDATE_FUNC_ARGS)
{
	return Element_STKM_run_stickman(&sim->player, UPDATE_FUNC_SUBCALL_ARGS);
}

int Element_STKM_graphics(GRAPHICS_FUNC_ARGS)
{
	*colr = *colg = *colb = *cola = 0;
	*pixel_mode = PSPEC_STICKMAN;
	return 1;
}

static bool createAllowed(ELEMENT_CREATE_ALLOWED_FUNC_ARGS)
{
	return sim->elementCount[PT_STKM] <= 0 && !sim->player.spwn;
}

static void changeType(ELEMENT_CHANGETYPE_FUNC_ARGS)
{
	if (to == PT_STKM)
	{
		Element_STKM_init_legs(sim, &sim->player, i);
		sim->player.spwn = true;
	}
	else
		sim->player.spwn = false;
}

// Draw a line but coordinates are given by Vec
void drawLineVec(Simulation *sim, Vec2<int> start, Vec2<int> end, int type)
{
	RasterizeLine<false>(start, end,
		[sim, type](Vec2<int> pos)
		{
			if (InBounds(pos.X, pos.Y) && !sim->pmap[pos.Y][pos.X])
				sim->create_part(-1, pos.X, pos.Y, type);
		}
	);
}

// One step of Verlet integration
void verletStep(Vec2<float>& curr, Vec2<float>& old, Vec2<float> accel, float dt)
{
	auto newp = curr*2.0f - old + accel*dt*dt;
	old = curr;
	curr = newp;
}

// Enforce a joint constraint between points end1 and end2. Legacy version
void simJointLegacy(Vec2<float>& end1, Vec2<float>& end2, float dist)
{
	float dist2 = dist*dist;
	float d = dist2 / (pow(end1.X - end2.X, 2) + pow(end1.Y - end2.Y, 2) + dist2) - 0.5f; // Fast distance
	end2 -= (end1 - end2)*d; // This is incorrect since end2 is changed, but that's how legacy code does it
	end1 += (end1 - end2)*d;
}

// Vector length
float lenVec(Vec2<float> vec)
{
	return hypot(vec.X, vec.Y);
}

// Normalize a vector
void normalize(Vec2<float>& vec)
{
	float len = lenVec(vec);
	if (len > 0)
		vec = vec * (1/len);
}

// Enforce a joint constraint between points end1 and end2
void simJoint(Vec2<float>& end1, Vec2<float>& end2, float dist)
{
	Vec2<float> diff = end1 - end2;
	float len = lenVec(diff);
	float perc = len < 0.001f ? -1.0f : (len - dist)/len;
	Vec2<float> adj = diff*(0.5f*perc);

	end1 -= adj;
	end2 += adj;
}

// Enforce a spring constraint between points end1 and end2.
// Takes rest length, stiffness, returns acceleration
Vec2<float> simSpring(Vec2<float> end1, Vec2<float> end2, float rest, float stiff)
{
	Vec2<float> diff = end1 - end2;
	float len = lenVec(diff);
	float perc = (len - rest)/rest;
	Vec2<float> adj = diff*(0.5f*stiff*perc);

	return adj;
}

// Scalar product of two vectors
float scalarProd(Vec2<float> a, Vec2<float> b)
{
	return a.X*b.X + a.Y*b.Y;
}

// Cross product of two vectors, returns a scalar
float crossProd(Vec2<float> a, Vec2<float> b)
{
	return a.X*b.Y - a.Y*b.X;
}

// Enforce an angle constraint, a-b-c can bend as ^ if dir is true, and v if dir is false
void simAngle(bool dir, Vec2<float> a, Vec2<float>& b, Vec2<float>& c)
{
	float sgn = dir ? 1.0f : -1.0f;
	Vec2<float> ab = b - a;
	Vec2<float> ac = c - a;
	float lab = lenVec(ab);

	if (lab > 0.0001f)
	{
		float scProd = scalarProd(ab, ac);
		float vcProd = crossProd(ac, ab);
		Vec2<float> proj = a + (ab * (scProd / (lab*lab)));
		Vec2<float> adj = (proj - c)*(0.5f);

		if (sgn*vcProd < 0)
		{
			b -= adj;
			c += adj;
		}
	}
}

bool VecInBounds(Vec2<float> pos)
{
	return InBounds((int)pos.X, (int)pos.Y);
}

// Check if a point moving from prev to curr collides with something.
// If "move" flag is true, we set curr to a collision point.
// If "slip" flag is true, collisions will be computed with low friction
bool collide(Simulation* sim, int t, Vec2<float> prev, Vec2<float>& curr, bool move = false, bool slip = false)
{
	// A crude line-stepping algorithm
	Vec2<int> begin{int(prev.X), int(prev.Y)};
	Vec2<int> end{int(curr.X), int(curr.Y)};

	int steps = std::max(abs(end.X - begin.X), abs(end.Y - begin.Y));
	Vec2<float> delta = curr - prev;

	if (steps > 0)
	{
		delta = delta * (1.0f/(float(steps)));
		Vec2<float> p = prev + delta;
		Vec2<float> prevP = prev;

		for(int i = 0; i < steps; i++)
		{
			int ipx = p.X;
			int ipy = p.Y;

			if (InBounds(ipx, ipy) && !sim->eval_move(t, ipx, ipy, nullptr))
			{
				if (move)
				{
					auto tr = TYP(sim->pmap[ipy][ipx]);
					bool slippery = (tr == PT_ICEI || tr == PT_NICE || tr == PT_RIME || tr == PT_DRIC || tr == PT_GLAS);

					if (slip || slippery)
					{
						Vec2<float> dx{delta.X, 0};
						Vec2<float> dy{0, delta.Y};

						// Do a step in the X direction
						auto doX = [&]() {
							if (sim->eval_move(t, prevP.X + delta.X, prevP.Y, nullptr))
							{
								curr = prevP + dx;
								return true;
							}
							return false;
						};
						// Do a step in the Y direction
						auto doY = [&]() {
							if (sim->eval_move(t, prevP.X, prevP.Y + delta.Y, nullptr))
							{
								curr = prevP + dy;
								return true;
							}
							return false;
						};

						if (fabs(delta.X) > fabs(delta.Y))
						{
							// First try X direction, then Y
							if (doX()) return true;
							if (doY()) return true;
						}
						else
						{
							// First try Y direction, then X
							if (doY()) return true;
							if (doX()) return true;
						}
					}

					// Return previous safe point
					curr = prevP;
				}

				return true;
			}
			else
			{
				// Advance the point further
				prevP = p;
				p += delta;
			}
		}
	}

	return false;
}

// Check if a given position contains a liquid
bool isLiquid(Simulation* sim, Vec2<float> pos)
{
	if (VecInBounds(pos))
	{
		auto r = sim->pmap[int(pos.Y)][int(pos.X)];

		if (!r)
			return false;

		auto &sd = SimulationData::CRef();
		auto &elements = sd.elements;

		return (elements[TYP(r)].Properties & TYPE_LIQUID);
	}
	else
		return false;
}

// Check if a given position is free (i.e. contains air)
bool isAir(Simulation* sim, Vec2<float> pos)
{
	if (VecInBounds(pos))
	{
		auto r = sim->pmap[int(pos.Y)][int(pos.X)];

		return r == 0;
	}
	return true;
}

void die(Simulation *sim, playerst *playerp, int i)
{
	int x = (int)(sim->parts[i].x + 0.5f);
	int y = (int)(sim->parts[i].y + 0.5f);
	auto elem = playerp->elem;

	auto &sd = SimulationData::CRef();
	auto &elements = sd.elements;

	// Don't do anything when we have FAN or solid power
	if (!playerp->fan && (elements[elem].Properties & (TYPE_PART | TYPE_GAS | TYPE_LIQUID | TYPE_ENERGY)))
	{
		Vec2<int> hv{x, y};
		Vec2<float> dwn = playerp->downVec;
		int t = sim->parts[i].type;

		// This code follows the graphics code for drawing stickman's head
		if (t == PT_FIGH)
		{
			Vec2<int> dv{int(round(dwn.X*2.0f)), int(round(dwn.Y*2.0f))};
			Vec2<int> lv{dv.Y, -dv.X}; // dv rotated 90 degrees

			for (int eps1 = -1; eps1 < 2; eps1 += 2)
				for (int eps2 = -1; eps2 < 2; eps2 += 2)
					drawLineVec(sim, hv + dv*eps1, hv + lv*eps2, elem);
		}
		else if (t == PT_STKM)
		{
			// We use 2.1 here instead of "correct" 2.0 because it looks better that way
			Vec2<int> dv{int(round((dwn.X - dwn.Y)*2.1f)), int(round((dwn.X + dwn.Y)*2.1f))};
			Vec2<int> lv{dv.Y, -dv.X}; // dv rotated 90 degrees

			for (int eps1 = -1; eps1 < 2; eps1 += 2)
				for (int eps2 = -1; eps2 < 2; eps2 += 2)
					drawLineVec(sim, hv + dv*eps1, hv + lv*eps2, elem);
		}
		else if (t == PT_STKM2)
		{
			// A pixelated circle
			drawLineVec(sim, hv + Vec2<int>{-1, -2}, hv + Vec2<int>{1, -2}, elem);
			drawLineVec(sim, hv + Vec2<int>{-1, 2}, hv + Vec2<int>{1, 2}, elem);
			drawLineVec(sim, hv + Vec2<int>{-2, -1}, hv + Vec2<int>{-2, 1}, elem);
			drawLineVec(sim, hv + Vec2<int>{2, -1}, hv + Vec2<int>{2, 1}, elem);
		}
	}

	// Clear just in case
	playerp->spwn = false;

	// Kill the head particle
	sim->kill_part(i);
}

// STKM feet interaction
void Element_STKM_interact(Simulation *sim, playerst *playerp, int i, int x, int y)
{
	auto &sd = SimulationData::CRef();
	auto &elements = sd.elements;
	int r;

	if (x<0 || y<0 || x>=XRES || y>=YRES || !sim->parts[i].type)
		return;

	r = sim->pmap[y][x];
	if (r)
	{
		// Ignore PLSM if we're in rocket boots
		if (playerp->rocketBoots && TYP(r) == PT_PLSM)
			return;

		int damage = 0;

		if (TYP(r)==PT_SPRK && playerp->elem!=PT_LIGH)
			damage += 5;

		if (!sd.IsHeatInsulator(sim->parts[ID(r)]) && ((playerp->elem!=PT_LIGH && sim->parts[ID(r)].temp>=373.15f) || sim->parts[ID(r)].temp<=173.15f))
			damage++;

		if (elements[TYP(r)].Properties&PROP_DEADLY)
			damage++;

		if (elements[TYP(r)].Properties&PROP_RADIOACTIVE)
			damage++;

		// Shield eats some damage away
		if (playerp->elem == PT_SHLD1 && playerp->frames > 0)
		{
			int m = std::min(damage, int(playerp->frames));
			damage -= m;
			playerp->frames -= m;
		}

		if (damage)
			sim->parts[i].life = std::max(0, sim->parts[i].life - damage);

		if (TYP(r)==PT_BHOL || TYP(r)==PT_NBHL)
		{
			sim->parts[i].life = 0;
			return;
		}

		if ((TYP(r)==PT_VOID || (TYP(r)==PT_PVOD && sim->parts[ID(r)].life==10)) &&
				(!sim->parts[ID(r)].ctype || (sim->parts[ID(r)].ctype==sim->parts[i].type)!=(sim->parts[ID(r)].tmp&1)))
		{
			sim->parts[i].life = 0;
			return;
		}

		// This is old teleportation code from _legacy
		if (TYP(r)==PT_PRTI && sim->parts[i].type)
		{
			int nnx, count=1;//gives rx=0, ry=1 in update_PRTO
			sim->parts[ID(r)].tmp = (int)((sim->parts[ID(r)].temp-73.15f)/100+1);
			if (sim->parts[ID(r)].tmp>=CHANNELS) sim->parts[ID(r)].tmp = CHANNELS-1;
			else if (sim->parts[ID(r)].tmp<0) sim->parts[ID(r)].tmp = 0;
			for (nnx=0; nnx<80; nnx++)
				if (!sim->portalp[sim->parts[ID(r)].tmp][count][nnx].type)
				{
					sim->portalp[sim->parts[ID(r)].tmp][count][nnx] = sim->parts[i];
					sim->kill_part(i);
					//stop new STKM/fighters being created to replace the ones in the portal:
					playerp->spwn = true;
					if (sim->portalp[sim->parts[ID(r)].tmp][count][nnx].type==PT_FIGH)
						sim->fighcount++;
					break;
				}
		}
	}
}

int Element_STKM_movement(playerst *playerp, UPDATE_FUNC_ARGS)
{
	float dt = 0.7f;
	int t = parts[i].type;
	Vec2<float> grav{0, 0};
	Vec2<float> headAcc{0, 0};

	bool lrPressed = (int)(playerp->comm)&3; // Are we going left or right?
	int direction = 0; // -1 left, 0 stay, 1 right

	// Determine walking direction
	if (int(playerp->comm)&1)
		direction = -1;
	if (int(playerp->comm)&2)
		direction = 1;

	// Overwrite previous direction
	if (direction)
		playerp->pdir = direction;

	// Set gravity vector
	sim->GetGravityField(parts[i].x, parts[i].y, 1.0f, 1.0f, grav.X, grav.Y);

	// Antigravity mode
	if (playerp->elem == PT_GPMP)
		grav = grav * (-1.0f);

	// Set (normalized) down direction
	Vec2<float> downVec{0, 0};
	float gravLen = lenVec(grav);
	if (gravLen > 0.01f)
		downVec = grav * (1/gravLen);
	else
		downVec = Vec2<float>(0, 1);

	playerp->downVec = downVec;

	// Horizontal (normalized) direction vector
	Vec2<float> horizVec{downVec.Y, -downVec.X};

	// horizVec will point in the moving direction (if there is one)
	if (direction)
		horizVec = horizVec*(float(direction));

	// Head floats up
	if (gravLen > 1.0f)
		headAcc -= downVec*(dt*dt); // Cap head's "antigravity force"
	else
		headAcc -= grav*(dt*dt);

	// Feet go down
	playerp->accs[1] += grav;
	playerp->accs[3] += grav;

	// Verlet integration
	for(int j = 0; j < 4; j++)
	{
		verletStep(playerp->legs[j], playerp->legsp[j], playerp->accs[j], dt);
		playerp->accs[j] = Vec2<float>{0, 0};
	}

	// Since STKM head is not a Verlet point, we create this "dummy" version and update vx/vy later
	Vec2<float> dummyHead{parts[i].x, parts[i].y};

	// Do several iterations of constraint solving
	for(int j = 0; j < 4; j++)
	{
		// Angle constraints
		if (direction)
		{
			simAngle(direction == (-1), dummyHead, playerp->legs[0], playerp->legs[1]);
			simAngle(direction == (-1), dummyHead, playerp->legs[2], playerp->legs[3]);
		}

		// Simulation of joints
		simJoint(playerp->legs[0], playerp->legs[1], 5.0f);
		simJoint(playerp->legs[2], playerp->legs[3], 5.0f);

		simJoint(dummyHead, playerp->legs[0], 6.0f);
		simJoint(dummyHead, playerp->legs[2], 6.0f);
	}

	parts[i].vx += dummyHead.X - parts[i].x;
	parts[i].vy += dummyHead.Y - parts[i].y;

	// Reset dummy head to current coords
	dummyHead = Vec2<float>(parts[i].x, parts[i].y);

	// Count limbs which are underwater or in air (as opposed to underground)
	bool swimming = false;
	bool newSwimming = false;
	int immersedLimb = 0;
	int airLimb = 0;
	for(int j = 0; j < 4; j++)
	{
		if (isLiquid(sim, playerp->legs[j]))
			immersedLimb++;
		if (isAir(sim, playerp->legs[j]))
			airLimb++;
	}

	swimming = (immersedLimb > 2);
	newSwimming = swimming;

	// Allow swimming for some time even if not enough limbs are underwater
	bool headAboveWater = playerp->comm && isAir(sim, dummyHead + downVec);
	if (!swimming && !headAboveWater && immersedLimb > 1)
	{
		if (playerp->swimCounter > 0)
		{
			swimming = true;
			playerp->swimCounter--;
		}
		else
			playerp->swimCounter = 0; // Just in case we got a negative value somehow
	}

	playerp->isSwimming = swimming;

	// Disable gravity for legs which are immersed in liquid
	for(int j = 0; j < 4; j += 2)
		if (isLiquid(sim, playerp->legs[j]) && isLiquid(sim, playerp->legs[j+1]))
			playerp->accs[j+1] -= grav;

	// Lazy leg has lower friction with the surfaces
	// Helps with STKM getting stuck in 1px ledges
	int lazyLeg = (playerp->gaitState == 0) ? 1 : 3;
	if (playerp->gaitState != 0 && playerp->gaitState != 2)
		lazyLeg = 10; // Junk value

	// Collision with obstacles
	for(int j = 1; j < 4; j += 2)
	{
		// If we are moving from the occupied spot, use the old collision method
		if (VecInBounds(playerp->legsp[j]) && !sim->eval_move(t, int(playerp->legsp[j].X), int(playerp->legsp[j].Y), nullptr))
		{
			if (VecInBounds(playerp->legs[j]) && !sim->eval_move(t, int(playerp->legs[j].X), int(playerp->legs[j].Y), nullptr))
				playerp->legs[j] = playerp->legsp[j];
		}
		else
			collide(sim, t, playerp->legsp[j], playerp->legs[j], true, swimming || (j == lazyLeg));
	}

	if (swimming)
	{
		// Set the swimming "cooldown" counter
		if (newSwimming)
			playerp->swimCounter = 60;

		playerp->isJumping = false;

		// Disable head flying up
		headAcc += grav*(dt*dt);

		const float cCounter = 0.4f;

		// Swim left/right
		if (direction)
		{
			headAcc += horizVec*(dt*dt);
			for(int j = 1; j < 4; j += 2)
				playerp->accs[j] -= horizVec*cCounter;
		}

		// Swim up
		if (playerp->comm&4)
		{
			headAcc -= downVec*(dt*dt);
			for(int j = 1; j < 4; j += 2)
				playerp->accs[j] += downVec*cCounter;
		}

		// Swim down
		if (playerp->comm&8)
		{
			headAcc += downVec*(dt*dt);
			for(int j = 1; j < 4; j += 2)
				playerp->accs[j] -= downVec*cCounter;
		}
	}
	else if (playerp->rocketBoots && (playerp->comm&0x04))
	{
		const float rBootsUp = 0.1f;
		const float rBootsSide = 0.14f;

		// Fly up
		headAcc -= downVec*(rBootsUp*dt*dt);
		for(int j = 1; j < 4; j += 2)
			playerp->accs[j] -= grav*0.5f + downVec*rBootsUp;

		// Fly left/right
		if (direction)
		{
			headAcc += horizVec*(rBootsSide*dt*dt);
			for(int j = 1; j < 4; j += 2)
				playerp->accs[j] += horizVec*rBootsSide;
		}

		// Keep legs apart
		auto adj = simSpring(playerp->legs[1], playerp->legs[3], 7, 0.2f);
		playerp->accs[1] -= adj;
		playerp->accs[3] += adj;

		// PLSM trail
		for (int j = 1; j < 4; j += 2)
		{
			Vec2<float> spwn = playerp->legs[j] + downVec;
			int np = sim->create_part(-1, spwn.X, spwn.Y, PT_PLSM);
			if (np>=0)
			{
				parts[np].vx = parts[i].vx+20.0f*downVec.X;
				parts[np].vy = parts[i].vy+20.0f*downVec.Y;
				parts[np].life += 10;
			}
		}
	}
	else
	{
		// Select gait state when we start running or change direction
		if (lrPressed && ((playerp->comm&3) != (playerp->pcomm&3)))
		{
			Vec2<float> leg1 = playerp->legs[1] - dummyHead;
			Vec2<float> leg3 = playerp->legs[3] - dummyHead;
			float proj1 = scalarProd(leg1, horizVec);
			float proj3 = scalarProd(leg3, horizVec);

			if (proj1 > proj3)
				playerp->gaitState = 1;
			else
				playerp->gaitState = 3;
		}

		// Terminate jump on contact with the ground
		Vec2<float> downFoot1 = playerp->legs[1] + downVec;
		Vec2<float> downFoot3 = playerp->legs[3] + downVec;
		if (collide(sim, t, playerp->legs[1], downFoot1) || collide(sim, t, playerp->legs[3], downFoot3))
			playerp->isJumping = false;

		// Try to keep legs apart
		if (playerp->isJumping)
		{
			auto adj = simSpring(playerp->legs[1], playerp->legs[3], 7, 0.2f);
			playerp->accs[1] -= adj;
			playerp->accs[3] += adj;
		}

		if (playerp->gaitTimer > 0)
			playerp->gaitTimer--;

		// Gait parameters
		float* params = paramsBest;
		switch(playerp->elem)
		{
			case PT_ACEL:
				params = paramsFast;
				break;
			case PT_DCEL:
				params = paramsSlow;
				break;
			default:
				params = paramsBest;
				break;
		}

		// Locomotion state machine
		switch(playerp->gaitState)
		{
			case 0:
			case 2:
				if (playerp->gaitTimer == 0)
					playerp->gaitState++;
				break;
			case 1:
			case 3:
				{
					int swingFoot = playerp->gaitState; // Swing foot number is the same as gaitState
					Vec2<float> downFoot = playerp->legs[swingFoot] + downVec;
					if (lrPressed && collide(sim, t, playerp->legs[playerp->gaitState], downFoot))
					{
						playerp->gaitState = (playerp->gaitState+1) % 4;
						playerp->gaitTimer = (int)(params[6] / dt);
					}
				}
				break;
			default:
				break;
		}

		// Move the stickman
		if (!playerp->isJumping && (playerp->gaitState == 0 || playerp->gaitState == 2))
		{
			int stanceFoot = (playerp->gaitState == 0 || playerp->gaitState == 1) ? 3 : 1;
			int swingFoot = (playerp->gaitState == 0 || playerp->gaitState == 1) ? 1 : 3;
			Vec2<float> adj{0, 0};

			// Extend the stance leg
			adj = simSpring(playerp->legs[stanceFoot], dummyHead, params[0], params[1]);
			playerp->accs[stanceFoot] -= adj;
			headAcc += adj*(dt*dt);

			// Contract the swing leg
			adj = simSpring(playerp->legs[swingFoot], dummyHead, params[2], params[3]);
			playerp->accs[swingFoot] -= adj;
			headAcc += adj*(dt*dt);

			// Rotate the swing leg
			Vec2<float> swingVel = (playerp->legs[swingFoot] - playerp->legsp[swingFoot]) * (1.0f/dt); // Swing leg velocity
			Vec2<float> legVec = playerp->legs[swingFoot] - dummyHead; // Radius-vector from the head to the swing leg

			Vec2<float> perpVec{legVec.Y, -legVec.X}; // (Normalized) perpendicular vector
			normalize(perpVec);

			float angleRef = float(direction)*params[7];
			float angleCurr = (legVec.X || legVec.Y) ? atan2(crossProd(legVec, downVec), scalarProd(legVec, downVec)) : 0;
			float omegaRef = float(direction)*params[10];
			float omegaCurr = scalarProd(perpVec, swingVel);
			float coeff = params[8]*(angleRef - angleCurr) + params[9]*(omegaRef - omegaCurr);

			adj = perpVec*coeff;
			playerp->accs[swingFoot] += adj;
			headAcc -= adj*(dt*dt);

			// Rotate the stance leg (essentially move head only)
			Vec2<float> headVel{parts[i].vx / dt, parts[i].vy / dt};
			legVec = playerp->legs[stanceFoot] - dummyHead; // Radius-vector from the head to the stance leg
			perpVec = Vec2<float>(legVec.Y, -legVec.X);
			normalize(perpVec);

			angleRef = float(direction)*params[12];
			angleCurr = (legVec.X || legVec.Y) ? atan2(crossProd(legVec, downVec), scalarProd(legVec, downVec)) : 0;
			omegaRef = float(direction)*params[11];
			omegaCurr = scalarProd(perpVec, headVel);
			coeff = params[4]*(angleRef - angleCurr) + params[5]*(omegaRef - omegaCurr);

			adj = perpVec*coeff;
			headAcc += adj*(dt*dt);
		}

		if (!(playerp->comm&4))
		{
			// Reduce vertical velocity if we release the jumping key during the jump
			// Allows shorter jumps
			if (!playerp->jumpAllowed)
			{
				const float redFactor = 0.5f;
				Vec2<float> headVel{parts[i].vx, parts[i].vy};

				headVel = downVec*(redFactor*scalarProd(headVel, downVec)) + horizVec*scalarProd(headVel, horizVec);
				parts[i].vx = headVel.X;
				parts[i].vy = headVel.Y;

				for(int j = 0; j < 4; j++)
				{
					Vec2<float> legVel = playerp->legs[j] - playerp->legsp[j];
					playerp->legsp[j] = playerp->legs[j] - (downVec*(redFactor*scalarProd(legVel, downVec)) + horizVec*scalarProd(legVel, horizVec));
				}
			}

			// Allow jumping only after we release the jumping key
			// If we press the key again during the jump, it will be "buffered"
			playerp->jumpAllowed = true;
		}

		// Jump
		if (playerp->jumpAllowed && !swimming && (playerp->comm&4))
		{
			// Coordinates used to check if feet touch a wall
			Vec2<float> dirFoot1 = playerp->legs[1] - horizVec;
			Vec2<float> dirFoot3 = playerp->legs[3] - horizVec;

			bool verticalJump = collide(sim, t, playerp->legs[1], downFoot1) || collide(sim, t, playerp->legs[3], downFoot3);
			bool wallJump = playerp->isJumping && direction &&
				(collide(sim, t, playerp->legs[1], dirFoot1) || collide(sim, t, playerp->legs[3], dirFoot3));

			// If we have contact with the ground or the wall
			if (verticalJump || wallJump)
			{
				// Vertical and horizontal initial velocities
				float verticalCoeff = wallJump ? -2.2f : -2.0f;
				float horizontalCoeff = wallJump ? 0.3f : 0.2f;

				// ACEL power boosts jumping height and length
				if (playerp->elem == PT_ACEL)
				{
					verticalCoeff *= 1.2f;
					horizontalCoeff *= 1.5f;
				}
				else if (playerp->elem == PT_DCEL) // DCEL reduces jump params
				{
					verticalCoeff *= 0.7f;
					horizontalCoeff *= 0.5f;
				}

				Vec2<float> jumpingVel = downVec*verticalCoeff;
				float horizVel = direction ? horizontalCoeff : 0.0f;

				// Set the horizontal and vertical components
				Vec2<float> newHeadVel = jumpingVel + horizVec*horizVel;
				parts[i].vx = newHeadVel.X;
				parts[i].vy = newHeadVel.Y;

				for(int j = 0; j < 4; j++)
					playerp->legsp[j] = playerp->legs[j] - (jumpingVel + horizVec*horizVel);

				playerp->jumpAllowed = false;
				playerp->isJumping = true;
				playerp->dashAllowed = true;
			}
		}

		// Nudge the horizontal velocity during the jump if left/right is pressed
		if (lrPressed && !playerp->jumpAllowed)
		{
			Vec2<float> headVel{parts[i].vx, parts[i].vy};
			const float nudge = 0.1f;
			const float maxVel = playerp->elem == PT_ACEL ? 1.2f : 0.9f;

			if (scalarProd(headVel, horizVec) < maxVel)
			{
				parts[i].vx += horizVec.X*nudge*dt*dt;
				parts[i].vy += horizVec.Y*nudge*dt*dt;
			}

			for(int j = 0; j < 4; j++)
			{
				Vec2<float> legVel = playerp->legs[j] - playerp->legsp[j];

				if (scalarProd(legVel, horizVec) < maxVel)
					playerp->accs[j] += horizVec*nudge;
			}
		}

		// Action key pressed
		if (playerp->comm&8)
		{
			// Previous direction and the corresponding vector
			Vec2<float> prevHorizVec = Vec2<float>(downVec.Y, -downVec.X)*float(playerp->pdir);

			// Coordinates where we will spawn particles
			Vec2<float> spawnCoord = Vec2<float>(x, y) + prevHorizVec*3.0f;
			int sx = spawnCoord.X;
			int sy = spawnCoord.Y;

			auto &sd = SimulationData::CRef();
			auto &elements = sd.elements;

			auto r = 0;
			if (InBounds(sx, sy))
				r = pmap[sy][sx];

			// Spark conductive solids
			if (r && (((elements[TYP(r)].Properties & (TYPE_SOLID|PROP_CONDUCTS)) == (TYPE_SOLID|PROP_CONDUCTS)) || TYP(r) == PT_SPRK))
			{
				sim->create_part(-1, sx, sy, PT_SPRK);
			}
			else if (playerp->fan) // FAN power
			{
				int bx = sx/CELL, by = sy/CELL;

				if (InCellBounds(bx, by))
				{
					const float fanVel = 5.0f;
					sim->vx[by][bx] = prevHorizVec.X*fanVel;
					sim->vy[by][bx] = prevHorizVec.Y*fanVel;

					// Recoil
					headAcc -= prevHorizVec*(fanVel*0.02f);
				}
			}
			else if (playerp->elem == PT_ARAY) // Dash
			{
				if (playerp->dashAllowed && lrPressed)
				{
					int time = 1;
					Vec2<float> dashVec{0, 0};

					// Determine the length of the dash
					while(time < 40)
					{
						dashVec = horizVec*float(time);

						// Check if we hit a wall during the dash
						bool slam = !sim->try_move(i, x, y, x+dashVec.X, y+dashVec.Y);
						for(int j = 0; j < 4; j++)
						{
							Vec2<float> legd = playerp->legs[j] + dashVec;
							slam = slam || collide(sim, t, playerp->legs[j], legd);
						}

						if (slam)
						{
							time -= 1;
							break;
						}
						else
							time += 1;
					}

					// Perform the dash
					if (time > 0)
					{
						dashVec = horizVec*float(time);

						// Draw an EMBR trail
						for(int tt = 0; tt < time-1; tt += 5)
						{
							for(int j = 1; j < 4; j += 2)
							{
								auto embrPos = playerp->legs[j] + horizVec*float(tt);
								int ei = sim->create_part(-1, embrPos.X, embrPos.Y, PT_EMBR);
								if (ei != -1)
									parts[ei].life = 10;
							}
						}

						// Move the head and the limbs
						auto newHeadPos = dummyHead + dashVec;
						if (sim->do_move(i, x, y, newHeadPos.X, newHeadPos.Y))
						{
							// Project head velocity on the horizontal vector
							// This is a bit incorrect since later we add headAcc, but it's good enough
							float sp = scalarProd(Vec2<float>{parts[i].vx, parts[i].vy}, horizVec);
							parts[i].vx = horizVec.X*sp;
							parts[i].vy = horizVec.Y*sp;

							for(int j = 0; j < 4; j++)
							{
								playerp->legsp[j] += dashVec;
								playerp->legs[j] += dashVec;

								// Project leg velocity on the horizontal vector
								Vec2<float> vp = playerp->legsp[j] - playerp->legs[j];
								sp = scalarProd(vp, horizVec);
								playerp->legsp[j] = playerp->legs[j] + horizVec*sp;
							}

							if (sim->edgeMode == EDGE_LOOP)
							{
								// This repeats the looping code from do_move
								float nxf = newHeadPos.X, nyf = newHeadPos.Y;
								int nx = (int)(nxf+0.5f), ny = (int)(nyf+0.5f);
								bool x_ok = (nx >= CELL && nx < XRES-CELL);
								bool y_ok = (ny >= CELL && ny < YRES-CELL);

								if (!x_ok)
									nxf = remainder_p(nxf-CELL+.5f, XRES-CELL*2.0f)+CELL-.5f;
								if (!y_ok)
									nyf = remainder_p(nyf-CELL+.5f, YRES-CELL*2.0f)+CELL-.5f;

								nx = (int)(nxf+0.5f);
								ny = (int)(nyf+0.5f);

								// "Loop" the limbs
								if (!x_ok || !y_ok)
								{
									Vec2<float> loopVec{float(nx - x), float(ny - y)};
									for (int j = 0; j < 4; j++)
									{
										playerp->legs[j] += loopVec-dashVec;
										playerp->legsp[j] += loopVec-dashVec;
									}
								}
							}
						}
						playerp->dashAllowed = false;
					}
				}
			}
			else if (playerp->elem == PT_LIGH) // Lightning power
			{
				int np = -1;

				if (playerp->frames > 0)
					playerp->frames--;
				else
					np = sim->create_part(-1, sx, sy, PT_LIGH);

				if (np >= 0)
				{
					int angle = int(atan2(-prevHorizVec.Y, prevHorizVec.X)*180.0f/std::numbers::pi_v<float>);
					const int power = 50;

					parts[np].tmp = angle;
					parts[np].life = sim->rng.between(0, 1+power/15) + power/7;
					parts[np].temp = 273.15f + 500.0f;
					parts[np].tmp2 = 1;

					playerp->frames = 10; // LIGH cooldown
				}
			}
			else if (playerp->elem > 0 && playerp->elem < PT_NUM) // Spawn everything else
			{
				if (elements[playerp->elem].Properties & (TYPE_PART | TYPE_GAS | TYPE_LIQUID | TYPE_ENERGY))
				{
					bool isGas = elements[playerp->elem].Properties & TYPE_GAS;

					// Spawn gases further away
					if (isGas)
					{
						const float disp = 5.0f;

						sx += prevHorizVec.X*disp + downVec.X*disp;
						sy += prevHorizVec.Y*disp + downVec.Y*disp;
					}

					auto np = sim->create_part(-1, sx, sy, playerp->elem);
					if (np >= 0)
					{
						float spawnVel = 4.0f;

						parts[np].vx = 0;
						parts[np].vy = 0;

						// Shoot gases downwards
						if (isGas)
						{
							spawnVel *= 3.0f;
							parts[np].vx = downVec.X*spawnVel;
							parts[np].vy = downVec.Y*spawnVel;

							// Make horizontal velocity even larger
							spawnVel *= 3.0f;
						}

						parts[np].vx += prevHorizVec.X*spawnVel;
						parts[np].vy += prevHorizVec.Y*spawnVel;

						// Recoil
						headAcc -= prevHorizVec*(spawnVel*float(elements[playerp->elem].Weight)*0.0003f);
					}
				}
			}
		}
	}

	// Update head velocity
	parts[i].vx += headAcc.X;
	parts[i].vy += headAcc.Y;

	// Slow down STKM when swimming
	if (swimming)
	{
		parts[i].vx *= 0.9f;
		parts[i].vy *= 0.9f;
	}

	// Charge detector wall
	for (int j = 0; j < 4; j++)
	{
		auto leg = playerp->legs[j];
		Vec2<int> dot{int(leg.X)/CELL, int(leg.Y)/CELL};

		if (InCellBounds(dot.X, dot.Y) && sim->bmap[dot.Y][dot.X] == WL_DETECT)
			sim->set_emap(dot.X, dot.Y);
	}

	// Check if feet touch something
	Element_STKM_interact(sim, playerp, i, (int)(playerp->legs[1].X), (int)(playerp->legs[1].Y));
	Element_STKM_interact(sim, playerp, i, (int)(playerp->legs[3].X), (int)(playerp->legs[3].Y));
	Element_STKM_interact(sim, playerp, i, (int)(playerp->legs[1].X+downVec.X), (int)(playerp->legs[1].Y+downVec.Y));
	Element_STKM_interact(sim, playerp, i, (int)(playerp->legs[3].X+downVec.X), (int)(playerp->legs[3].Y+downVec.Y));

	playerp->pcomm = playerp->comm;

	return 0;
}

int Element_STKM_run_stickman_legacy(playerst *playerp, UPDATE_FUNC_ARGS);

// Current STKM update function
int Element_STKM_run_stickman(playerst *playerp, UPDATE_FUNC_ARGS)
{
	// Fallback to legacy STKM update function when the flag is set
	if (parts[i].flags & FLAG_OLDSTKM)
	{
		playerp->legacy = true;
		return Element_STKM_run_stickman_legacy(playerp, UPDATE_FUNC_SUBCALL_ARGS);
	}
	else
		playerp->legacy = false;

	// Extreme ambient temperature causes harm
	if (sim->aheat_enable && (sim->hv[y/CELL][x/CELL] > (200.0f + 273.14f) || sim->hv[y/CELL][x/CELL] < (-100.0f + 273.15f)))
		parts[i].life--;

	// Pressure causes harm unless we have FAN power
	if (!playerp->fan && (sim->pv[y/CELL][x/CELL] > 10.0f || sim->pv[y/CELL][x/CELL] < -10.0f))
		parts[i].life--;

	// Keep body temperature
	parts[i].temp = 36.6f + 273.15f;

	// Set element power from ctype
	auto &sd = SimulationData::CRef();
	if (!playerp->fan && parts[i].ctype && sd.IsElementOrNone(parts[i].ctype))
		Element_STKM_set_element(sim, playerp, parts[i].ctype);

	// Search for an element near the head
	for (int rx=-2; rx<=2; rx++)
		for (int ry=-2; ry<=2; ry++)
			if (InBounds(x+rx, y+ry) && (rx || ry))
			{
				auto r = pmap[y+ry][x+rx];
				if (!r)
					r = sim->photons[y+ry][x+rx];

				// Check powers given by walls
				if (!r)
				{
					auto bt = sim->bmap[(y+ry)/CELL][(x+rx)/CELL];

					if (bt == WL_FAN)
					{
						playerp->fan = true;
						playerp->elem = PT_DUST;
					}
					else if (bt == WL_EHOLE)
						playerp->rocketBoots = false;
					else if (bt == WL_GRAV)
						playerp->rocketBoots = true;
					else
						continue;
				}

				// PLNT gives +5 HP
				if (TYP(r) == PT_PLNT)
				{
					parts[i].life = std::min(100, parts[i].life+5);
					sim->kill_part(ID(r));
				}
				else if (TYP(r) == PT_GPMP && parts[ID(r)].life != 10)
					playerp->elem = PT_GPMP; // Disabled GPMP gives antigravity power
				else if (playerp->rocketBoots && TYP(r) == PT_PLSM)
					continue; // Ignore PLSM when in rocket boots
				else if (TYP(r) == PT_FIGH)
				{
					// Zombie FIGH
					if (parts[ID(r)].ctype == PT_DCEL && sim->rng.chance(1, 20))
					{
						playerp->elem = PT_DCEL; // Get inftected from zombie FIGH
						if (parts[i].type != PT_FIGH) // Zombies shouldn't harm each other
						{
							parts[i].life -= 5;
							parts[i].vx += float(sim->rng.between(-1, 1))*0.8f;
							parts[i].vy += float(sim->rng.between(-1, 1))*0.8f;
						}
					}

					// Fighters kicking STKM
					if (parts[i].type != PT_FIGH && parts[ID(r)].ctype != PT_DCEL)
					{
						auto fi = parts[ID(r)].tmp;
						if (fi >= 0 && fi < MAX_FIGHTERS)
						{
							playerst* figh = &sim->fighters[(unsigned char)fi];

							if (hypot(parts[i].vx - parts[ID(r)].vx, parts[i].vy - parts[ID(r)].vy) > 1.0f &&
									figh->isJumping && sim->rng.chance(1, 2))
							{
								if (playerp->elem == PT_SHLD1 && playerp->frames > 0)
									playerp->frames--; // Shield takes damage
								else
								{
									parts[i].life -= 1;
									parts[i].vx += parts[ID(r)].vx;
									parts[i].vy += parts[ID(r)].vy;
								}
							}
						}
					}
				}

				Element_STKM_set_element(sim, playerp, TYP(r));
			}

	// Kill STKM when not enough HP
	if (parts[i].life <= 0)
	{
		die(sim, playerp, i);
		return 1;
	}

	if (playerp->elem == PT_SHLD1)
	{
		if (playerp->frames < 0)
			playerp->frames = 0;

		// Recharge the shield
		if (playerp->frames < 10)
			playerp->frames++;

		// Shield center
		Vec2<float> midpoint = (Vec2<float>{float(x), float(y)} + playerp->legs[1] + playerp->legs[3]) * (1/3.0f);
		Vec2<float> center = midpoint - (playerp->downVec*3.0f);
		Vec2<int> cntr{int(center.X), int(center.Y)};

		// Repel particles inside the shield
		for (int rx = -8; rx <= 8; rx++)
			for (int ry = -8; ry <= 8; ry++)
			{
				int cx = cntr.X + rx;
				int cy = cntr.Y + ry;

				if (InBounds(cx, cy) && (cx != x || cy != y) && (rx*rx + ry*ry <= 64))
				{
					auto r = pmap[cy][cx];
					if (!r)
						r = sim->photons[cy][cx];

					float power = float(playerp->frames)/10.0f;

					if (r && !(sd.elements[TYP(r)].Properties & TYPE_SOLID)) {
						parts[ID(r)].vx = restrict_flt(parts[ID(r)].vx + float(isign(rx))*power, -MAX_VELOCITY, MAX_VELOCITY);
						parts[ID(r)].vy = restrict_flt(parts[ID(r)].vy + float(isign(ry))*power, -MAX_VELOCITY, MAX_VELOCITY);
					}
				}
			}
	}

	parts[i].ctype = playerp->elem;

	return Element_STKM_movement(playerp, UPDATE_FUNC_SUBCALL_ARGS);
}

// Legacy feet interaction code
void Element_STKM_interact_legacy(Simulation *sim, playerst *playerp, int i, int x, int y)
{
	auto &sd = SimulationData::CRef();
	auto &elements = sd.elements;
	int r;
	if (x<0 || y<0 || x>=XRES || y>=YRES || !sim->parts[i].type)
		return;
	r = sim->pmap[y][x];
	if (r)
	{
		int damage = 0;
		if (TYP(r)==PT_SPRK && playerp->elem!=PT_LIGH) //If on charge
		{
			damage += sim->rng.between(32, 51);
		}

		if (!sd.IsHeatInsulator(sim->parts[ID(r)]) && ((playerp->elem!=PT_LIGH && sim->parts[ID(r)].temp>=323) || sim->parts[ID(r)].temp<=243) && (!playerp->rocketBoots || TYP(r)!=PT_PLSM))
		{
			damage += 2;
			playerp->accs[1].Y -= 1;
		}

		if (elements[TYP(r)].Properties&PROP_DEADLY)
			switch (TYP(r))
			{
				case PT_ACID:
					damage += 5;
					break;
				default:
					damage++;
					break;
			}

		if (elements[TYP(r)].Properties&PROP_RADIOACTIVE)
			damage++;

		if (damage)
		{
			if (damage > sim->parts[i].life)
			{
				die(sim, playerp, i);
				return;
			}
			sim->parts[i].life -= damage;
		}

		if (TYP(r)==PT_PRTI && sim->parts[i].type)
		{
			int nnx, count=1;//gives rx=0, ry=1 in update_PRTO
			sim->parts[ID(r)].tmp = (int)((sim->parts[ID(r)].temp-73.15f)/100+1);
			if (sim->parts[ID(r)].tmp>=CHANNELS) sim->parts[ID(r)].tmp = CHANNELS-1;
			else if (sim->parts[ID(r)].tmp<0) sim->parts[ID(r)].tmp = 0;
			for (nnx=0; nnx<80; nnx++)
				if (!sim->portalp[sim->parts[ID(r)].tmp][count][nnx].type)
				{
					sim->portalp[sim->parts[ID(r)].tmp][count][nnx] = sim->parts[i];
					sim->kill_part(i);
					//stop new STKM/fighters being created to replace the ones in the portal:
					playerp->spwn = true;
					if (sim->portalp[sim->parts[ID(r)].tmp][count][nnx].type==PT_FIGH)
						sim->fighcount++;
					break;
				}
		}
		if ((TYP(r)==PT_BHOL || TYP(r)==PT_NBHL) && sim->parts[i].type)
		{
			if (!sim->legacy_enable)
			{
				sim->parts[ID(r)].temp = restrict_flt(sim->parts[ID(r)].temp+sim->parts[i].temp/2, MIN_TEMP, MAX_TEMP);
			}
			sim->kill_part(i);
		}
		if ((TYP(r)==PT_VOID || (TYP(r)==PT_PVOD && sim->parts[ID(r)].life==10)) && (!sim->parts[ID(r)].ctype || (sim->parts[ID(r)].ctype==sim->parts[i].type)!=(sim->parts[ID(r)].tmp&1)) && sim->parts[i].type)
		{
			sim->kill_part(i);
		}
	}
}

// Legacy STKM update function, not maintained anymore
int Element_STKM_run_stickman_legacy(playerst *playerp, UPDATE_FUNC_ARGS)
{
	auto &sd = SimulationData::CRef();
	auto &elements = sd.elements;
	int r, rx, ry;
	int t = parts[i].type;
	float dt = 0.9f;///(FPSB*FPSB);  //Delta time in square
	float gvx, gvy;
	float gx, gy, dl, dr;
	float rocketBootsHeadEffect = 0.35f;
	float rocketBootsFeetEffect = 0.15f;
	float rocketBootsHeadEffectV = 0.3f;// stronger acceleration vertically, to counteract gravity
	float rocketBootsFeetEffectV = 0.45f;

	if (!playerp->fan && parts[i].ctype && sd.IsElementOrNone(parts[i].ctype))
		Element_STKM_set_element(sim, playerp, parts[i].ctype);
	playerp->frames++;

	//Temperature handling
	if (parts[i].temp<243)
		parts[i].life -= 1;
	if ((parts[i].temp<309.6f) && (parts[i].temp>=243))
		parts[i].temp += 1;

	//Death
	if (parts[i].life<1 || (sim->pv[y/CELL][x/CELL]>=4.5f && !playerp->fan) ) //If his HP is less than 0 or there is very big wind...
	{
		die(sim, playerp, i);
		return 1;
	}

	// Follow gravity
	sim->GetGravityField(parts[i].x, parts[i].y, 1.0f, 1.0f, gvx, gvy);

	float mvx = gvx;
	float mvy = gvy;
	bool rbLowGrav = false;
	float tmp = fabsf(mvx) > fabsf(mvy)?fabsf(mvx):fabsf(mvy);
	if (tmp < 0.001f)
	{
		rbLowGrav = true;
		mvx = -parts[i].vx;
		mvy = -parts[i].vy;
		tmp = fabsf(mvx) > fabsf(mvy)?fabsf(mvx):fabsf(mvy);
	}
	if (tmp < 0.001f)
	{
		mvx = 0;
		mvy = 1.0f;
		tmp = 1.0f;
	}
	else
		tmp = 1.0f/sqrtf(mvx*mvx+mvy*mvy);
	mvx *= tmp;// scale to a unit vector
	mvy *= tmp;

	Vec2<float> downDir{mvx, mvy}; // Down direction
	Vec2<float> leftDir{-mvy, mvx}; // Above, but rotated 90 clockwise

	if (rbLowGrav)
	{
		rocketBootsHeadEffectV = rocketBootsHeadEffect;
		rocketBootsFeetEffectV = rocketBootsFeetEffect;
	}

	parts[i].vx -= gvx*dt;  //Head up!
	parts[i].vy -= gvy*dt;

	// Verlet integration
	for(int j = 0; j < 4; j+=2)
	{
		verletStep(playerp->legs[j], playerp->legsp[j], playerp->accs[j], dt);
		verletStep(playerp->legs[j+1], playerp->legsp[j+1], playerp->accs[j+1] + Vec2<float>{gvx, gvy}, dt);
	}

	// Reset acceleration to 0
	for(int j = 0; j < 4; j++)
		playerp->accs[j] = Vec2<float>{0, 0};

	gx = (playerp->legs[1].X + playerp->legs[3].X)/2 - gvy;
	gy = (playerp->legs[1].Y + playerp->legs[3].Y)/2 + gvx;
	dl = pow(gx - playerp->legs[1].X, 2) + pow(gy - playerp->legs[1].Y, 2);
	dr = pow(gx - playerp->legs[3].X, 2) + pow(gy - playerp->legs[3].Y, 2);

	// Go left or right
	if (((int)(playerp->comm)&0x01) == 0x01 || ((int)(playerp->comm)&0x02) == 0x02)
	{
		bool moved = false;
		bool goLeft = ((int)(playerp->comm)&0x01) == 0x01;
		auto movingDir = goLeft ? leftDir : -leftDir;
		int movingLeg;

		if (goLeft)
			movingLeg = dl>dr ? 1 : 3;
		else
			movingLeg = dl<dr ? 1 : 3;

		if (VecInBounds(playerp->legs[movingLeg]) && !sim->eval_move(t, int(playerp->legs[movingLeg].X), int(playerp->legs[movingLeg].Y), nullptr))
		{
			playerp->accs[movingLeg] = movingDir*3.0f - downDir*3.0f; // Foot
			playerp->accs[movingLeg-1] = movingDir; // Knee
			moved = true;
		}

		if (!moved && playerp->rocketBoots)
		{
			parts[i].vx += rocketBootsHeadEffect*movingDir.X;
			parts[i].vy += rocketBootsHeadEffect*movingDir.Y;

			playerp->accs[1] += movingDir*rocketBootsFeetEffect;
			playerp->accs[3] += movingDir*rocketBootsFeetEffect;

			// PLSM trail
			for (int leg=0; leg<2; leg++)
			{
				if (leg==1 && (((int)(playerp->comm)&0x02) == 0x02))
					continue;

				int footX = int(playerp->legs[leg*2+1].X), footY = int(playerp->legs[leg*2+1].Y);
				int np = sim->create_part(-1, footX, footY, PT_PLSM);
				if (np>=0)
				{
					parts[np].vx = parts[i].vx - 25.0f*movingDir.X;
					parts[np].vy = parts[i].vy - 25.0f*movingDir.Y;
					parts[np].life += 30;
				}
			}
		}
	}

	if (playerp->rocketBoots && ((int)(playerp->comm)&0x03) == 0x03)
	{
		// Pressing left and right simultaneously with rocket boots on slows the stickman down
		// Particularly useful in zero gravity
		parts[i].vx *= 0.5f;
		parts[i].vy *= 0.5f;
		playerp->accs[0] = Vec2<float>{0, 0};
		playerp->accs[3] = Vec2<float>{0, 0};
	}

	//Jump
	if (((int)(playerp->comm)&0x04) == 0x04)
	{
		if (playerp->rocketBoots)
		{
			parts[i].vx -= rocketBootsHeadEffectV*mvx;
			parts[i].vy -= rocketBootsHeadEffectV*mvy;
			playerp->accs[1] -= downDir*rocketBootsFeetEffectV;
			playerp->accs[3] -= downDir*rocketBootsFeetEffectV;

			// PLSM trail
			for (int leg=0; leg<2; leg++)
			{
				int footX = int(playerp->legs[leg*2+1].X), footY = int(playerp->legs[leg*2+1].Y);
				int np = sim->create_part(-1, footX, footY+1, PT_PLSM);
				if (np>=0)
				{
					parts[np].vx = parts[i].vx+30.0f*downDir.X;
					parts[np].vy = parts[i].vy+30.0f*downDir.Y;
					parts[np].life += 10;
				}
			}
		}
		else if ((VecInBounds(playerp->legs[1]) && !sim->eval_move(t, int(playerp->legs[1].X), int(playerp->legs[1].Y), nullptr)) ||
				 (VecInBounds(playerp->legs[3]) && !sim->eval_move(t, int(playerp->legs[3].X), int(playerp->legs[3].Y), nullptr)))
		{
			parts[i].vx -= 4*downDir.X;
			parts[i].vy -= 4*downDir.Y;
			playerp->accs[1] -= downDir;
			playerp->accs[3] -= downDir;
		}
	}

	// Charge detector wall if a foot inside
	for(int j = 0; j < 2; j++)
	{
		auto foot = playerp->legs[2*j+1];

		if (InCellBounds(int(foot.X+0.5f)/CELL, int(foot.Y+0.5f)/CELL) && sim->bmap[(int)(foot.Y+0.5f)/CELL][(int)(foot.X+0.5f)/CELL]==WL_DETECT)
			sim->set_emap((int)foot.X/CELL, (int)foot.Y/CELL);
	}

	// Searching for particles near head
	for (rx=-2; rx<3; rx++)
		for (ry=-2; ry<3; ry++)
			if (InBounds(x+rx, y+ry) && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					r = sim->photons[y+ry][x+rx];

				if (!r && !sim->bmap[(y+ry)/CELL][(x+rx)/CELL])
					continue;

				Element_STKM_set_element(sim, playerp, TYP(r));
				if (TYP(r) == PT_PLNT && parts[i].life<100) //Plant gives him 5 HP
				{
					if (parts[i].life<=95)
						parts[i].life += 5;
					else
						parts[i].life = 100;
					sim->kill_part(ID(r));
				}

				if (TYP(r) == PT_NEUT)
				{
					if (parts[i].life<=100) parts[i].life -= (102-parts[i].life)/2;
					else parts[i].life = int(parts[i].life * 0.9f);
					sim->kill_part(ID(r));
				}
				if (sim->bmap[(ry+y)/CELL][(rx+x)/CELL]==WL_FAN)
					playerp->fan = true;
				else if (sim->bmap[(ry+y)/CELL][(rx+x)/CELL]==WL_EHOLE)
					playerp->rocketBoots = false;
				else if (sim->bmap[(ry+y)/CELL][(rx+x)/CELL]==WL_GRAV /* && parts[i].type!=PT_FIGH */)
					playerp->rocketBoots = true;
				if (TYP(r)==PT_PRTI)
					Element_STKM_interact_legacy(sim, playerp, i, rx, ry);
				if (!parts[i].type)//STKM_interact may kill STKM
					return 1;
			}

	//Head position
	rx = x + 3*((((int)playerp->pcomm)&0x02) == 0x02) - 3*((((int)playerp->pcomm)&0x01) == 0x01);
	ry = y - 3*(playerp->pcomm == 0);

	//Spawn
	if (((int)(playerp->comm)&0x08) == 0x08)
	{
		ry -= 2 * sim->rng.between(0, 1) + 1;
		r = pmap[ry][rx];
		if (elements[TYP(r)].Properties&TYPE_SOLID)
		{
			sim->create_part(-1, rx, ry, PT_SPRK);
			playerp->frames = 0;
		}
		else
		{
			int np = -1;
			if (playerp->fan)
			{
				for(int j = -4; j < 5; j++)
					for (int k = -4; k < 5; k++)
					{
						int airx = rx + 3*((((int)playerp->pcomm)&0x02) == 0x02) - 3*((((int)playerp->pcomm)&0x01) == 0x01)+j;
						int airy = ry+k;
						sim->pv[airy/CELL][airx/CELL] += 0.03f;
						if (airy + CELL < YRES)
							sim->pv[airy/CELL+1][airx/CELL] += 0.03f;
						if (airx + CELL < XRES)
						{
							sim->pv[airy/CELL][airx/CELL+1] += 0.03f;
							if (airy + CELL < YRES)
								sim->pv[airy/CELL+1][airx/CELL+1] += 0.03f;
						}
					}
			}
			else if (playerp->elem==PT_LIGH && playerp->frames<30)//limit lightning creation rate
				np = -1;
			else
				np = sim->create_part(-1, rx, ry, playerp->elem);
			if ( (np < NPART) && np>=0)
			{
				if (playerp->elem == PT_PHOT)
				{
					int random = abs((sim->rng.between(-1, 1)))*3;
					if (random==0)
					{
						sim->kill_part(np);
					}
					else
					{
						parts[np].vy = 0;
						if (((int)playerp->pcomm)&(0x01|0x02))
							parts[np].vx = float((((((int)playerp->pcomm)&0x02) == 0x02) - (((int)(playerp->pcomm)&0x01) == 0x01))*random);
						else
							parts[np].vx = float(random);
					}
				}
				else if (playerp->elem == PT_LIGH)
				{
					int angle;
					int power = 100;
					if (gvx!=0 || gvy!=0)
						angle = int(atan2(mvx, mvy)*180.0f/std::numbers::pi_v<float>);
					else
						angle = sim->rng.between(0, 359);
					if (((int)playerp->pcomm)&0x01)
						angle += 180;
					if (angle>360)
						angle-=360;
					if (angle<0)
						angle+=360;
					parts[np].tmp = angle;
					parts[np].life = sim->rng.between(0, 1+power/15) + power/7;
					parts[np].temp = parts[np].life*power/2.5;
					parts[np].tmp2 = 1;
				}
				else if (!playerp->fan)
				{
					parts[np].vx -= -mvy*(5*((((int)playerp->pcomm)&0x02) == 0x02) - 5*(((int)(playerp->pcomm)&0x01) == 0x01));
					parts[np].vy -= mvx*(5*((((int)playerp->pcomm)&0x02) == 0x02) - 5*(((int)(playerp->pcomm)&0x01) == 0x01));
					parts[i].vx -= (elements[(int)playerp->elem].Weight*parts[np].vx)/1000;
				}
				playerp->frames = 0;
			}

		}
	}

	// Simulation of joints
	simJointLegacy(playerp->legs[0], playerp->legs[1], 5.0f);
	simJointLegacy(playerp->legs[2], playerp->legs[3], 5.0f);

	Vec2<float> dummyHead{parts[i].x, parts[i].y};
	simJointLegacy(dummyHead, playerp->legs[0], 6.0f);
	parts[i].vx += dummyHead.X - parts[i].x;
	parts[i].vy += dummyHead.Y - parts[i].y;

	dummyHead = Vec2<float>{parts[i].x, parts[i].y};
	simJointLegacy(dummyHead, playerp->legs[2], 6.0f);
	parts[i].vx += dummyHead.X - parts[i].x;
	parts[i].vy += dummyHead.Y - parts[i].y;

	for(int j = 1; j < 4; j += 2)
	{
		if (VecInBounds(playerp->legs[j]) && !sim->eval_move(t, int(playerp->legs[j].X), int(playerp->legs[j].Y), nullptr))
			playerp->legs[j] = playerp->legsp[j];
	}

	// This makes stick man "pop" from obstacles
	for(int j = 1; j < 4; j += 2)
	{
		if (VecInBounds(playerp->legs[j]) && !sim->eval_move(t, int(playerp->legs[j].X), int(playerp->legs[j].Y), nullptr))
		{
			auto swap = playerp->legs[j];
			playerp->legs[j] = playerp->legsp[j];
			playerp->legsp[j] = swap;
		}
	}

	// Keeping legs distance
	for(int j = 0; j < 2; j++)
	{
		if ((pow((playerp->legs[j].X - playerp->legs[j+2].X), 2) + pow((playerp->legs[j].Y-playerp->legs[j+2].Y), 2))<16)
		{
			float tvx, tvy;
			tvx = -gvy;
			tvy = gvx;

			if (tvx || tvy)
			{
				Vec2<float> tv{0.2f*tvx, 0.2f*tvy};
				playerp->accs[j] -= tv;
				playerp->accs[j+2] += tv;
			}
		}
	}

	// If feet touch something
	Element_STKM_interact_legacy(sim, playerp, i, (int)(playerp->legs[1].X+0.5), (int)(playerp->legs[1].Y+0.5));
	Element_STKM_interact_legacy(sim, playerp, i, (int)(playerp->legs[3].X+0.5), (int)(playerp->legs[3].Y+0.5));
	Element_STKM_interact_legacy(sim, playerp, i, (int)(playerp->legs[1].X+0.5), (int)playerp->legs[1].Y);
	Element_STKM_interact_legacy(sim, playerp, i, (int)(playerp->legs[3].X+0.5), (int)playerp->legs[3].Y);

	if (!parts[i].type)
		return 1;

	parts[i].ctype = playerp->elem;
	return 0;
}

void Element_STKM_init_legs(Simulation * sim, playerst *playerp, int i)
{
	float x, y;
	int xi, yi;

	xi = (int)(sim->parts[i].x+0.5f);
	yi = (int)(sim->parts[i].y+0.5f);

	x = xi;
	y = yi;

	// Set default legs positions
	for (int j = 0; j < 4; j+=2)
	{
		float sign = j == 0 ? -1.0f : 1.0f;

		if (sim->parts[i].flags & FLAG_OLDSTKM)
		{
			playerp->legs[j] = Vec2<float>{x+sign*1, y+6};
			playerp->legsp[j] = Vec2<float>{x+sign*1, y+6};

			playerp->legs[j+1] = Vec2<float>{x+sign*3, y+12};
			playerp->legsp[j+1] = Vec2<float>{x+sign*3, y+12};
		}
		else
		{
			playerp->legs[j] = Vec2<float>{x+sign*2, y+5};
			playerp->legsp[j] = Vec2<float>{x+sign*2, y+5};

			playerp->legs[j+1] = Vec2<float>{x+sign*3, y+10};
			playerp->legsp[j+1] = Vec2<float>{x+sign*3, y+10};
		}
	}

	// Default acceleration is zero
	for (int j = 0; j < 4; j++)
		playerp->accs[j] = Vec2<float>{0, 0};

	playerp->downVec = Vec2<float>{0, 1};

	playerp->comm = 0;
	playerp->pcomm = 0;
	playerp->pdir = 1;
	playerp->elem = PT_DUST;
	playerp->frames = 0;
	playerp->gaitState = 0;
	playerp->gaitTimer = 0;
	playerp->rocketBoots = false;
	playerp->fan = false;
	playerp->spwn = true;
	playerp->jumpAllowed = true;
	playerp->isJumping = false;
	playerp->dashAllowed = false;
	playerp->swimCounter = 0;
	playerp->isSwimming = false;
	playerp->legacy = sim->parts[i].flags & FLAG_OLDSTKM;
}

void Element_STKM_set_element(Simulation *sim, playerst *playerp, int element)
{
	auto &sd = SimulationData::CRef();
	auto &elements = sd.elements;

	// This function is also used to set STKM's element with right-click selection where part id isn't available,
	// so we have to pass STKM's legacy state through a .legacy field in the playerst structure
	if (playerp->legacy)
	{ // Old STKM
		if (elements[element].Falldown != 0
				|| elements[element].Properties&(TYPE_GAS | TYPE_LIQUID | TYPE_ENERGY)
				|| element == PT_LOLZ || element == PT_LOVE)
		{
			if (!playerp->rocketBoots || element != PT_PLSM)
			{
				playerp->elem = element;
				playerp->fan = false;
			}
		}
		if (element == PT_TESC || element == PT_LIGH)
		{
			playerp->elem = PT_LIGH;
			playerp->fan = false;
		}
	}
	else
	{ // New STKM
		bool goodElement = false;

		if (element != PT_STKM && element != PT_STKM2 && element != PT_FIGH &&
				elements[element].Properties & (TYPE_PART | TYPE_GAS | TYPE_LIQUID | TYPE_ENERGY))
		{
			// Ignore PLSM when in rocket boots
			if (!playerp->rocketBoots || element != PT_PLSM)
				goodElement = true;
		}
		else if (element == PT_TESC) // LIGH power is set from TESC only
		{
			element = PT_LIGH;
			goodElement = true;
		}
		else if (element == PT_ARAY)
			goodElement = true;
		else if (element == PT_INVIS)
			goodElement = true;
		else if (element == PT_ACEL)
			goodElement = true;
		else if (element == PT_DCEL)
			goodElement = true;
		else if (element == PT_SHLD1)
			goodElement = true;

		// Set elemental power
		if (goodElement)
		{
			playerp->elem = element;
			playerp->fan = false;
		}
	}

}
