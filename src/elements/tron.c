#include <element.h>

/* TRON element is meant to resemble a tron bike (or worm) moving around and trying to avoid obstacles itself.
 * It has four direction each turn to choose from, 0 (left) 1 (up) 2 (right) 3 (down).
 * Each turn has a small random chance to randomly turn one way (so it doesn't do the exact same thing in a large room)
 * If the place it wants to move isn't a barrier, it will try and 'see' infront of itself to determine its safety.
 * For now the tron can only see its own body length in pixels ahead of itself (and around corners)
 *  - - - - - - - - - -
 *  - - - - + - - - - -
 *  - - - + + + - - - -
 *  - - +<--+-->+ - - -
 *  - +<----+---->+ - -
 *  - - - - H - - - - -
 * Where H is the head with tail length 4, it checks the + area to see if it can hit any of the edges, then it is called safe, or picks the biggest area if none safe.
 * .tmp bit values: 0 tail , 1 head , 2 no tail growth , 4-8 is direction , 16 is wait flag
 * .tmp2 is tail length (gets longer every few hundred frames)
 * .life is the timer that kills the end of the tail (the head uses life for how often it grows longer)
 */
#define TRON_HEAD 1
#define TRON_NOGROW 2
#define TRON_WAIT 16 //it was just created, so WAIT a frame
#define TRON_NODIE 32
int tron_rx[4] = {-1, 0, 1, 0};
int tron_ry[4] = { 0,-1, 0, 1};
int new_tronhead(int x, int y, int i, int direction)
{
	int np = create_part(-1, x , y ,PT_TRON);
	if (np==-1)
		return -1;
	if (parts[i].life >= 100) // increase tail length
	{
		parts[i].tmp2++;
		parts[i].life = 5;
	}
	//give new head our properties
	parts[np].tmp = 1 | direction<<2 | parts[i].tmp&(TRON_NOGROW|TRON_NODIE);
	if (np > i)
		parts[np].tmp |= TRON_WAIT;
	
	parts[np].tmp2 = parts[i].tmp2;
	parts[np].life = parts[i].life + 2;
	parts[np].dcolour = parts[i].dcolour;
	return 1;
}
int trymovetron(int x, int y, int dir, int i, int len)
{
	int k,j,r,rx,ry,tx,ty,count;
	count = 0;
	rx = x;
	ry = y;
	for (k = 1; k <= len; k ++)
	{
		rx += tron_rx[dir];
		ry += tron_ry[dir];
		r = pmap[ry][rx];
		if (!r && !bmap[(ry)/CELL][(rx)/CELL] && ry > CELL && rx > CELL && ry < YRES-CELL && rx < XRES-CELL)
		{
			count++;
			for (tx = rx - tron_ry[dir] , ty = ry - tron_rx[dir], j=1; abs(tx-rx) < (len-k) && abs(ty-ry) < (len-k); tx-=tron_ry[dir],ty-=tron_rx[dir],j++)
			{
				r = pmap[ty][tx];
				if (!r && !bmap[(ty)/CELL][(tx)/CELL] && ry > CELL && rx > CELL && ry < YRES-CELL && rx < XRES-CELL)
				{
					if (j == (len-k))//there is a safe path, so we can break out
						return len+1;
					count++;
				}
				else //we hit a block so no need to check farther here
					break;
			}
			for (tx = rx + tron_ry[dir] , ty = ry + tron_rx[dir], j=1; abs(tx-rx) < (len-k) && abs(ty-ry) < (len-k); tx+=tron_ry[dir],ty+=tron_rx[dir],j++)
			{
				r = pmap[ty][tx];
				if (!r && !bmap[(ty)/CELL][(tx)/CELL] && ry > CELL && rx > CELL && ry < YRES-CELL && rx < XRES-CELL)
				{
					if (j == (len-k))
						return len+1;
					count++;
				}
				else
					break;
			}
		}
		else //a block infront, no need to continue
			break;
	}
	return count;
}
int update_TRON(UPDATE_FUNC_ARGS) {
	int r, rx, ry, np;
	if (parts[i].tmp&TRON_WAIT)
	{
		parts[i].tmp -= TRON_WAIT;
		return 0;
	}
	if (parts[i].tmp&TRON_HEAD)
	{
		int firstdircheck = 0,seconddir,seconddircheck = 0,lastdir,lastdircheck = 0;
		int direction = (parts[i].tmp>>2 & 0x3);
		int originaldir = direction;

		//random turn
		int random = rand()%340;
		if (random==1 || random==3)
		{
			//randomly turn left(3) or right(1)
			direction = (direction + random)%4;
		}
		
		//check infront
		//do sight check
		firstdircheck = trymovetron(x,y,direction,i,parts[i].tmp2);
		if (firstdircheck < parts[i].tmp2)
		{
			if (originaldir != direction) //if we just tried a random turn, don't pick random again
			{
				seconddir = originaldir;
				lastdir = (direction + 2)%4;
			}
			else
			{
				seconddir = (direction + ((rand()%2)*2)+1)% 4;
				lastdir = (seconddir + 2)%4;
			}
			seconddircheck = trymovetron(x,y,seconddir,i,parts[i].tmp2);
			lastdircheck = trymovetron(x,y,lastdir,i,parts[i].tmp2);
		}
		//find the best move
		if (seconddircheck > firstdircheck)
			direction = seconddir;
		if (lastdircheck > seconddircheck && lastdircheck > firstdircheck)
			direction = lastdir;
		//now try making new head, even if it fails
		if (new_tronhead(x + tron_rx[direction],y + tron_ry[direction],i,direction) == -1)
		{
			//ohgod crash, <sparkle effect start here>
			//trigger tail death for TRON_NODIE, or is that mode even needed? just set a high tail length(but it still won't start dying when it crashes)
		}

		//set own life and clear .tmp (it dies if it can't move anyway)
		parts[i].life = parts[i].tmp2;
		parts[i].tmp = 0;
	}
	else // fade tail deco, or prevent tail from dieing
	{
		if (parts[i].tmp&TRON_NODIE)
			parts[i].life++;
		parts[i].dcolour =  clamp_flt((float)parts[i].life/(float)parts[i].tmp2,0,1.0f) << 24 |  parts[i].dcolour&0x00FFFFFF;
	}
	
	return 0;
}
