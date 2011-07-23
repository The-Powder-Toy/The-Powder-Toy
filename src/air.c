#include <math.h>
#include <air.h>
#include <powder.h>
#include <defines.h>
float kernel[9];

float gravmap[YRES/CELL][XRES/CELL];  //Maps to be used by the main thread
float gravx[YRES/CELL][XRES/CELL];
float gravy[YRES/CELL][XRES/CELL];
float gravp[YRES/CELL][XRES/CELL];
float *gravpf;
float *gravyf;
float *gravxf;
unsigned gravmask[YRES/CELL][XRES/CELL];

float th_ogravmap[YRES/CELL][XRES/CELL]; // Maps to be processed by the gravity thread
float th_gravmap[YRES/CELL][XRES/CELL];
float th_gravx[YRES/CELL][XRES/CELL];
float th_gravy[YRES/CELL][XRES/CELL];
float th_gravp[YRES/CELL][XRES/CELL];
float *th_gravpf;
float *th_gravyf;
float *th_gravxf;

float vx[YRES/CELL][XRES/CELL], ovx[YRES/CELL][XRES/CELL];
float vy[YRES/CELL][XRES/CELL], ovy[YRES/CELL][XRES/CELL];
float pv[YRES/CELL][XRES/CELL], opv[YRES/CELL][XRES/CELL];
unsigned char bmap_blockair[YRES/CELL][XRES/CELL];

float cb_vx[YRES/CELL][XRES/CELL];
float cb_vy[YRES/CELL][XRES/CELL];
float cb_pv[YRES/CELL][XRES/CELL];
float cb_hv[YRES/CELL][XRES/CELL];

float fvx[YRES/CELL][XRES/CELL], fvy[YRES/CELL][XRES/CELL];

float hv[YRES/CELL][XRES/CELL], ohv[YRES/CELL][XRES/CELL]; // For Ambient Heat 

void make_kernel(void) //used for velocity
{
	int i, j;
	float s = 0.0f;
	for (j=-1; j<2; j++)
		for (i=-1; i<2; i++)
		{
			kernel[(i+1)+3*(j+1)] = expf(-2.0f*(i*i+j*j));
			s += kernel[(i+1)+3*(j+1)];
		}
	s = 1.0f / s;
	for (j=-1; j<2; j++)
		for (i=-1; i<2; i++)
			kernel[(i+1)+3*(j+1)] *= s;
}
void update_airh(void)
{
	int x, y, i, j;
	float dh, dx, dy, f, tx, ty;
	for (i=0; i<YRES/CELL; i++) //reduces pressure/velocity on the edges every frame
	{
		hv[i][0] = 295.15f;
		hv[i][1] = 295.15f;
		hv[i][XRES/CELL-3] = 295.15f;
		hv[i][XRES/CELL-2] = 295.15f;
		hv[i][XRES/CELL-1] = 295.15f;
	}
	for (i=0; i<XRES/CELL; i++) //reduces pressure/velocity on the edges every frame
	{
		hv[0][i] = 295.15f;
		hv[1][i] = 295.15f;
		hv[YRES/CELL-3][i] = 295.15f;
		hv[YRES/CELL-2][i] = 295.15f;
		hv[YRES/CELL-1][i] = 295.15f;
	}
	for (y=0; y<YRES/CELL; y++) //update velocity and pressure
	{
		for (x=0; x<XRES/CELL; x++)
		{
			dh = 0.0f;
			dx = 0.0f;
			dy = 0.0f;
			for (j=-1; j<2; j++)
			{
				for (i=-1; i<2; i++)
				{
					if (y+j>0 && y+j<YRES/CELL-2 &&
					        x+i>0 && x+i<XRES/CELL-2 &&
					        bmap[y+j][x+i]!=WL_WALL &&
					        bmap[y+j][x+i]!=WL_WALLELEC &&
					        bmap[y+j][x+i]!=WL_GRAV && 
					        (bmap[y+j][x+i]!=WL_EWALL || emap[y+j][x+i]))
						{
						f = kernel[i+1+(j+1)*3];
						dh += hv[y+j][x+i]*f;
						dx += vx[y+j][x+i]*f;
						dy += vy[y+j][x+i]*f;
					}
					else
					{
						f = kernel[i+1+(j+1)*3];
						dh += hv[y][x]*f;
						dx += vx[y][x]*f;
						dy += vy[y][x]*f;
					}
				}
			}
			tx = x - dx*0.7f;
			ty = y - dy*0.7f;
			i = (int)tx;
			j = (int)ty;
			tx -= i;
			ty -= j;
			if (i>=2 && i<XRES/CELL-3 && j>=2 && j<YRES/CELL-3)
			{
				dh *= 1.0f - AIR_VADV;
				dh += AIR_VADV*(1.0f-tx)*(1.0f-ty)*hv[j][i];
				dh += AIR_VADV*tx*(1.0f-ty)*hv[j][i+1];
				dh += AIR_VADV*(1.0f-tx)*ty*hv[j+1][i];
				dh += AIR_VADV*tx*ty*hv[j+1][i+1];
			}
			if(!gravityMode){ //Vertical gravity only for the time being
				float airdiff = dh-hv[y][x];
				pv[y][x] += airdiff/5000.0f;
				if(airdiff>0)	
					vy[y][x] -= airdiff/5000.0f;
			}
			ohv[y][x] = dh;
		}
	}
	memcpy(hv, ohv, sizeof(hv));
}
void bilinear_interpolation(float *src, float *dst, int sw, int sh, int rw, int rh)
{
	int y, x, fxceil, fyceil;
	float fx, fy, fyc, fxc;
	double intp;
	float tr, tl, br, bl;
	//Bilinear interpolation for upscaling
	for (y=0; y<rh; y++)
		for (x=0; x<rw; x++)
		{
			fx = ((float)x)*((float)sw)/((float)rw);
			fy = ((float)y)*((float)sh)/((float)rh);
			fxc = modf(fx, &intp);
			fyc = modf(fy, &intp);
			fxceil = (int)ceil(fx);
			fyceil = (int)ceil(fy);
			if (fxceil>=sw) fxceil = sw-1;
			if (fyceil>=sh) fyceil = sh-1;
			tr = src[sw*(int)floor(fy)+fxceil];
			tl = src[sw*(int)floor(fy)+(int)floor(fx)];
			br = src[sw*fyceil+fxceil];
			bl = src[sw*fyceil+(int)floor(fx)];
			dst[rw*y+x] = ((tl*(1.0f-fxc))+(tr*(fxc)))*(1.0f-fyc) + ((bl*(1.0f-fxc))+(br*(fxc)))*(fyc);				
		}
}

void update_grav(void)
{
	int x, y, i, j, changed = 0;
	float val, distance;
#ifndef GRAV_DIFF
	//Find any changed cells
	for (i=0; i<YRES/CELL; i++)
	{
		if(changed)
			break;
		for (j=0; j<XRES/CELL; j++)
		{
			if(th_ogravmap[i][j]!=th_gravmap[i][j]){
				changed = 1;
				break;
			}
		}
	}
	if(!changed)
		goto fin;
	memset(th_gravy, 0, sizeof(th_gravy));
	memset(th_gravx, 0, sizeof(th_gravx));
#endif
	for (i = 0; i < YRES / CELL; i++) {
		for (j = 0; j < XRES / CELL; j++) {
#ifdef GRAV_DIFF
			if (th_ogravmap[i][j] != th_gravmap[i][j])
			{
#else
			if (th_gravmap[i][j] > 0.0001f || th_gravmap[i][j]<-0.0001f) //Only calculate with populated or changed cells.
			{
#endif
				for (y = 0; y < YRES / CELL; y++) {
					for (x = 0; x < XRES / CELL; x++) {
						if (x == j && y == i)//Ensure it doesn't calculate with itself
							continue;
						distance = sqrt(pow(j - x, 2) + pow(i - y, 2));
#ifdef GRAV_DIFF
						val = th_gravmap[i][j] - th_ogravmap[i][j];
#else
						val = th_gravmap[i][j];
#endif
						th_gravx[y][x] += M_GRAV * val * (j - x) / pow(distance, 3);
						th_gravy[y][x] += M_GRAV * val * (i - y) / pow(distance, 3);
						th_gravp[y][x] += M_GRAV * val / pow(distance, 2);
					}
				}
			}
		}
	}
	bilinear_interpolation(th_gravy, th_gravyf, XRES/CELL, YRES/CELL, XRES, YRES);
	bilinear_interpolation(th_gravx, th_gravxf, XRES/CELL, YRES/CELL, XRES, YRES);
	bilinear_interpolation(th_gravp, th_gravpf, XRES/CELL, YRES/CELL, XRES, YRES);
fin:
	memcpy(th_ogravmap, th_gravmap, sizeof(th_gravmap));
	memset(th_gravmap, 0, sizeof(th_gravmap));
}
void update_air(void)
{
	int x, y, i, j;
	float dp, dx, dy, f, tx, ty;

	for (y=0; y<YRES/CELL; y++)
		for (x=0; x<XRES/CELL; x++)
		{
			bmap_blockair[y][x] = (bmap[y][x]==WL_WALL || bmap[y][x]==WL_WALLELEC || (bmap[y][x]==WL_EWALL && !emap[y][x]));
		}
	if (airMode != 4) { //airMode 4 is no air/pressure update

		for (i=0; i<YRES/CELL; i++) //reduces pressure/velocity on the edges every frame
		{
			pv[i][0] = pv[i][0]*0.8f;
			pv[i][1] = pv[i][1]*0.8f;
			pv[i][2] = pv[i][2]*0.8f;
			pv[i][XRES/CELL-2] = pv[i][XRES/CELL-2]*0.8f;
			pv[i][XRES/CELL-1] = pv[i][XRES/CELL-1]*0.8f;
			vx[i][0] = vx[i][1]*0.9f;
			vx[i][1] = vx[i][2]*0.9f;
			vx[i][XRES/CELL-2] = vx[i][XRES/CELL-3]*0.9f;
			vx[i][XRES/CELL-1] = vx[i][XRES/CELL-2]*0.9f;
			vy[i][0] = vy[i][1]*0.9f;
			vy[i][1] = vy[i][2]*0.9f;
			vy[i][XRES/CELL-2] = vy[i][XRES/CELL-3]*0.9f;
			vy[i][XRES/CELL-1] = vy[i][XRES/CELL-2]*0.9f;
		}
		for (i=0; i<XRES/CELL; i++) //reduces pressure/velocity on the edges every frame
		{
			pv[0][i] = pv[0][i]*0.8f;
			pv[1][i] = pv[1][i]*0.8f;
			pv[2][i] = pv[2][i]*0.8f;
			pv[YRES/CELL-2][i] = pv[YRES/CELL-2][i]*0.8f;
			pv[YRES/CELL-1][i] = pv[YRES/CELL-1][i]*0.8f;
			vx[0][i] = vx[1][i]*0.9f;
			vx[1][i] = vx[2][i]*0.9f;
			vx[YRES/CELL-2][i] = vx[YRES/CELL-3][i]*0.9f;
			vx[YRES/CELL-1][i] = vx[YRES/CELL-2][i]*0.9f;
			vy[0][i] = vy[1][i]*0.9f;
			vy[1][i] = vy[2][i]*0.9f;
			vy[YRES/CELL-2][i] = vy[YRES/CELL-3][i]*0.9f;
			vy[YRES/CELL-1][i] = vy[YRES/CELL-2][i]*0.9f;
		}

		for (j=1; j<YRES/CELL; j++) //clear some velocities near walls
		{
			for (i=1; i<XRES/CELL; i++)
			{
				if (bmap_blockair[j][i])
				{
					vx[j][i] = 0.0f;
					vx[j][i-1] = 0.0f;
					vy[j][i] = 0.0f;
					vy[j-1][i] = 0.0f;
				}
			}
		}

		for (y=1; y<YRES/CELL; y++) //pressure adjustments from velocity
			for (x=1; x<XRES/CELL; x++)
			{
				dp = 0.0f;
				dp += vx[y][x-1] - vx[y][x];
				dp += vy[y-1][x] - vy[y][x];
				pv[y][x] *= AIR_PLOSS;
				pv[y][x] += dp*AIR_TSTEPP;
			}

		for (y=0; y<YRES/CELL-1; y++) //velocity adjustments from pressure
			for (x=0; x<XRES/CELL-1; x++)
			{
				dx = dy = 0.0f;
				dx += pv[y][x] - pv[y][x+1];
				dy += pv[y][x] - pv[y+1][x];
				vx[y][x] *= AIR_VLOSS;
				vy[y][x] *= AIR_VLOSS;
				vx[y][x] += dx*AIR_TSTEPV;
				vy[y][x] += dy*AIR_TSTEPV;
				if (bmap_blockair[y][x] || bmap_blockair[y][x+1])
					vx[y][x] = 0;
				if (bmap_blockair[y][x] || bmap_blockair[y+1][x])
					vy[y][x] = 0;
			}

		for (y=0; y<YRES/CELL; y++) //update velocity and pressure
			for (x=0; x<XRES/CELL; x++)
			{
				dx = 0.0f;
				dy = 0.0f;
				dp = 0.0f;
				for (j=-1; j<2; j++)
					for (i=-1; i<2; i++)
						if (y+j>0 && y+j<YRES/CELL-1 &&
						        x+i>0 && x+i<XRES/CELL-1 &&
						        !bmap_blockair[y+j][x+i])
						{
							f = kernel[i+1+(j+1)*3];
							dx += vx[y+j][x+i]*f;
							dy += vy[y+j][x+i]*f;
							dp += pv[y+j][x+i]*f;
						}
						else
						{
							f = kernel[i+1+(j+1)*3];
							dx += vx[y][x]*f;
							dy += vy[y][x]*f;
							dp += pv[y][x]*f;
						}

				tx = x - dx*0.7f;
				ty = y - dy*0.7f;
				i = (int)tx;
				j = (int)ty;
				tx -= i;
				ty -= j;
				if (i>=2 && i<XRES/CELL-3 &&
				        j>=2 && j<YRES/CELL-3)
				{
					dx *= 1.0f - AIR_VADV;
					dy *= 1.0f - AIR_VADV;

					dx += AIR_VADV*(1.0f-tx)*(1.0f-ty)*vx[j][i];
					dy += AIR_VADV*(1.0f-tx)*(1.0f-ty)*vy[j][i];

					dx += AIR_VADV*tx*(1.0f-ty)*vx[j][i+1];
					dy += AIR_VADV*tx*(1.0f-ty)*vy[j][i+1];

					dx += AIR_VADV*(1.0f-tx)*ty*vx[j+1][i];
					dy += AIR_VADV*(1.0f-tx)*ty*vy[j+1][i];

					dx += AIR_VADV*tx*ty*vx[j+1][i+1];
					dy += AIR_VADV*tx*ty*vy[j+1][i+1];
				}

				if (bmap[y][x] == WL_FAN)
				{
					dx += fvx[y][x];
					dy += fvy[y][x];
				}
				// pressure/velocity caps
				if (dp > 256.0f) dp = 256.0f;
				if (dp < -256.0f) dp = -256.0f;
				if (dx > 256.0f) dx = 256.0f;
				if (dx < -256.0f) dx = -256.0f;
				if (dy > 256.0f) dy = 256.0f;
				if (dy < -256.0f) dy = -256.0f;


				switch (airMode)
				{
				default:
				case 0:  //Default
					break;
				case 1:  //0 Pressure
					dp = 0.0f;
					break;
				case 2:  //0 Velocity
					dx = 0.0f;
					dy = 0.0f;
					break;
				case 3: //0 Air
					dx = 0.0f;
					dy = 0.0f;
					dp = 0.0f;
					break;
				case 4: //No Update
					break;
				}

				ovx[y][x] = dx;
				ovy[y][x] = dy;
				opv[y][x] = dp;
			}
		memcpy(vx, ovx, sizeof(vx));
		memcpy(vy, ovy, sizeof(vy));
		memcpy(pv, opv, sizeof(pv));
	}
}
