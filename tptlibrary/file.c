#include "tpt_syscalls.h"

int update_TEST(int i, int x, int y, int nt, int ss)
{
	int k, j, index;

	for(k=-1; k<2; k++)
		for(j=-1; j<2; j++)
		{
			index = pmapData(x+k, y+j);
			if ((index&0xFF) == 2)
				killPart(index>>8);
		}
	return 0;
}
