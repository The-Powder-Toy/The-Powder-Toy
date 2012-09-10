#include "tpt_syscalls.h"

int update_TEST(int i, int x, int y, int nt, int ss)
{
	partCreate(-1, x, y-1, 2);
	return 0;
}
