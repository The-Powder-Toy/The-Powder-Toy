#include <element.h>

int update_BOX(UPDATE_FUNC_ARGS) {
	int r, rx, ry, rt;
	float pp, d;
	float dt = 0.9;///(FPSB*FPSB);  //Delta time in square
	//Tempirature handling
	if (parts[i].temp<243)
		parts[i].life -= 1;
	if ((parts[i].temp<309.6f) && (parts[i].temp>=243))
		parts[i].temp += 1;


	parts[i].vy += -0.7*dt;  //Head up!

	//Verlet integration
	pp = 2*box[3]-box[5]+box[19]*dt*dt;;
	box[5] = box[3];
	box[3] = pp;
	pp = 2*box[4]-box[6]+box[20]*dt*dt;;
	box[6] = box[4];
	box[4] = pp;

	pp = 2*box[7]-box[9]+box[21]*dt*dt;;
	box[9] = box[7];
	box[7] = pp;
	pp = 2*box[8]-box[10]+(box[22]+1)*dt*dt;;
	box[10] = box[8];
	box[8] = pp;

	pp = 2*box[11]-box[13]+box[23]*dt*dt;;
	box[13] = box[11];
	box[11] = pp;
	pp = 2*box[12]-box[14]+box[24]*dt*dt;;
	box[14] = box[12];
	box[12] = pp;

	pp = 2*box[15]-box[17]+box[25]*dt*dt;;
	box[17] = box[15];
	box[15] = pp;
	pp = 2*box[16]-box[18]+(box[26]+1)*dt*dt;;
	box[18] = box[16];
	box[16] = pp;

	//Setting acceleration to 0
	box[19] = 0;
	box[20] = 0;

	box[21] = 0;
	box[22] = 0;

	box[23] = 0;
	box[24] = 0;

	box[25] = 0;
	box[26] = 0;

	//Simulation of joints
	d = 25/(pow((box[3]-box[7]), 2) + pow((box[4]-box[8]), 2)+25) - 0.5;  //Fast distance
	box[7] -= (box[3]-box[7])*d;
	box[8] -= (box[4]-box[8])*d;
	box[3] += (box[3]-box[7])*d;
	box[4] += (box[4]-box[8])*d;

	d = 25/(pow((box[11]-box[15]), 2) + pow((box[12]-box[16]), 2)+25) - 0.5;
	box[15] -= (box[11]-box[15])*d;
	box[16] -= (box[12]-box[16])*d;
	box[11] += (box[11]-box[15])*d;
	box[12] += (box[12]-box[16])*d;

	d = 36/(pow((box[3]-parts[i].x), 2) + pow((box[4]-parts[i].y), 2)+36) - 0.5;
	parts[i].vx -= (box[3]-parts[i].x)*d;
	parts[i].vy -= (box[4]-parts[i].y)*d;
	box[3] += (box[3]-parts[i].x)*d;
	box[4] += (box[4]-parts[i].y)*d;

	d = 36/(pow((box[11]-parts[i].x), 2) + pow((box[12]-parts[i].y), 2)+36) - 0.5;
	parts[i].vx -= (box[11]-parts[i].x)*d;
	parts[i].vy -= (box[12]-parts[i].y)*d;
	box[11] += (box[11]-parts[i].x)*d;
	box[12] += (box[12]-parts[i].y)*d;

	//Side collisions checking
	for (rx = -3; rx <= 3; rx++)
	{
		r = pmap[(int)(box[16]-2)][(int)(box[15]+rx)];
		if (r && ptypes[r&0xFF].state != ST_GAS && ptypes[r&0xFF].state != ST_LIQUID)
			box[15] -= rx;

		r = pmap[(int)(box[8]-2)][(int)(box[7]+rx)];
		if (r && ptypes[r&0xFF].state != ST_GAS && ptypes[r&0xFF].state != ST_LIQUID)
			box[7] -= rx;
	}

	r = pmap[(int)(box[16]+0.5)][(int)(box[15]+0.5)];
	if ((r&0xFF)==PT_SPRK && r && (r>>8)<NPART) //If on charge
	{
		parts[i].life -= (int)(rand()/1000)+38;
	}

	return 0;
}
