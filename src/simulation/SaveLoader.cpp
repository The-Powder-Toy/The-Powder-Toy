/*
 * SaveLoader.cpp
 *
 *  Created on: Jan 26, 2012
 *      Author: Simon
 */

#include <bzlib.h>
#include <cmath>
#include "Air.h"
#include "SaveLoader.h"
#include "bson/BSON.h"

//!TODO: enum for LoadSave return

int SaveLoader::Info(unsigned char * data, int dataLength, int & width, int & height)
{
	unsigned char * saveData = data;
	if (dataLength<16)
	{
		return 1;
	}
	if(saveData[0] == 'O' && saveData[1] == 'P' && saveData[2] == 'S')
	{
		return OPSInfo(data, dataLength, width, height);
	}
	else if((saveData[0]==0x66 && saveData[1]==0x75 && saveData[2]==0x43) || (saveData[0]==0x50 && saveData[1]==0x53 && saveData[2]==0x76))
	{
		return PSVInfo(data, dataLength, width, height);
	}
	return 1;
}

int SaveLoader::Load(unsigned char * data, int dataLength, Simulation * sim, bool replace, int x, int y)
{
	unsigned char * saveData = data;
	if (dataLength<16)
	{
		return 1;
	}
	if(saveData[0] == 'O' && saveData[1] == 'P' && saveData[2] == 'S')
	{
		return OPSLoad(data, dataLength, sim, replace, x, y);
	}
	else if((saveData[0]==0x66 && saveData[1]==0x75 && saveData[2]==0x43) || (saveData[0]==0x50 && saveData[1]==0x53 && saveData[2]==0x76))
	{
		return PSVLoad(data, dataLength, sim, replace, x, y);
	}
	return 1;
}

unsigned char * SaveLoader::Build(int & dataLength, Simulation * sim, int orig_x0, int orig_y0, int orig_w, int orig_h)
{
	unsigned char * temp = OPSBuild(dataLength, sim, orig_x0, orig_y0, orig_w, orig_h);
	if(!temp)
		temp = PSVBuild(dataLength, sim, orig_x0, orig_y0, orig_w, orig_h);
	return temp;
}

int SaveLoader::OPSInfo(unsigned char * data, int dataLength, int & width, int & height)
{
	return 2;
}


int SaveLoader::OPSLoad(unsigned char * data, int dataLength, Simulation * sim, bool replace, int x0, int y0)
{
	Particle *partsptr = sim->parts;
	unsigned char * inputData = data, *bsonData = NULL, *partsData = NULL, *partsPosData = NULL, *fanData = NULL, *wallData = NULL;
	unsigned int inputDataLen = dataLength, bsonDataLen = 0, partsDataLen, partsPosDataLen, fanDataLen, wallDataLen;
	int i, freeIndicesCount, x, y, returnCode = 0, j;
	int *freeIndices = NULL;
	int blockX, blockY, blockW, blockH, fullX, fullY, fullW, fullH;
	bson b;
	bson_iterator iter;

	//Block sizes
	blockX = x0/CELL;
	blockY = y0/CELL;
	blockW = inputData[6];
	blockH = inputData[7];

	//Full size, normalised
	fullX = blockX*CELL;
	fullY = blockY*CELL;
	fullW = blockW*CELL;
	fullH = blockH*CELL;

	//From newer version
	if(inputData[4] > SAVE_VERSION)
	{
		fprintf(stderr, "Save from newer version\n");
		return 2;
	}

	//Incompatible cell size
	if(inputData[5] > CELL)
	{
		fprintf(stderr, "Cell size mismatch\n");
		return 1;
	}

	//Too large/off screen
	if(blockX+blockW > XRES/CELL || blockY+blockH > YRES/CELL)
	{
		fprintf(stderr, "Save too large\n");
		return 1;
	}

	bsonDataLen = ((unsigned)inputData[8]);
	bsonDataLen |= ((unsigned)inputData[9]) << 8;
	bsonDataLen |= ((unsigned)inputData[10]) << 16;
	bsonDataLen |= ((unsigned)inputData[11]) << 24;

	bsonData = (unsigned char*)malloc(bsonDataLen+1);
	if(!bsonData)
	{
		fprintf(stderr, "Internal error while parsing save: could not allocate buffer\n");
		return 3;
	}
	//Make sure bsonData is null terminated, since all string functions need null terminated strings
	//(bson_iterator_key returns a pointer into bsonData, which is then used with strcmp)
	bsonData[bsonDataLen] = 0;

	if (BZ2_bzBuffToBuffDecompress((char*)bsonData, &bsonDataLen, (char*)(inputData+12), inputDataLen-12, 0, 0))
	{
		fprintf(stderr, "Unable to decompress\n");
		return 1;
	}

	if(replace)
	{
		//Remove everything
		sim->clear_sim();
	}

	bson_init_data(&b, (char*)bsonData);
	bson_iterator_init(&iter, &b);

	std::vector<sign> tempSigns;

	while(bson_iterator_next(&iter))
	{
		if(strcmp(bson_iterator_key(&iter), "signs")==0)
		{
			if(bson_iterator_type(&iter)==BSON_ARRAY)
			{
				bson_iterator subiter;
				bson_iterator_subiterator(&iter, &subiter);
				while(bson_iterator_next(&subiter))
				{
					if(strcmp(bson_iterator_key(&subiter), "sign")==0)
					{
						if(bson_iterator_type(&subiter)==BSON_OBJECT)
						{
							bson_iterator signiter;
							bson_iterator_subiterator(&subiter, &signiter);

							sign tempSign("", 0, 0, sign::Left);
							while(bson_iterator_next(&signiter))
							{
								if(strcmp(bson_iterator_key(&signiter), "text")==0 && bson_iterator_type(&signiter)==BSON_STRING)
								{
									tempSign.text = bson_iterator_string(&signiter);
									clean_text((char*)tempSign.text.c_str(), 158-14);
								}
								else if(strcmp(bson_iterator_key(&signiter), "justification")==0 && bson_iterator_type(&signiter)==BSON_INT)
								{
									tempSign.ju = (sign::Justification)bson_iterator_int(&signiter);
								}
								else if(strcmp(bson_iterator_key(&signiter), "x")==0 && bson_iterator_type(&signiter)==BSON_INT)
								{
									tempSign.x = bson_iterator_int(&signiter)+fullX;
								}
								else if(strcmp(bson_iterator_key(&signiter), "y")==0 && bson_iterator_type(&signiter)==BSON_INT)
								{
									tempSign.y = bson_iterator_int(&signiter)+fullY;
								}
								else
								{
									fprintf(stderr, "Unknown sign property %s\n", bson_iterator_key(&signiter));
								}
							}
							tempSigns.push_back(tempSign);
						}
						else
						{
							fprintf(stderr, "Wrong type for %s\n", bson_iterator_key(&subiter));
						}
					}
				}
			}
			else
			{
				fprintf(stderr, "Wrong type for %s\n", bson_iterator_key(&iter));
			}
		}
		else if(strcmp(bson_iterator_key(&iter), "parts")==0)
		{
			if(bson_iterator_type(&iter)==BSON_BINDATA && ((unsigned char)bson_iterator_bin_type(&iter))==BSON_BIN_USER && (partsDataLen = bson_iterator_bin_len(&iter)) > 0)
			{
				partsData = (unsigned char*)bson_iterator_bin_data(&iter);
			}
			else
			{
				fprintf(stderr, "Invalid datatype of particle data: %d[%d] %d[%d] %d[%d]\n", bson_iterator_type(&iter), bson_iterator_type(&iter)==BSON_BINDATA, (unsigned char)bson_iterator_bin_type(&iter), ((unsigned char)bson_iterator_bin_type(&iter))==BSON_BIN_USER, bson_iterator_bin_len(&iter), bson_iterator_bin_len(&iter)>0);
			}
		}
		if(strcmp(bson_iterator_key(&iter), "partsPos")==0)
		{
			if(bson_iterator_type(&iter)==BSON_BINDATA && ((unsigned char)bson_iterator_bin_type(&iter))==BSON_BIN_USER && (partsPosDataLen = bson_iterator_bin_len(&iter)) > 0)
			{
				partsPosData = (unsigned char*)bson_iterator_bin_data(&iter);
			}
			else
			{
				fprintf(stderr, "Invalid datatype of particle position data: %d[%d] %d[%d] %d[%d]\n", bson_iterator_type(&iter), bson_iterator_type(&iter)==BSON_BINDATA, (unsigned char)bson_iterator_bin_type(&iter), ((unsigned char)bson_iterator_bin_type(&iter))==BSON_BIN_USER, bson_iterator_bin_len(&iter), bson_iterator_bin_len(&iter)>0);
			}
		}
		else if(strcmp(bson_iterator_key(&iter), "wallMap")==0)
		{
			if(bson_iterator_type(&iter)==BSON_BINDATA && ((unsigned char)bson_iterator_bin_type(&iter))==BSON_BIN_USER && (wallDataLen = bson_iterator_bin_len(&iter)) > 0)
			{
				wallData = (unsigned char*)bson_iterator_bin_data(&iter);
			}
			else
			{
				fprintf(stderr, "Invalid datatype of wall data: %d[%d] %d[%d] %d[%d]\n", bson_iterator_type(&iter), bson_iterator_type(&iter)==BSON_BINDATA, (unsigned char)bson_iterator_bin_type(&iter), ((unsigned char)bson_iterator_bin_type(&iter))==BSON_BIN_USER, bson_iterator_bin_len(&iter), bson_iterator_bin_len(&iter)>0);
			}
		}
		else if(strcmp(bson_iterator_key(&iter), "fanMap")==0)
		{
			if(bson_iterator_type(&iter)==BSON_BINDATA && ((unsigned char)bson_iterator_bin_type(&iter))==BSON_BIN_USER && (fanDataLen = bson_iterator_bin_len(&iter)) > 0)
			{
				fanData = (unsigned char*)bson_iterator_bin_data(&iter);
			}
			else
			{
				fprintf(stderr, "Invalid datatype of fan data: %d[%d] %d[%d] %d[%d]\n", bson_iterator_type(&iter), bson_iterator_type(&iter)==BSON_BINDATA, (unsigned char)bson_iterator_bin_type(&iter), ((unsigned char)bson_iterator_bin_type(&iter))==BSON_BIN_USER, bson_iterator_bin_len(&iter), bson_iterator_bin_len(&iter)>0);
			}
		}
		else if(strcmp(bson_iterator_key(&iter), "legacyEnable")==0 && replace)
		{
			if(bson_iterator_type(&iter)==BSON_BOOL)
			{
				sim->legacy_enable = bson_iterator_bool(&iter);
			}
			else
			{
				fprintf(stderr, "Wrong type for %s\n", bson_iterator_key(&iter));
			}
		}
		else if(strcmp(bson_iterator_key(&iter), "gravityEnable")==0 && replace)
		{
			if(bson_iterator_type(&iter)==BSON_BOOL)
			{
				bool tempGrav = sim->ngrav_enable;
				tempGrav = bson_iterator_bool(&iter);
#ifndef RENDERER
				//Change the gravity state
				if(sim->ngrav_enable != tempGrav)
				{
					if(tempGrav)
						sim->grav->start_grav_async();
					else
						sim->grav->stop_grav_async();
				}
#endif
			}
			else
			{
				fprintf(stderr, "Wrong type for %s\n", bson_iterator_key(&iter));
			}
		}
		else if(strcmp(bson_iterator_key(&iter), "waterEEnabled")==0 && replace)
		{
			if(bson_iterator_type(&iter)==BSON_BOOL)
			{
				sim->water_equal_test = bson_iterator_bool(&iter);
			}
			else
			{
				fprintf(stderr, "Wrong type for %s\n", bson_iterator_key(&iter));
			}
		}
		else if(strcmp(bson_iterator_key(&iter), "paused")==0 && !sim->sys_pause)
		{
			if(bson_iterator_type(&iter)==BSON_BOOL)
			{
				sim->sys_pause = bson_iterator_bool(&iter);
			}
			else
			{
				fprintf(stderr, "Wrong type for %s\n", bson_iterator_key(&iter));
			}
		}
		else if(strcmp(bson_iterator_key(&iter), "gravityMode")==0 && replace)
		{
			if(bson_iterator_type(&iter)==BSON_INT)
			{
				sim->gravityMode = bson_iterator_int(&iter);
			}
			else
			{
				fprintf(stderr, "Wrong type for %s\n", bson_iterator_key(&iter));
			}
		}
		else if(strcmp(bson_iterator_key(&iter), "airMode")==0 && replace)
		{
			if(bson_iterator_type(&iter)==BSON_INT)
			{
				sim->air->airMode = bson_iterator_int(&iter);
			}
			else
			{
				fprintf(stderr, "Wrong type for %s\n", bson_iterator_key(&iter));
			}
		}
		/*else if((strcmp(bson_iterator_key(&iter), "leftSelectedElement")==0 || strcmp(bson_iterator_key(&iter), "rightSelectedElement")) && replace)
		{
			if(bson_iterator_type(&iter)==BSON_INT && bson_iterator_int(&iter) > 0 && bson_iterator_int(&iter) < PT_NUM)
			{
				if(bson_iterator_key(&iter)[0] == 'l')
				{
					sl = bson_iterator_int(&iter);
				}
				else
				{
					sr = bson_iterator_int(&iter);
				}
			}
			else
			{
				fprintf(stderr, "Wrong type for %s\n", bson_iterator_key(&iter));
			}
		}*/
		/*else if(strcmp(bson_iterator_key(&iter), "activeMenu")==0 && replace)
		{
			if(bson_iterator_type(&iter)==BSON_INT && bson_iterator_int(&iter) > 0 && bson_iterator_int(&iter) < SC_TOTAL && msections[bson_iterator_int(&iter)].doshow)
			{
				active_menu = bson_iterator_int(&iter);
			}
			else
			{
				fprintf(stderr, "Wrong value for %s\n", bson_iterator_key(&iter));
			}
		}*/
	}

	//Read wall and fan data
	if(wallData)
	{
		j = 0;
		if(blockW * blockH > wallDataLen)
		{
			fprintf(stderr, "Not enough wall data\n");
			goto fail;
		}
		for(x = 0; x < blockW; x++)
		{
			for(y = 0; y < blockH; y++)
			{
				if (wallData[y*blockW+x])
					sim->bmap[blockY+y][blockX+x] = wallData[y*blockW+x];
				if (wallData[y*blockW+x] == WL_FAN && fanData)
				{
					if(j+1 >= fanDataLen)
					{
						fprintf(stderr, "Not enough fan data\n");
					}
					sim->fvx[blockY+y][blockX+x] = (fanData[j++]-127.0f)/64.0f;
					sim->fvy[blockY+y][blockX+x] = (fanData[j++]-127.0f)/64.0f;
				}
			}
		}
	}

	//Read particle data
	if(partsData && partsPosData)
	{
		int newIndex = 0, fieldDescriptor, tempTemp;
		int posCount, posTotal, partsPosDataIndex = 0;
		int saved_x, saved_y;
		int freeIndicesIndex = 0;
		if(fullW * fullH * 3 > partsPosDataLen)
		{
			fprintf(stderr, "Not enough particle position data\n");
			goto fail;
		}
		sim->parts_lastActiveIndex = NPART-1;
		freeIndicesCount = 0;
		freeIndices = (int*)calloc(sizeof(int), NPART);
		for (i = 0; i<NPART; i++)
		{
			//Ensure ALL parts (even photons) are in the pmap so we can overwrite, keep a track of indices we can use
			if (partsptr[i].type)
			{
				x = (int)(partsptr[i].x+0.5f);
				y = (int)(partsptr[i].y+0.5f);
				sim->pmap[y][x] = (i<<8)|1;
			}
			else
				freeIndices[freeIndicesCount++] = i;
		}
		i = 0;
		for (saved_y=0; saved_y<fullH; saved_y++)
		{
			for (saved_x=0; saved_x<fullW; saved_x++)
			{
				//Read total number of particles at this position
				posTotal = 0;
				posTotal |= partsPosData[partsPosDataIndex++]<<16;
				posTotal |= partsPosData[partsPosDataIndex++]<<8;
				posTotal |= partsPosData[partsPosDataIndex++];
				//Put the next posTotal particles at this position
				for (posCount=0; posCount<posTotal; posCount++)
				{
					//i+3 because we have 4 bytes of required fields (type (1), descriptor (2), temp (1))
					if (i+3 >= partsDataLen)
						goto fail;
					x = saved_x + fullX;
					y = saved_y + fullY;
					fieldDescriptor = partsData[i+1];
					fieldDescriptor |= partsData[i+2] << 8;
					if(x >= XRES || x < 0 || y >= YRES || y < 0)
					{
						fprintf(stderr, "Out of range [%d]: %d %d, [%d, %d], [%d, %d]\n", i, x, y, (unsigned)partsData[i+1], (unsigned)partsData[i+2], (unsigned)partsData[i+3], (unsigned)partsData[i+4]);
						goto fail;
					}
					if(partsData[i] >= PT_NUM)
						partsData[i] = PT_DMND;	//Replace all invalid elements with diamond
					if(sim->pmap[y][x])
					{
						//Replace existing particle or allocated block
						newIndex = sim->pmap[y][x]>>8;
					}
					else if(freeIndicesIndex<freeIndicesCount)
					{
						//Create new particle
						newIndex = freeIndices[freeIndicesIndex++];
					}
					else
					{
						//Nowhere to put new particle, tpt is sad :(
						break;
					}
					if(newIndex < 0 || newIndex >= NPART)
						goto fail;

					//Clear the particle, ready for our new properties
					memset(&(partsptr[newIndex]), 0, sizeof(Particle));

					//Required fields
					partsptr[newIndex].type = partsData[i];
					partsptr[newIndex].x = x;
					partsptr[newIndex].y = y;
					i+=3;

					//Read temp
					if(fieldDescriptor & 0x01)
					{
						//Full 16bit int
						tempTemp = partsData[i++];
						tempTemp |= (((unsigned)partsData[i++]) << 8);
						partsptr[newIndex].temp = tempTemp;
					}
					else
					{
						//1 Byte room temp offset
						tempTemp = (char)partsData[i++];
						partsptr[newIndex].temp = tempTemp+294.15f;
					}

					//Read life
					if(fieldDescriptor & 0x02)
					{
						if(i >= partsDataLen) goto fail;
						partsptr[newIndex].life = partsData[i++];
						//Read 2nd byte
						if(fieldDescriptor & 0x04)
						{
							if(i >= partsDataLen) goto fail;
							partsptr[newIndex].life |= (((unsigned)partsData[i++]) << 8);
						}
					}

					//Read tmp
					if(fieldDescriptor & 0x08)
					{
						if(i >= partsDataLen) goto fail;
						partsptr[newIndex].tmp = partsData[i++];
						//Read 2nd byte
						if(fieldDescriptor & 0x10)
						{
							if(i >= partsDataLen) goto fail;
							partsptr[newIndex].tmp |= (((unsigned)partsData[i++]) << 8);
						}
					}

					//Read ctype
					if(fieldDescriptor & 0x20)
					{
						if(i >= partsDataLen) goto fail;
						partsptr[newIndex].ctype = partsData[i++];
						//Read additional bytes
						if(fieldDescriptor & 0x200)
						{
							if(i+2 >= partsDataLen) goto fail;
							partsptr[newIndex].ctype |= (((unsigned)partsData[i++]) << 24);
							partsptr[newIndex].ctype |= (((unsigned)partsData[i++]) << 16);
							partsptr[newIndex].ctype |= (((unsigned)partsData[i++]) << 8);
						}
					}

					//Read dcolour
					if(fieldDescriptor & 0x40)
					{
						if(i+3 >= partsDataLen) goto fail;
						partsptr[newIndex].dcolour = (((unsigned)partsData[i++]) << 24);
						partsptr[newIndex].dcolour |= (((unsigned)partsData[i++]) << 16);
						partsptr[newIndex].dcolour |= (((unsigned)partsData[i++]) << 8);
						partsptr[newIndex].dcolour |= ((unsigned)partsData[i++]);
					}

					//Read vx
					if(fieldDescriptor & 0x80)
					{
						if(i >= partsDataLen) goto fail;
						partsptr[newIndex].vx = (partsData[i++]-127.0f)/16.0f;
					}

					//Read vy
					if(fieldDescriptor & 0x100)
					{
						if(i >= partsDataLen) goto fail;
						partsptr[newIndex].vy = (partsData[i++]-127.0f)/16.0f;
					}

					//Read tmp2
					if(fieldDescriptor & 0x400)
					{
						if(i >= partsDataLen) goto fail;
						partsptr[newIndex].tmp2 = partsData[i++];
					}

					if ((sim->player.spwn == 1 && partsptr[newIndex].type==PT_STKM) || (sim->player2.spwn == 1 && partsptr[newIndex].type==PT_STKM2))
					{
						partsptr[newIndex].type = PT_NONE;
					}
					else if (partsptr[newIndex].type == PT_STKM)
					{
						//STKM_init_legs(&player, newIndex);
						sim->player.spwn = 1;
						sim->player.elem = PT_DUST;
					}
					else if (partsptr[newIndex].type == PT_STKM2)
					{
						//STKM_init_legs(&player2, newIndex);
						sim->player2.spwn = 1;
						sim->player2.elem = PT_DUST;
					}
					else if (partsptr[newIndex].type == PT_FIGH)
					{
						//TODO: 100 should be replaced with a macro
						unsigned char fcount = 0;
						while (fcount < 100 && fcount < (sim->fighcount+1) && sim->fighters[fcount].spwn==1) fcount++;
						if (fcount < 100 && sim->fighters[fcount].spwn==0)
						{
							partsptr[newIndex].tmp = fcount;
							sim->fighters[fcount].spwn = 1;
							sim->fighters[fcount].elem = PT_DUST;
							sim->fighcount++;
							//STKM_init_legs(&(sim->fighters[sim->fcount]), newIndex);
						}
					}
					if (!sim->elements[partsptr[newIndex].type].Enabled)
						partsptr[newIndex].type = PT_NONE;
				}
			}
		}
	}
	goto fin;
fail:
	//Clean up everything
	returnCode = 1;
fin:
	bson_destroy(&b);
	if(freeIndices)
		free(freeIndices);
	return returnCode;
}

unsigned char * SaveLoader::OPSBuild(int & dataLength, Simulation * sim, int orig_x0, int orig_y0, int orig_w, int orig_h)
{
	Particle *partsptr = sim->parts;
	unsigned char *partsData = NULL, *partsPosData = NULL, *fanData = NULL, *wallData = NULL, *finalData = NULL, *outputData = NULL;
	unsigned *partsPosLink = NULL, *partsPosFirstMap = NULL, *partsPosCount = NULL, *partsPosLastMap = NULL;
	unsigned int partsDataLen, partsPosDataLen, fanDataLen, wallDataLen, finalDataLen, outputDataLen;
	int blockX, blockY, blockW, blockH, fullX, fullY, fullW, fullH;
	int x, y, i, wallDataFound = 0;
	int posCount, signsCount;
	bson b;

	//Get coords in blocks
	blockX = orig_x0/CELL;
	blockY = orig_y0/CELL;

	//Snap full coords to block size
	fullX = blockX*CELL;
	fullY = blockY*CELL;

	//Original size + offset of original corner from snapped corner, rounded up by adding CELL-1
	blockW = (orig_w+orig_x0-fullX+CELL-1)/CELL;
	blockH = (orig_h+orig_y0-fullY+CELL-1)/CELL;
	fullW = blockW*CELL;
	fullH = blockH*CELL;

	//Copy fan and wall data
	wallData = (unsigned char*)malloc(blockW*blockH);
	wallDataLen = blockW*blockH;
	fanData = (unsigned char*)malloc((blockW*blockH)*2);
	fanDataLen = 0;
	for(x = blockX; x < blockX+blockW; x++)
	{
		for(y = blockY; y < blockY+blockH; y++)
		{
			wallData[(y-blockY)*blockW+(x-blockX)] = sim->bmap[y][x];
			if(sim->bmap[y][x] && !wallDataFound)
				wallDataFound = 1;
			if(sim->bmap[y][x]==WL_FAN)
			{
				i = (int)(sim->fvx[y][x]*64.0f+127.5f);
				if (i<0) i=0;
				if (i>255) i=255;
				fanData[fanDataLen++] = i;
				i = (int)(sim->fvy[y][x]*64.0f+127.5f);
				if (i<0) i=0;
				if (i>255) i=255;
				fanData[fanDataLen++] = i;
			}
		}
	}
	if(!fanDataLen)
	{
		free(fanData);
		fanData = NULL;
	}
	if(!wallDataFound)
	{
		free(wallData);
		wallData = NULL;
	}

	//Index positions of all particles, using linked lists
	//partsPosFirstMap is pmap for the first particle in each position
	//partsPosLastMap is pmap for the last particle in each position
	//partsPosCount is the number of particles in each position
	//partsPosLink contains, for each particle, (i<<8)|1 of the next particle in the same position
	partsPosFirstMap = (unsigned int *)calloc(fullW*fullH, sizeof(unsigned));
	partsPosLastMap = (unsigned int *)calloc(fullW*fullH, sizeof(unsigned));
	partsPosCount = (unsigned int *)calloc(fullW*fullH, sizeof(unsigned));
	partsPosLink = (unsigned int *)calloc(NPART, sizeof(unsigned));
	for(i = 0; i < NPART; i++)
	{
		if(partsptr[i].type)
		{
			x = (int)(partsptr[i].x+0.5f);
			y = (int)(partsptr[i].y+0.5f);
			if (x>=orig_x0 && x<orig_x0+orig_w && y>=orig_y0 && y<orig_y0+orig_h)
			{
				//Coordinates relative to top left corner of saved area
				x -= fullX;
				y -= fullY;
				if (!partsPosFirstMap[y*fullW + x])
				{
					//First entry in list
					partsPosFirstMap[y*fullW + x] = (i<<8)|1;
					partsPosLastMap[y*fullW + x] = (i<<8)|1;
				}
				else
				{
					//Add to end of list
					partsPosLink[partsPosLastMap[y*fullW + x]>>8] = (i<<8)|1;//link to current end of list
					partsPosLastMap[y*fullW + x] = (i<<8)|1;//set as new end of list
				}
				partsPosCount[y*fullW + x]++;
			}
		}
	}

	//Store number of particles in each position
	partsPosData = (unsigned char*)malloc(fullW*fullH*3);
	partsPosDataLen = 0;
	for (y=0;y<fullH;y++)
	{
		for (x=0;x<fullW;x++)
		{
			posCount = partsPosCount[y*fullW + x];
			partsPosData[partsPosDataLen++] = (posCount&0x00FF0000)>>16;
			partsPosData[partsPosDataLen++] = (posCount&0x0000FF00)>>8;
			partsPosData[partsPosDataLen++] = (posCount&0x000000FF);
		}
	}

	//Copy parts data
	/* Field descriptor format:
	|		0		|		0		|		0		|		0		|		0		|		0		|		0		|		0		|		0		|		0		|		0		|		0		|		0		|		0		|		0		|		0		|
																					|		tmp2	|	ctype[2]	|		vy		|		vx		|	dcololour	|	ctype[1]	|		tmp[2]	|		tmp[1]	|		life[2]	|		life[1]	|	temp dbl len|
	life[2] means a second byte (for a 16 bit field) if life[1] is present
	*/
	partsData = (unsigned char *)malloc(NPART * (sizeof(Particle)+1));
	partsDataLen = 0;
	for (y=0;y<fullH;y++)
	{
		for (x=0;x<fullW;x++)
		{
			//Find the first particle in this position
			i = partsPosFirstMap[y*fullW + x];

			//Loop while there is a pmap entry
			while (i)
			{
				unsigned short fieldDesc = 0;
				int fieldDescLoc = 0, tempTemp, vTemp;

				//Turn pmap entry into a partsptr index
				i = i>>8;

				//Type (required)
				partsData[partsDataLen++] = partsptr[i].type;

				//Location of the field descriptor
				fieldDescLoc = partsDataLen++;
				partsDataLen++;

				//Extra Temperature (2nd byte optional, 1st required), 1 to 2 bytes
				//Store temperature as an offset of 21C(294.15K) or go into a 16byte int and store the whole thing
				if(fabs(partsptr[i].temp-294.15f)<127)
				{
					tempTemp = (partsptr[i].temp-294.15f);
					partsData[partsDataLen++] = tempTemp;
				}
				else
				{
					fieldDesc |= 1;
					tempTemp = partsptr[i].temp;
					partsData[partsDataLen++] = tempTemp;
					partsData[partsDataLen++] = tempTemp >> 8;
				}

				//Life (optional), 1 to 2 bytes
				if(partsptr[i].life)
				{
					fieldDesc |= 1 << 1;
					partsData[partsDataLen++] = partsptr[i].life;
					if(partsptr[i].life > 255)
					{
						fieldDesc |= 1 << 2;
						partsData[partsDataLen++] = partsptr[i].life >> 8;
					}
				}

				//Tmp (optional), 1 to 2 bytes
				if(partsptr[i].tmp)
				{
					fieldDesc |= 1 << 3;
					partsData[partsDataLen++] = partsptr[i].tmp;
					if(partsptr[i].tmp > 255)
					{
						fieldDesc |= 1 << 4;
						partsData[partsDataLen++] = partsptr[i].tmp >> 8;
					}
				}

				//Ctype (optional), 1 or 4 bytes
				if(partsptr[i].ctype)
				{
					fieldDesc |= 1 << 5;
					partsData[partsDataLen++] = partsptr[i].ctype;
					if(partsptr[i].ctype > 255)
					{
						fieldDesc |= 1 << 9;
						partsData[partsDataLen++] = (partsptr[i].ctype&0xFF000000)>>24;
						partsData[partsDataLen++] = (partsptr[i].ctype&0x00FF0000)>>16;
						partsData[partsDataLen++] = (partsptr[i].ctype&0x0000FF00)>>8;
					}
				}

				//Dcolour (optional), 4 bytes
				if(partsptr[i].dcolour && (partsptr[i].dcolour & 0xFF000000))
				{
					fieldDesc |= 1 << 6;
					partsData[partsDataLen++] = (partsptr[i].dcolour&0xFF000000)>>24;
					partsData[partsDataLen++] = (partsptr[i].dcolour&0x00FF0000)>>16;
					partsData[partsDataLen++] = (partsptr[i].dcolour&0x0000FF00)>>8;
					partsData[partsDataLen++] = (partsptr[i].dcolour&0x000000FF);
				}

				//VX (optional), 1 byte
				if(fabs(partsptr[i].vx) > 0.001f)
				{
					fieldDesc |= 1 << 7;
					vTemp = (int)(partsptr[i].vx*16.0f+127.5f);
					if (vTemp<0) vTemp=0;
					if (vTemp>255) vTemp=255;
					partsData[partsDataLen++] = vTemp;
				}

				//VY (optional), 1 byte
				if(fabs(partsptr[i].vy) > 0.001f)
				{
					fieldDesc |= 1 << 8;
					vTemp = (int)(partsptr[i].vy*16.0f+127.5f);
					if (vTemp<0) vTemp=0;
					if (vTemp>255) vTemp=255;
					partsData[partsDataLen++] = vTemp;
				}

				//Tmp2 (optional), 1 byte
				if(partsptr[i].tmp2)
				{
					fieldDesc |= 1 << 10;
					partsData[partsDataLen++] = partsptr[i].tmp2;
				}

				//Write the field descriptor;
				partsData[fieldDescLoc] = fieldDesc;
				partsData[fieldDescLoc+1] = fieldDesc>>8;

				//Get the pmap entry for the next particle in the same position
				i = partsPosLink[i];
			}
		}
	}
	if(!partsDataLen)
	{
		free(partsData);
		partsData = NULL;
	}

	bson_init(&b);
	bson_append_bool(&b, "waterEEnabled", sim->water_equal_test);
	bson_append_bool(&b, "legacyEnable", sim->legacy_enable);
	bson_append_bool(&b, "gravityEnable", sim->ngrav_enable);
	bson_append_bool(&b, "paused", sim->sys_pause);
	bson_append_int(&b, "gravityMode", sim->gravityMode);
	bson_append_int(&b, "airMode", sim->air->airMode);

	//bson_append_int(&b, "leftSelectedElement", sl);
	//bson_append_int(&b, "rightSelectedElement", sr);
	//bson_append_int(&b, "activeMenu", active_menu);
	if(partsData)
		bson_append_binary(&b, "parts", BSON_BIN_USER, (const char *)partsData, partsDataLen);
	if(partsPosData)
		bson_append_binary(&b, "partsPos", BSON_BIN_USER, (const char *)partsPosData, partsPosDataLen);
	if(wallData)
		bson_append_binary(&b, "wallMap", BSON_BIN_USER, (const char *)wallData, wallDataLen);
	if(fanData)
		bson_append_binary(&b, "fanMap", BSON_BIN_USER, (const char *)fanData, fanDataLen);
	signsCount = 0;
	for(i = 0; i < sim->signs.size(); i++)
	{
		if(sim->signs[i].text.length() && sim->signs[i].x>=fullX && sim->signs[i].x<=fullX+fullW && sim->signs[i].y>=fullY && sim->signs[i].y<=fullY+fullH)
		{
			signsCount++;
		}
	}
	if(signsCount)
	{
		bson_append_start_array(&b, "signs");
		for(i = 0; i < sim->signs.size(); i++)
		{
			if(sim->signs[i].text.length() && sim->signs[i].x>=fullX && sim->signs[i].x<=fullX+fullW && sim->signs[i].y>=fullY && sim->signs[i].y<=fullY+fullH)
			{
				bson_append_start_object(&b, "sign");
				bson_append_string(&b, "text", sim->signs[i].text.c_str());
				bson_append_int(&b, "justification", sim->signs[i].ju);
				bson_append_int(&b, "x", sim->signs[i].x-fullX);
				bson_append_int(&b, "y", sim->signs[i].y-fullY);
				bson_append_finish_object(&b);
			}
		}
	}
	bson_append_finish_array(&b);
	bson_finish(&b);
	bson_print(&b);

	finalData = (unsigned char *)bson_data(&b);
	finalDataLen = bson_size(&b);
	outputDataLen = finalDataLen*2+12;
	outputData = (unsigned char *)malloc(outputDataLen);

	outputData[0] = 'O';
	outputData[1] = 'P';
	outputData[2] = 'S';
	outputData[3] = '1';
	outputData[4] = SAVE_VERSION;
	outputData[5] = CELL;
	outputData[6] = blockW;
	outputData[7] = blockH;
	outputData[8] = finalDataLen;
	outputData[9] = finalDataLen >> 8;
	outputData[10] = finalDataLen >> 16;
	outputData[11] = finalDataLen >> 24;

	if (BZ2_bzBuffToBuffCompress((char*)(outputData+12), &outputDataLen, (char*)finalData, bson_size(&b), 9, 0, 0) != BZ_OK)
	{
		puts("Save Error\n");
		free(outputData);
		dataLength = 0;
		outputData = NULL;
		goto fin;
	}

	printf("compressed data: %d\n", outputDataLen);
	dataLength = outputDataLen + 12;

fin:
	bson_destroy(&b);
	if(partsData)
		free(partsData);
	if(wallData)
		free(wallData);
	if(fanData)
		free(fanData);

	return outputData;
}

int SaveLoader::PSVInfo(unsigned char * data, int dataLength, int & width, int & height)
{
	width = data[6];
	height = data[7];
	return 0;
}

int SaveLoader::PSVLoad(unsigned char * data, int dataLength, Simulation * sim, bool replace, int x0, int y0)
{
	unsigned char * d = NULL, * c = data;
	int q,i,j,k,x,y,p=0,*m=NULL, ver, pty, ty, legacy_beta=0, tempGrav = 0;
	int bx0=x0/CELL, by0=y0/CELL, bw, bh, w, h;
	int nf=0, new_format = 0, ttv = 0;
	Particle *parts = sim->parts;
	int *fp = (int *)malloc(NPART*sizeof(int));
	
	std::vector<sign> tempSigns;
	char tempSignText[255];
	sign tempSign("", 0, 0, sign::Left);

	//New file header uses PSv, replacing fuC. This is to detect if the client uses a new save format for temperatures
	//This creates a problem for old clients, that display and "corrupt" error instead of a "newer version" error

	if (dataLength<16)
		return 1;
	if (!(c[2]==0x43 && c[1]==0x75 && c[0]==0x66) && !(c[2]==0x76 && c[1]==0x53 && c[0]==0x50))
		return 1;
	if (c[2]==0x76 && c[1]==0x53 && c[0]==0x50) {
		new_format = 1;
	}
	if (c[4]>SAVE_VERSION)
		return 2;
	ver = c[4];

	if (ver<34)
	{
		sim->legacy_enable = 1;
	}
	else
	{
		if (ver>=44) {
			sim->legacy_enable = c[3]&0x01;
			if (!sim->sys_pause) {
				sim->sys_pause = (c[3]>>1)&0x01;
			}
			if (ver>=46 && replace) {
				sim->gravityMode = ((c[3]>>2)&0x03);// | ((c[3]>>2)&0x01);
				sim->air->airMode = ((c[3]>>4)&0x07);// | ((c[3]>>4)&0x02) | ((c[3]>>4)&0x01);
			}
			if (ver>=49 && replace) {
				tempGrav = ((c[3]>>7)&0x01);
			}
		} else {
			if (c[3]==1||c[3]==0) {
				sim->legacy_enable = c[3];
			} else {
				legacy_beta = 1;
			}
		}
	}

	bw = c[6];
	bh = c[7];
	if (bx0+bw > XRES/CELL)
		bx0 = XRES/CELL - bw;
	if (by0+bh > YRES/CELL)
		by0 = YRES/CELL - bh;
	if (bx0 < 0)
		bx0 = 0;
	if (by0 < 0)
		by0 = 0;

	if (c[5]!=CELL || bx0+bw>XRES/CELL || by0+bh>YRES/CELL)
		return 3;
	i = (unsigned)c[8];
	i |= ((unsigned)c[9])<<8;
	i |= ((unsigned)c[10])<<16;
	i |= ((unsigned)c[11])<<24;
	d = (unsigned char *)malloc(i);
	if (!d)
		return 1;

	if (BZ2_bzBuffToBuffDecompress((char *)d, (unsigned *)&i, (char *)(c+12), dataLength-12, 0, 0))
		return 1;
	dataLength = i;

	if (dataLength < bw*bh)
		return 1;

	// normalize coordinates
	x0 = bx0*CELL;
	y0 = by0*CELL;
	w  = bw *CELL;
	h  = bh *CELL;

	if (replace)
	{
		if (ver<46) {
			sim->gravityMode = 0;
			sim->air->airMode = 0;
		}
		sim->clear_sim();
	}
	sim->parts_lastActiveIndex = NPART-1;
	m = (int *)calloc(XRES*YRES, sizeof(int));

	// make a catalog of free parts
	//memset(pmap, 0, sizeof(pmap)); "Using sizeof for array given as function argument returns the size of pointer."
	memset(sim->pmap, 0, sizeof(unsigned)*(XRES*YRES));
	for (i=0; i<NPART; i++)
		if (parts[i].type)
		{
			x = (int)(parts[i].x+0.5f);
			y = (int)(parts[i].y+0.5f);
			sim->pmap[y][x] = (i<<8)|1;
		}
		else
			fp[nf++] = i;

	// load the required air state
	for (y=by0; y<by0+bh; y++)
		for (x=bx0; x<bx0+bw; x++)
		{
			if (d[p])
			{
				//In old saves, ignore walls created by sign tool bug
				//Not ignoring other invalid walls or invalid walls in new saves, so that any other bugs causing them are easier to notice, find and fix
				if (ver<71 && d[p]==O_WL_SIGN)
				{
					p++;
					continue;
				}

				sim->bmap[y][x] = d[p];
				if (sim->bmap[y][x]==1)
					sim->bmap[y][x]=WL_WALL;
				if (sim->bmap[y][x]==2)
					sim->bmap[y][x]=WL_DESTROYALL;
				if (sim->bmap[y][x]==3)
					sim->bmap[y][x]=WL_ALLOWLIQUID;
				if (sim->bmap[y][x]==4)
					sim->bmap[y][x]=WL_FAN;
				if (sim->bmap[y][x]==5)
					sim->bmap[y][x]=WL_STREAM;
				if (sim->bmap[y][x]==6)
					sim->bmap[y][x]=WL_DETECT;
				if (sim->bmap[y][x]==7)
					sim->bmap[y][x]=WL_EWALL;
				if (sim->bmap[y][x]==8)
					sim->bmap[y][x]=WL_WALLELEC;
				if (sim->bmap[y][x]==9)
					sim->bmap[y][x]=WL_ALLOWAIR;
				if (sim->bmap[y][x]==10)
					sim->bmap[y][x]=WL_ALLOWSOLID;
				if (sim->bmap[y][x]==11)
					sim->bmap[y][x]=WL_ALLOWALLELEC;
				if (sim->bmap[y][x]==12)
					sim->bmap[y][x]=WL_EHOLE;
				if (sim->bmap[y][x]==13)
					sim->bmap[y][x]=WL_ALLOWGAS;
				
				if (sim->bmap[y][x]==O_WL_WALLELEC)
					sim->bmap[y][x]=WL_WALLELEC;
				if (sim->bmap[y][x]==O_WL_EWALL)
					sim->bmap[y][x]=WL_EWALL;
				if (sim->bmap[y][x]==O_WL_DETECT)
					sim->bmap[y][x]=WL_DETECT;
				if (sim->bmap[y][x]==O_WL_STREAM)
					sim->bmap[y][x]=WL_STREAM;
				if (sim->bmap[y][x]==O_WL_FAN||sim->bmap[y][x]==O_WL_FANHELPER)
					sim->bmap[y][x]=WL_FAN;
				if (sim->bmap[y][x]==O_WL_ALLOWLIQUID)
					sim->bmap[y][x]=WL_ALLOWLIQUID;
				if (sim->bmap[y][x]==O_WL_DESTROYALL)
					sim->bmap[y][x]=WL_DESTROYALL;
				if (sim->bmap[y][x]==O_WL_ERASE)
					sim->bmap[y][x]=WL_ERASE;
				if (sim->bmap[y][x]==O_WL_WALL)
					sim->bmap[y][x]=WL_WALL;
				if (sim->bmap[y][x]==O_WL_ALLOWAIR)
					sim->bmap[y][x]=WL_ALLOWAIR;
				if (sim->bmap[y][x]==O_WL_ALLOWSOLID)
					sim->bmap[y][x]=WL_ALLOWSOLID;
				if (sim->bmap[y][x]==O_WL_ALLOWALLELEC)
					sim->bmap[y][x]=WL_ALLOWALLELEC;
				if (sim->bmap[y][x]==O_WL_EHOLE)
					sim->bmap[y][x]=WL_EHOLE;
				if (sim->bmap[y][x]==O_WL_ALLOWGAS)
					sim->bmap[y][x]=WL_ALLOWGAS;
				if (sim->bmap[y][x]==O_WL_GRAV)
					sim->bmap[y][x]=WL_GRAV;
				if (sim->bmap[y][x]==O_WL_ALLOWENERGY)
					sim->bmap[y][x]=WL_ALLOWENERGY;
			}

			p++;
		}
	for (y=by0; y<by0+bh; y++)
		for (x=bx0; x<bx0+bw; x++)
			if (d[(y-by0)*bw+(x-bx0)]==4||d[(y-by0)*bw+(x-bx0)]==WL_FAN)
			{
				if (p >= dataLength)
					goto corrupt;
				sim->fvx[y][x] = (d[p++]-127.0f)/64.0f;
			}
	for (y=by0; y<by0+bh; y++)
		for (x=bx0; x<bx0+bw; x++)
			if (d[(y-by0)*bw+(x-bx0)]==4||d[(y-by0)*bw+(x-bx0)]==WL_FAN)
			{
				if (p >= dataLength)
					goto corrupt;
				sim->fvy[y][x] = (d[p++]-127.0f)/64.0f;
			}

	// load the particle map
	i = 0;
	pty = p;
	for (y=y0; y<y0+h; y++)
		for (x=x0; x<x0+w; x++)
		{
			if (p >= dataLength)
				goto corrupt;
			j=d[p++];
			if (j >= PT_NUM) {
				//TODO: Possibly some server side translation
				j = PT_DUST;//goto corrupt;
			}
			sim->gol[x][y]=0;
			if (j)
			{
				if (sim->pmap[y][x])
				{
					k = sim->pmap[y][x]>>8;
				}
				else if (i<nf)
				{
					k = fp[i];
					i++;
				}
				else
				{
					m[(x-x0)+(y-y0)*w] = NPART+1;
					continue;
				}
				memset(parts+k, 0, sizeof(Particle));
				parts[k].type = j;
				if (j == PT_COAL)
					parts[k].tmp = 50;
				if (j == PT_FUSE)
					parts[k].tmp = 50;
				if (j == PT_PHOT)
					parts[k].ctype = 0x3fffffff;
				if (j == PT_SOAP)
					parts[k].ctype = 0;
				if (j==PT_BIZR || j==PT_BIZRG || j==PT_BIZRS)
					parts[k].ctype = 0x47FFFF;
				parts[k].x = (float)x;
				parts[k].y = (float)y;
				m[(x-x0)+(y-y0)*w] = k+1;
			}
		}

	// load particle properties
	for (j=0; j<w*h; j++)
	{
		i = m[j];
		if (i)
		{
			i--;
			if (p+1 >= dataLength)
				goto corrupt;
			if (i < NPART)
			{
				parts[i].vx = (d[p++]-127.0f)/16.0f;
				parts[i].vy = (d[p++]-127.0f)/16.0f;
			}
			else
				p += 2;
		}
	}
	for (j=0; j<w*h; j++)
	{
		i = m[j];
		if (i)
		{
			if (ver>=44) {
				if (p >= dataLength) {
					goto corrupt;
				}
				if (i <= NPART) {
					ttv = (d[p++])<<8;
					ttv |= (d[p++]);
					parts[i-1].life = ttv;
				} else {
					p+=2;
				}
			} else {
				if (p >= dataLength)
					goto corrupt;
				if (i <= NPART)
					parts[i-1].life = d[p++]*4;
				else
					p++;
			}
		}
	}
	if (ver>=44) {
		for (j=0; j<w*h; j++)
		{
			i = m[j];
			if (i)
			{
				if (p >= dataLength) {
					goto corrupt;
				}
				if (i <= NPART) {
					ttv = (d[p++])<<8;
					ttv |= (d[p++]);
					parts[i-1].tmp = ttv;
					if (ver<53 && !parts[i-1].tmp)
						for (q = 1; q<=NGOLALT; q++) {
							if (parts[i-1].type==sim->goltype[q-1] && sim->grule[q][9]==2)
								parts[i-1].tmp = sim->grule[q][9]-1;
						}
					if (ver>=51 && ver<53 && parts[i-1].type==PT_PBCN)
					{
						parts[i-1].tmp2 = parts[i-1].tmp;
						parts[i-1].tmp = 0;
					}
				} else {
					p+=2;
				}
			}
		}
	}
	if (ver>=53) {
		for (j=0; j<w*h; j++)
		{
			i = m[j];
			ty = d[pty+j];
			if (i && ty==PT_PBCN)
			{
				if (p >= dataLength)
					goto corrupt;
				if (i <= NPART)
					parts[i-1].tmp2 = d[p++];
				else
					p++;
			}
		}
	}
	//Read ALPHA component
	for (j=0; j<w*h; j++)
	{
		i = m[j];
		if (i)
		{
			if (ver>=49) {
				if (p >= dataLength) {
					goto corrupt;
				}
				if (i <= NPART) {
					parts[i-1].dcolour = d[p++]<<24;
				} else {
					p++;
				}
			}
		}
	}
	//Read RED component
	for (j=0; j<w*h; j++)
	{
		i = m[j];
		if (i)
		{
			if (ver>=49) {
				if (p >= dataLength) {
					goto corrupt;
				}
				if (i <= NPART) {
					parts[i-1].dcolour |= d[p++]<<16;
				} else {
					p++;
				}
			}
		}
	}
	//Read GREEN component
	for (j=0; j<w*h; j++)
	{
		i = m[j];
		if (i)
		{
			if (ver>=49) {
				if (p >= dataLength) {
					goto corrupt;
				}
				if (i <= NPART) {
					parts[i-1].dcolour |= d[p++]<<8;
				} else {
					p++;
				}
			}
		}
	}
	//Read BLUE component
	for (j=0; j<w*h; j++)
	{
		i = m[j];
		if (i)
		{
			if (ver>=49) {
				if (p >= dataLength) {
					goto corrupt;
				}
				if (i <= NPART) {
					parts[i-1].dcolour |= d[p++];
				} else {
					p++;
				}
			}
		}
	}
	for (j=0; j<w*h; j++)
	{
		i = m[j];
		ty = d[pty+j];
		if (i)
		{
			if (ver>=34&&legacy_beta==0)
			{
				if (p >= dataLength)
				{
					goto corrupt;
				}
				if (i <= NPART)
				{
					if (ver>=42) {
						if (new_format) {
							ttv = (d[p++])<<8;
							ttv |= (d[p++]);
							if (parts[i-1].type==PT_PUMP) {
								parts[i-1].temp = ttv + 0.15;//fix PUMP saved at 0, so that it loads at 0.
							} else {
								parts[i-1].temp = ttv;
							}
						} else {
							parts[i-1].temp = (d[p++]*((MAX_TEMP+(-MIN_TEMP))/255))+MIN_TEMP;
						}
					} else {
						parts[i-1].temp = ((d[p++]*((O_MAX_TEMP+(-O_MIN_TEMP))/255))+O_MIN_TEMP)+273;
					}
				}
				else
				{
					p++;
					if (new_format) {
						p++;
					}
				}
			}
			else
			{
				parts[i-1].temp = sim->elements[parts[i-1].type].Temperature;
			}
		}
	}
	for (j=0; j<w*h; j++)
	{
		int gnum = 0;
		i = m[j];
		ty = d[pty+j];
		if (i && (ty==PT_CLNE || (ty==PT_PCLN && ver>=43) || (ty==PT_BCLN && ver>=44) || (ty==PT_SPRK && ver>=21) || (ty==PT_LAVA && ver>=34) || (ty==PT_PIPE && ver>=43) || (ty==PT_LIFE && ver>=51) || (ty==PT_PBCN && ver>=52) || (ty==PT_WIRE && ver>=55) || (ty==PT_STOR && ver>=59) || (ty==PT_CONV && ver>=60)))
		{
			if (p >= dataLength)
				goto corrupt;
			if (i <= NPART)
				parts[i-1].ctype = d[p++];
			else
				p++;
		}
		//TODO: STKM_init_legs
		// no more particle properties to load, so we can change type here without messing up loading
		if (i && i<=NPART)
		{
			if ((sim->player.spwn == 1 && ty==PT_STKM) || (sim->player2.spwn == 1 && ty==PT_STKM2))
			{
				parts[i-1].type = PT_NONE;
			}
			else if (parts[i-1].type == PT_STKM)
			{
				//STKM_init_legs(&player, i-1);
				sim->player.spwn = 1;
				sim->player.elem = PT_DUST;
			}
			else if (parts[i-1].type == PT_STKM2)
			{
				//STKM_init_legs(&player2, i-1);
				sim->player2.spwn = 1;
				sim->player2.elem = PT_DUST;
			}
			else if (parts[i-1].type == PT_FIGH)
			{
				unsigned char fcount = 0;
				while (fcount < 100 && fcount < (sim->fighcount+1) && sim->fighters[fcount].spwn==1) fcount++;
				if (fcount < 100 && sim->fighters[fcount].spwn==0)
				{
					parts[i-1].tmp = fcount;
					sim->fighters[fcount].spwn = 1;
					sim->fighters[fcount].elem = PT_DUST;
					sim->fighcount++;
					//STKM_init_legs(&(fighters[fcount]), i-1);
				}
			}
			else if (parts[i-1].type == PT_SPNG)
			{
				if (fabs(parts[i-1].vx)>0.0f || fabs(parts[i-1].vy)>0.0f)
					parts[i-1].flags |= FLAG_MOVABLE;
			}

			if (ver<48 && (ty==OLD_PT_WIND || (ty==PT_BRAY&&parts[i-1].life==0)))
			{
				// Replace invisible particles with something sensible and add decoration to hide it
				x = (int)(parts[i-1].x+0.5f);
				y = (int)(parts[i-1].y+0.5f);
				parts[i-1].dcolour = 0xFF000000;
				parts[i-1].type = PT_DMND;
			}
			if(ver<51 && ((ty>=78 && ty<=89) || (ty>=134 && ty<=146 && ty!=141))){
				//Replace old GOL
				parts[i-1].type = PT_LIFE;
				for (gnum = 0; gnum<NGOLALT; gnum++){
					if (ty==sim->goltype[gnum])
						parts[i-1].ctype = gnum;
				}
				ty = PT_LIFE;
			}
			if(ver<52 && (ty==PT_CLNE || ty==PT_PCLN || ty==PT_BCLN)){
				//Replace old GOL ctypes in clone
				for (gnum = 0; gnum<NGOLALT; gnum++){
					if (parts[i-1].ctype==sim->goltype[gnum])
					{
						parts[i-1].ctype = PT_LIFE;
						parts[i-1].tmp = gnum;
					}
				}
			}
			if(ty==PT_LCRY){
				if(ver<67)
				{
					//New LCRY uses TMP not life
					if(parts[i-1].life>=10)
					{
						parts[i-1].life = 10;
						parts[i-1].tmp2 = 10;
						parts[i-1].tmp = 3;
					}
					else if(parts[i-1].life<=0)
					{
						parts[i-1].life = 0;
						parts[i-1].tmp2 = 0;
						parts[i-1].tmp = 0;
					}
					else if(parts[i-1].life < 10 && parts[i-1].life > 0)
					{
						parts[i-1].tmp = 1;
					}
				}
				else
				{
					parts[i-1].tmp2 = parts[i-1].life;
				}
			}
			if (!sim->elements[parts[i-1].type].Enabled)
				parts[i-1].type = PT_NONE;
		}
	}

	#ifndef RENDERER
	//Change the gravity state
	if(sim->ngrav_enable != tempGrav && replace)
	{
		if(tempGrav)
			sim->grav->start_grav_async();
		else
			sim->grav->stop_grav_async();
	}
	#endif

	sim->grav->gravity_mask();

	if (p >= dataLength)
		goto version1;
	j = d[p++];
	for (i=0; i<j; i++)
	{
		if (p+6 > dataLength)
			goto corrupt;
		x = d[p++];
		x |= ((unsigned)d[p++])<<8;
		tempSign.x = x+x0;
		x = d[p++];
		x |= ((unsigned)d[p++])<<8;
		tempSign.y = x+y0;
		x = d[p++];
		tempSign.ju = (sign::Justification)x;
		x = d[p++];
		if (p+x > dataLength)
		goto corrupt;
		if(x>254)
			x = 254;
		memcpy(tempSignText, d+p, x);
		tempSignText[x] = 0;
		tempSign.text = tempSignText;
		tempSigns.push_back(tempSign);
		p += x;
	}
	
	for (i = 0; i < tempSigns.size(); i++)
	{
		if(i == MAXSIGNS)
			break;
		sim->signs.push_back(tempSigns[i]);
	}

version1:
	if (m) free(m);
	if (d) free(d);
	if (fp) free(fp);

	return 0;

corrupt:
	if (m) free(m);
	if (d) free(d);
	if (fp) free(fp);
	if (replace)
	{
		sim->legacy_enable = 0;
		sim->clear_sim();
	}
	return 1;
}

unsigned char * SaveLoader::PSVBuild(int & dataLength, Simulation * sim, int orig_x0, int orig_y0, int orig_w, int orig_h)
{
	unsigned char *d = (unsigned char*)calloc(1,3*(XRES/CELL)*(YRES/CELL)+(XRES*YRES)*15+MAXSIGNS*262), *c;
	int i,j,x,y,p=0,*m=(int*)calloc(XRES*YRES, sizeof(int));
	int x0, y0, w, h, bx0=orig_x0/CELL, by0=orig_y0/CELL, bw, bh;
	Particle *parts = sim->parts;
	bw=(orig_w+orig_x0-bx0*CELL+CELL-1)/CELL;
	bh=(orig_h+orig_y0-by0*CELL+CELL-1)/CELL;

	// normalize coordinates
	x0 = bx0*CELL;
	y0 = by0*CELL;
	w  = bw *CELL;
	h  = bh *CELL;

	// save the required air state
	for (y=by0; y<by0+bh; y++)
		for (x=bx0; x<bx0+bw; x++)
			d[p++] = sim->bmap[y][x];
	for (y=by0; y<by0+bh; y++)
		for (x=bx0; x<bx0+bw; x++)
			if (sim->bmap[y][x]==WL_FAN||sim->bmap[y][x]==4)
			{
				i = (int)(sim->fvx[y][x]*64.0f+127.5f);
				if (i<0) i=0;
				if (i>255) i=255;
				d[p++] = i;
			}
	for (y=by0; y<by0+bh; y++)
		for (x=bx0; x<bx0+bw; x++)
			if (sim->bmap[y][x]==WL_FAN||sim->bmap[y][x]==4)
			{
				i = (int)(sim->fvy[y][x]*64.0f+127.5f);
				if (i<0) i=0;
				if (i>255) i=255;
				d[p++] = i;
			}

	// save the particle map
	for (i=0; i<NPART; i++)
		if (parts[i].type)
		{
			x = (int)(parts[i].x+0.5f);
			y = (int)(parts[i].y+0.5f);
			if (x>=orig_x0 && x<orig_x0+orig_w && y>=orig_y0 && y<orig_y0+orig_h) {
				if (!m[(x-x0)+(y-y0)*w] ||
				        parts[m[(x-x0)+(y-y0)*w]-1].type == PT_PHOT ||
				        parts[m[(x-x0)+(y-y0)*w]-1].type == PT_NEUT)
					m[(x-x0)+(y-y0)*w] = i+1;
			}
		}
	for (j=0; j<w*h; j++)
	{
		i = m[j];
		if (i)
			d[p++] = parts[i-1].type;
		else
			d[p++] = 0;
	}

	// save particle properties
	for (j=0; j<w*h; j++)
	{
		i = m[j];
		if (i)
		{
			i--;
			x = (int)(parts[i].vx*16.0f+127.5f);
			y = (int)(parts[i].vy*16.0f+127.5f);
			if (x<0) x=0;
			if (x>255) x=255;
			if (y<0) y=0;
			if (y>255) y=255;
			d[p++] = x;
			d[p++] = y;
		}
	}
	for (j=0; j<w*h; j++)
	{
		i = m[j];
		if (i) {
			//Everybody loves a 16bit int
			//d[p++] = (parts[i-1].life+3)/4;
			int ttlife = (int)parts[i-1].life;
			d[p++] = ((ttlife&0xFF00)>>8);
			d[p++] = (ttlife&0x00FF);
		}
	}
	for (j=0; j<w*h; j++)
	{
		i = m[j];
		if (i) {
			//Now saving tmp!
			//d[p++] = (parts[i-1].life+3)/4;
			int tttmp = (int)parts[i-1].tmp;
			d[p++] = ((tttmp&0xFF00)>>8);
			d[p++] = (tttmp&0x00FF);
		}
	}
	for (j=0; j<w*h; j++)
	{
		i = m[j];
		if (i && (parts[i-1].type==PT_PBCN)) {
			//Save tmp2
			d[p++] = parts[i-1].tmp2;
		}
	}
	for (j=0; j<w*h; j++)
	{
		i = m[j];
		if (i) {
			//Save colour (ALPHA)
			d[p++] = (parts[i-1].dcolour&0xFF000000)>>24;
		}
	}
	for (j=0; j<w*h; j++)
	{
		i = m[j];
		if (i) {
			//Save colour (RED)
			d[p++] = (parts[i-1].dcolour&0x00FF0000)>>16;
		}
	}
	for (j=0; j<w*h; j++)
	{
		i = m[j];
		if (i) {
			//Save colour (GREEN)
			d[p++] = (parts[i-1].dcolour&0x0000FF00)>>8;
		}
	}
	for (j=0; j<w*h; j++)
	{
		i = m[j];
		if (i) {
			//Save colour (BLUE)
			d[p++] = (parts[i-1].dcolour&0x000000FF);
		}
	}
	for (j=0; j<w*h; j++)
	{
		i = m[j];
		if (i)
		{
			//New Temperature saving uses a 16bit unsigned int for temperatures, giving a precision of 1 degree versus 36 for the old format
			int tttemp = (int)parts[i-1].temp;
			d[p++] = ((tttemp&0xFF00)>>8);
			d[p++] = (tttemp&0x00FF);
		}
	}
	for (j=0; j<w*h; j++)
	{
		i = m[j];
		if (i && (parts[i-1].type==PT_CLNE || parts[i-1].type==PT_PCLN || parts[i-1].type==PT_BCLN || parts[i-1].type==PT_SPRK || parts[i-1].type==PT_LAVA || parts[i-1].type==PT_PIPE || parts[i-1].type==PT_LIFE || parts[i-1].type==PT_PBCN || parts[i-1].type==PT_WIRE || parts[i-1].type==PT_STOR || parts[i-1].type==PT_CONV))
			d[p++] = parts[i-1].ctype;
	}

	j = 0;
	for (i=0; i<sim->signs.size(); i++)
		if (sim->signs[i].text.length() &&
				sim->signs[i].x>=x0 && sim->signs[i].x<x0+w &&
				sim->signs[i].y>=y0 && sim->signs[i].y<y0+h)
			j++;
	d[p++] = j;
	for (i=0; i<sim->signs.size(); i++)
		if (sim->signs[i].text.length() &&
				sim->signs[i].x>=x0 && sim->signs[i].x<x0+w &&
				sim->signs[i].y>=y0 && sim->signs[i].y<y0+h)
		{
			d[p++] = (sim->signs[i].x-x0);
			d[p++] = (sim->signs[i].x-x0)>>8;
			d[p++] = (sim->signs[i].y-y0);
			d[p++] = (sim->signs[i].y-y0)>>8;
			d[p++] = sim->signs[i].ju;
			x = sim->signs[i].text.length();
			d[p++] = x;
			memcpy(d+p, sim->signs[i].text.c_str(), x);
			p+=x;
		}

	i = (p*101+99)/100 + 612;
	c = (unsigned char*)malloc(i);

	//New file header uses PSv, replacing fuC. This is to detect if the client uses a new save format for temperatures
	//This creates a problem for old clients, that display and "corrupt" error instead of a "newer version" error

	c[0] = 0x50;	//0x66;
	c[1] = 0x53;	//0x75;
	c[2] = 0x76;	//0x43;
	c[3] = sim->legacy_enable|((sim->sys_pause<<1)&0x02)|((sim->gravityMode<<2)&0x0C)|((sim->air->airMode<<4)&0x70)|((sim->ngrav_enable<<7)&0x80);
	c[4] = SAVE_VERSION;
	c[5] = CELL;
	c[6] = bw;
	c[7] = bh;
	c[8] = p;
	c[9] = p >> 8;
	c[10] = p >> 16;
	c[11] = p >> 24;

	i -= 12;

	if (BZ2_bzBuffToBuffCompress((char *)(c+12), (unsigned *)&i, (char *)d, p, 9, 0, 0) != BZ_OK)
	{
		free(d);
		free(c);
		free(m);
		return NULL;
	}
	free(d);
	free(m);

	dataLength = i+12;
	return c;
}
