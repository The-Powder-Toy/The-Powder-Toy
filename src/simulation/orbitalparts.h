#pragma once

inline void orbitalparts_get(int block1, int block2, int resblock1[], int resblock2[])
{
	resblock1[0] = (block1&0x000000FF);
	resblock1[1] = (block1&0x0000FF00)>>8;
	resblock1[2] = (block1&0x00FF0000)>>16;
	resblock1[3] = (block1&0xFF000000)>>24;

	resblock2[0] = (block2&0x000000FF);
	resblock2[1] = (block2&0x0000FF00)>>8;
	resblock2[2] = (block2&0x00FF0000)>>16;
	resblock2[3] = (block2&0xFF000000)>>24;
}

inline void orbitalparts_set(int *block1, int *block2, int resblock1[], int resblock2[])
{
	int block1tmp = 0;
	int block2tmp = 0;

	block1tmp = (resblock1[0]&0xFF);
	block1tmp |= (resblock1[1]&0xFF)<<8;
	block1tmp |= (resblock1[2]&0xFF)<<16;
	block1tmp |= (resblock1[3]&0xFF)<<24;

	block2tmp = (resblock2[0]&0xFF);
	block2tmp |= (resblock2[1]&0xFF)<<8;
	block2tmp |= (resblock2[2]&0xFF)<<16;
	block2tmp |= (resblock2[3]&0xFF)<<24;

	*block1 = block1tmp;
	*block2 = block2tmp;
}
