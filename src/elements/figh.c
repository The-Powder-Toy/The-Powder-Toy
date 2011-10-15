#include <element.h>

int update_FIGH(UPDATE_FUNC_ARGS)
{
	float* figh = fighters[(unsigned char)parts[i].tmp];

	float tarx, tary;

	parts[i].tmp2 = 0; //0 - stay in place, 1 - seek a stick man

	//Set target cords
	if (player[27])
	{
		if (player2[27])
			if ((pow(player[5]-x, 2) + pow(player[6]-y, 2))<=
					(pow(player2[5]-x, 2) + pow(player2[6]-y, 2)))
			{
				tarx = player[5];
				tary = player[6];
			}
			else
			{
				tarx = player2[5];
				tary = player2[6];
			}
		else
		{
			tarx = player[5];
			tary = player[6];
		}

		parts[i].tmp2 = 1;
	}
	else
		if (player2[27])
		{
			tarx = player2[5];
			tary = player2[6];

			parts[i].tmp2 = 1;
		}

	switch (parts[i].tmp2)
	{
		case 1:
			if ((pow(tarx-x, 2) + pow(tary-y, 2))<600)
			{
				if (figh[2] == PT_FIRE)
					figh[0] = (int)figh[0] | 0x08;
			}

			if ((pow(tarx-x, 2) + pow(tary-y, 2))<300)
			{
				if (figh[2] == PT_FIRE)
					figh[0] = 0x08;
				else
					figh[0] = 0;
			}
			else
				if (tarx<x)
				{
					figh[0] = 0x01;
					if (!eval_move(PT_DUST, figh[7]-4, figh[8]-1, NULL) || !eval_move(PT_DUST, figh[15]-4, figh[16]-1, NULL))
						figh[0] = (int)figh[0] | 0x04;
				}
				else
				{
					figh[0] = 0x02;
					if (!eval_move(PT_DUST, figh[7]+4, figh[8]-1, NULL) || !eval_move(PT_DUST, figh[15]+4, figh[16]-1, NULL))
						figh[0] = (int)figh[0] | 0x04;
				}
			break;
		default:
			figh[0] = 0;
			break;
	}

	run_stickman(figh, UPDATE_FUNC_SUBCALL_ARGS);
	return 0;
}
