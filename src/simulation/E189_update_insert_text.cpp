#include "simulation/Elements.h"
#include "simulation/E189_update.h"
#include <iostream>
#include "font.h"

void E189_Update::InsertText(Simulation *sim, int i, int x, int y, int ix, int iy)
{
	Element_E189::useDefaultPart = false;
	int tab_size = 80;
	// simulation, index, position (x2), direction (x2)
	int ct_x = (sim->parts[i].ctype & 0xFFFF), ct_y = ((sim->parts[i].ctype >> 16) & 0xFFFF);
	int it_x = ct_x, it_r, it_g, it_b, chr_1, esc = 0, pack, bkup, errflag = 0, cfptr;
	int oldr, oldg, oldb, call_ptr = 0, tmp = 0, diff;
	char __digits[5];
	short counter = 0;
	short calls[128][5]; /* dynamic */
	it_r = it_g = it_b = 255;
	for (;;)
	{
		x += ix; y += iy;
		int r = sim->pmap[y][x];
		if ((r&0xFF) != PT_E189) // if not "E189" break loop
			break;
		pack = sim->parts[r>>8].life;
		chr_1 = sim->parts[r>>8].ctype;
		if ((pack & ~0x1) == 0x2) // if "tron portal"
		{
			if (pack == 2) // if "tron input"
			{
				if (esc == 0)
					esc = 5;
				else if (esc = 1)
					bkup = 1;
			}
			else
			{
				if (esc == 1)
				{
					it_r = oldr; it_g = oldg; it_b = oldb;
				}
				esc = 0;
			}
			continue;
		}
		if (pack == 12) // if "spark reflector"
		{
			switch (chr_1 & 63) // chr_1 : initial ctype
			{
				case 0: ix = 1; iy = 0; break; // go east
				case 1: ix = 0; iy =-1; break; // go north
				case 2: ix =-1; iy = 0; break; // go west
				case 3: ix = 0; iy = 1; break; // go south
				case 4: // turn clockwise
					chr_1 = ix; ix = iy; iy = -chr_1;
				break;
				case 5: // turn counter clockwise
					chr_1 = ix; ix = -iy; iy = chr_1;
				break;
				case 6: /* "/" reflect */
					chr_1 = ix; ix = iy; iy = chr_1;
				break;
				case 7: /* "\" reflect */
					chr_1 = ix; ix = -iy; iy = -chr_1;
				break;
				case 8: /* "|" reflect */
					ix = -ix;
				break;
				case 9: /* "-" reflect */
					iy = -iy;
				break;
				case 10: /* turn 180 */
					ix = -ix; iy = -iy;
				break;
				case 11: /* random turn */
					pack = (rand() & 1) * 2 - 1;
					chr_1 = ix; ix = iy * pack; iy = chr_1 * pack;
				break;
				case 12: /* random straight */
					pack = (rand() & 1) * 2 - 1;
					ix *= pack; iy *= pack;
				break;
				case 13: /* random dir. */
				{
					int turn_rx[4] = {-1, 0, 1, 0};
					int turn_ry[4] = { 0,-1, 0, 1};
					chr_1 = (rand() & 3);
					ix = turn_rx[chr_1]; iy = turn_ry[chr_1];
				}
				case 14: // random vertical
					ix = 0; iy = (rand() & 1) * 2 - 1;
				break;
				case 15: // random horizontal
					iy = 0; ix = (rand() & 1) * 2 - 1;
				break;
				case 16: // trampoline
					x += ix; y += iy;
				break;
				case 17: // trampoline 2
					x += ix * 2; y += iy * 2;
				break;
				case 18: // trampoline 3 ... N
					r = sim->pmap[y+iy][x+ix];
					if ((r & 0xFF) == PT_E189)
					{
						chr_1 = sim->parts[r>>8].life;
						if (chr_1 & ~0x1 == 0x2)
							chr_1 += 2; // trampoline 4, 5
						else if (chr_1 == 12)
							chr_1 = sim->parts[r>>8].ctype; // trampoline N
						else
							chr_1 = 3; // trampoline 3
					}
					else
						chr_1 = 3;
					x += ix * chr_1; y += iy * chr_1;
				break;
				case 19: // random dir. w/o backward
					pack = rand() % 3;
					if (pack)
					{
						pack = pack * 2 - 3;
						chr_1 = ix; ix = iy * pack; iy = chr_1 * pack;
					}
				break;
				case 20: // function call (stack push)
					if (call_ptr >= 128)
					{
						std::cerr << "stack overflow!" << std::endl;
						errflag = 1;
						call_ptr = 0;
						x  = (int)(calls[0][0]);
						y  = (int)(calls[0][1]);
						ix = (int)(calls[0][2]);
						iy = (int)(calls[0][3]);
						cfptr = -1;
					}
					else
					{
						calls[call_ptr][0] = (short)(x+ix);
						calls[call_ptr][1] = (short)(y+iy);
						calls[call_ptr][2] = (short)ix;
						calls[call_ptr][3] = (short)iy;
						calls[call_ptr][4] = (short)cfptr;
						cfptr = call_ptr++;
					}
				break;
				case 21: // function return (stack pop)
					if (call_ptr <= 0)
						goto __break_loop_1;
					call_ptr = cfptr;
					x  = (int)(calls[call_ptr][0]);
					y  = (int)(calls[call_ptr][1]);
					ix = (int)(calls[call_ptr][2]);
					iy = (int)(calls[call_ptr][3]);
					cfptr = (int)(calls[call_ptr][4]);
				break;
				case 22: // push color data (stack push)
					if (call_ptr < 128)
					{
						calls[call_ptr][0] = (short)it_r;
						calls[call_ptr][1] = (short)it_g;
						calls[call_ptr][2] = (short)it_b;
						call_ptr++;
					}
					else
					{
						std::cerr << "stack overflow!" << std::endl;
						errflag = 1; call_ptr = 0;
					}
				break;
				case 23: // pop color data (stack pop)
					if (call_ptr <= 0)
						goto __break_loop_1;
					call_ptr --;
					it_r = (int)(calls[call_ptr][0]);
					it_g = (int)(calls[call_ptr][1]);
					it_b = (int)(calls[call_ptr][2]);
				break;
				case 24: // push counter register
					if (call_ptr < 128)
					{
						calls[call_ptr][0] = counter;
						call_ptr++;
					}
					else
					{
						std::cerr << "stack overflow!" << std::endl;
						errflag = 1; call_ptr = 0;
					}
				break;
				case 25: // pop counter register
					if (call_ptr <= 0)
						goto __break_loop_1;
					call_ptr --;
					counter = (int)(calls[call_ptr][0]);
				break;
				case 26: // if stack overflow then trampoline
					if (errflag) { x += ix; y += iy; }
				break;
				case 27: // if stack not overflow then trampoline
					if (!errflag) { x += ix; y += iy; }
				break;
				case 28: // if counter is non-zero then trampoline
					r = sim->pmap[y+iy][x+ix];
					if ((r & 0xFF) == PT_E189)
					{
						pack = sim->parts[r>>8].life;
						if (pack & ~0x1 == 0x2)
						{
							x += ix; y += iy;
						}
					}
					if (counter) { x += ix; y += iy; }
				break;
				case 29: // if counter is zero then trampoline
					if ((r & 0xFF) == PT_E189)
					{
						pack = sim->parts[r>>8].life;
						if (pack & ~0x1 == 0x2)
						{
							x += ix; y += iy;
						}
					}
					if (!counter) { x += ix; y += iy; }
				break;
				case 30: // counter increment by 1
					counter++;
				break;
				case 31: // counter decrement by 1
					counter--;
				break;
				case 32: // set counter value / set error flag
					r = sim->pmap[y+iy][x+ix];
					if ((r & 0xFF) == PT_E189)
					{
						pack = sim->parts[r>>8].life;
						if (pack == 12)
						{
							x += ix; y += iy;
							counter = (short)sim->parts[r>>8].ctype;
						}
						else if ((pack & ~0x1) == 0x2)
						{
							x += ix; y += iy;
							errflag = pack & 0x1;
						}
					}
				break;
				case 33: // toggle error flag
					errflag = !errflag;
				break;
				case 34: // push from stack
					call_ptr = (call_ptr + 1) % 128;
				break;
				case 35: // pop from stack
					call_ptr = (call_ptr + 127) % 128;
				break;
				case 36: // add constant
					r = sim->pmap[y+iy][x+ix];
					pack = sim->parts[r>>8].life;
					if ((r & 0xFF) == PT_E189 && pack == 12)
					{
						x += ix; y += iy;
						counter += (short)sim->parts[r>>8].ctype;
					}
					else
						counter += calls[call_ptr-1][0];
				break;
				case 37: // subtract constant
					r = sim->pmap[y+iy][x+ix];
					pack = sim->parts[r>>8].life;
					if ((r & 0xFF) == PT_E189 && pack == 12)
					{
						x += ix; y += iy;
						counter -= (short)sim->parts[r>>8].ctype;
					}
					else
						counter -= calls[call_ptr-1][0];
				break;
				case 38: // multiply constant
					r = sim->pmap[y+iy][x+ix];
					pack = sim->parts[r>>8].life;
					if ((r & 0xFF) == PT_E189 && pack == 12)
					{
						x += ix; y += iy;
						counter *= (short)sim->parts[r>>8].ctype;
					}
					else
						counter *= calls[call_ptr-1][0];
				break;
				case 39: // bitwise and
					r = sim->pmap[y+iy][x+ix];
					pack = sim->parts[r>>8].life;
					if ((r & 0xFF) == PT_E189 && pack == 12)
					{
						x += ix; y += iy;
						counter &= (short)sim->parts[r>>8].ctype;
					}
					else
						counter &= calls[call_ptr-1][0];
				break;
				case 40: // bitwise or
					r = sim->pmap[y+iy][x+ix];
					pack = sim->parts[r>>8].life;
					if ((r & 0xFF) == PT_E189 && pack == 12)
					{
						x += ix; y += iy;
						counter |= (short)sim->parts[r>>8].ctype;
					}
					else
						counter |= calls[call_ptr-1][0];
				break;
				case 41: // bitwise xor
					r = sim->pmap[y+iy][x+ix];
					pack = sim->parts[r>>8].life;
					if ((r & 0xFF) == PT_E189 && pack == 12)
					{
						x += ix; y += iy;
						counter ^= (short)sim->parts[r>>8].ctype;
					}
					else
						counter ^= calls[call_ptr-1][0];
				break;
				case 42: // bitwise shift
					r = sim->pmap[y+iy][x+ix];
					pack = sim->parts[r>>8].life;
					if ((r & 0xFF) == PT_E189 && pack == 12)
					{
						x += ix; y += iy;
						pack = sim->parts[r>>8].ctype;
					}
					else
						pack = calls[call_ptr-1][0];
					switch ((pack >> 4) & 3)
					{
						case 0: counter = counter << (pack & 0xF); break;
						case 1: counter = (signed short)counter >> (pack & 0xF); break;
						case 2: counter = (unsigned short)counter >> (pack & 0xF); break;
						case 3: counter = (counter << (pack & 0xF)) | ((unsigned short)counter >> (-pack & 0xF)); break;
					}
				break;
				case 43: // swap counter / stack
					pack = (int)calls[call_ptr-1][0];
					calls[call_ptr-1][0] = counter;
					counter = (short)pack;
				break;
				case 44: // if stack top less than counter then trampoline
					if (calls[call_ptr-1][0] < counter) { x += ix; y += iy; }
				break;
				case 45: // if counter less than stack top then trampoline
					if (calls[call_ptr-1][0] > counter) { x += ix; y += iy; }
				break;
				case 46: // if stack top and counter is equal then trampoline
					if (calls[call_ptr-1][0] == counter) { x += ix; y += iy; }
				break;
				case 47: // if stack top and counter is not equal then trampoline
					if (calls[call_ptr-1][0] != counter) { x += ix; y += iy; }
				break;
				case 48: // set random value
					 counter = rand();
				break;
				case 49: // push random value
					if (call_ptr < 128)
					{
						calls[call_ptr][0] = rand();
						call_ptr++;
					}
					else
					{
						std::cerr << "stack overflow!" << std::endl;
						errflag = 1; call_ptr = 0;
					}
				break;
				case 50: // get red channel
					counter = it_r;
				break;
				case 51: // set red channel
					it_r = counter;
				break;
				case 52: // get green channel
					counter = it_g;
				break;
				case 53: // set green channel
					it_g = counter;
				break;
				case 54: // get blue channel
					counter = it_b;
				break;
				case 55: // set blue channel
					it_b = counter;
				break;
				case 56: // get parameter
					r = sim->pmap[y+iy][x+ix];
					pack = sim->parts[r>>8].life;
					if ((r & 0xFF) == PT_E189 && pack == 12)
					{
						x += ix; y += iy;
						counter = calls[cfptr + sim->parts[r>>8].ctype][0];
					}
					else
						counter = calls[cfptr][0];
				break;
				case 57: // set parameter
					r = sim->pmap[y+iy][x+ix];
					pack = sim->parts[r>>8].life;
					if ((r & 0xFF) == PT_E189 && pack == 12)
					{
						x += ix; y += iy;
						calls[cfptr + sim->parts[r>>8].ctype][0] = counter;
					}
					else
						calls[cfptr][0] = counter;
				break;
				default:
					std::cerr << "Invalid opcode" << std::endl;
				return;
				}
			continue;
		}
		if (pack != 10)
			break;
		if (!esc)
		{
			switch (chr_1)
			{
			case 0: // no operation
				break;
			case 10: // "\n": newline
				ct_x = it_x;
				ct_y += FONT_H+2; // usually 12 pixels
				break;
			case 15:
				esc = 1;
				bkup = 0;
				break;
			case 256:
				esc = 2;
				break;
			case 257:
				esc = 3;
				break;
			case 258:
				esc = 4;
				break;
			case 259: // random character
				ct_x = E189_Update::AddCharacter(sim, ct_x, ct_y, (rand() & 0xFF), (it_r << 16) | (it_g << 8) | it_b);
				break;
			case 260: // random ASCII
				ct_x = E189_Update::AddCharacter(sim, ct_x, ct_y, (rand() % 95 + ' '), (it_r << 16) | (it_g << 8) | it_b);
				break;
			case 261: // random number
				ct_x = E189_Update::AddCharacter(sim, ct_x, ct_y, (rand() % 10 + '0'), (it_r << 16) | (it_g << 8) | it_b);
				break;
			case 262: // random uppercase
				ct_x = E189_Update::AddCharacter(sim, ct_x, ct_y, (rand() % 26 + 'A'), (it_r << 16) | (it_g << 8) | it_b);
				break;
			case 263: // random lowercase
				ct_x = E189_Update::AddCharacter(sim, ct_x, ct_y, (rand() % 26 + 'a'), (it_r << 16) | (it_g << 8) | it_b);
				break;
			case 264: // random mixed alphabet
				pack = rand();
				ct_x = E189_Update::AddCharacter(sim, ct_x, ct_y, ((pack >> 1) % 26 + ((pack % 2) << 5) + 'A'), (it_r << 16) | (it_g << 8) | it_b);
				break;
			case 265: // random mixed alphanumeric
				pack = rand() % 62;
				pack += ((pack > 36) ? 29 : (pack > 10) ? 87 : '0');
				ct_x = E189_Update::AddCharacter(sim, ct_x, ct_y, pack, (it_r << 16) | (it_g << 8) | it_b);
				break;
			case 266:
				ct_x = E189_Update::AddCharacter(sim, ct_x, ct_y, counter & 0xFF, (it_r << 16) | (it_g << 8) | it_b);
				break;
			case 267:
				ct_x = E189_Update::AddCharacter(sim, ct_x, ct_y, calls[call_ptr-1][0] & 0xFF, (it_r << 16) | (it_g << 8) | it_b);
				break;
			case 268: // print number
				chr_1 = (int)counter;
				pack = (it_r << 16) | (it_g << 8) | it_b;
				if (chr_1 < 0)
				{
					chr_1 = -chr_1; ct_x = E189_Update::AddCharacter(sim, ct_x, ct_y, '-', pack);
				}
				do
				{
					__digits[tmp++] = '0' + chr_1 % 10; // note: ascii '0' not number 0
					chr_1 /= 10;
				}
				while (chr_1);
				while (tmp)
					ct_x = E189_Update::AddCharacter(sim, ct_x, ct_y, __digits[--tmp], pack);
				break;
			case 269: // random punctuation (exclude space)
				pack = rand() & 31;
				pack += ((pack < 15) ? '!' : (pack < 22) ? 43 : (pack < 28) ? 69 : 95);
				ct_x = E189_Update::AddCharacter(sim, ct_x, ct_y, pack, (it_r << 16) | (it_g << 8) | it_b);
				break;
			case 270:
				Element_E189::useDefaultPart = !Element_E189::useDefaultPart;
				break;
			case 271: // horizontal tab
				diff = (ct_x - it_x) % tab_size;
				if (diff < 0) diff += tab_size; // can be non-branch
				ct_x += tab_size - diff;
				break;
			case 272: // set horizontal tab size
				esc = 6;
				break;
			default:
				if (chr_1 >= 0 && chr_1 <= 255)
					ct_x = E189_Update::AddCharacter(sim, ct_x, ct_y, chr_1, (it_r << 16) | (it_g << 8) | it_b);
				else
					std::cerr << "Invalid character" << std::endl;
			}
		}
		else
		{
			switch (esc)
			{
			case 1: // set color
				if (bkup)
				{
					oldr = it_r; oldg = it_g; oldb = it_b;
				}
				it_r = (chr_1 >> 16) & 0xFF;
				it_g = (chr_1 >> 8) & 0xFF;
				it_b =  chr_1 & 0xFF;
				esc = 0;
				break;
			case 2: // set location (fixed)
				ct_x =  chr_1 & 0xFFFF;
				ct_y = (chr_1 >> 16) & 0xFFFF;
				esc = 0;
				break;
			case 3: // reinitial location
				it_x =  chr_1 & 0xFFFF;
				ct_x =  it_x;
				ct_y = (chr_1 >> 16) & 0xFFFF;
				esc = 0;
				break;
			case 4: // set location (relative)
				ct_x += (signed short)( chr_1 & 0xFFFF);
				ct_y += (signed short)((chr_1 >> 16) & 0xFFFF);
				esc = 0;
				break;
			case 5: // packed
				pack = (it_r << 16) | (it_g << 8) | it_b;
				ct_x = E189_Update::AddCharacter(sim, ct_x, ct_y, chr_1 & 0xFF, pack);
				ct_x = E189_Update::AddCharacter(sim, ct_x, ct_y, (chr_1 >> 8) & 0xFF, pack);
				ct_x = E189_Update::AddCharacter(sim, ct_x, ct_y, (chr_1 >> 16) & 0xFF, pack);
				ct_x = E189_Update::AddCharacter(sim, ct_x, ct_y, (chr_1 >> 24) & 0xFF, pack);
				break;
			case 6: // set location (relative)
				tab_size = chr_1;
				esc = 0;
				break;
			}
		}
	}
	__break_loop_1:
	std::cout << "output complete!" << std::endl;
}

int E189_Update::AddCharacter(Simulation *sim, int x, int y, int c, int rgb)
{
	static int color_parts[4] = { PT_NONE, PT_TUNG, PT_SHLD1, PT_NWHL }; // particle colors
	
	int i, j, w, bn = 0, ba = 0, _r, xi, yj, _rt, _ri;
	unsigned char *rp = font_data + font_ptrs[c];
	w = *(rp++);
	for (j=0; j<FONT_H; j++)
		for (i=0; i<w; i++)
		{
			if (!bn)
			{
				ba = *(rp++);
				bn = 8;
			}
			if (ba & 3)
			{
				xi = x + i; yj = y + j;
				_r = sim->pmap[yj][xi];
				_rt = _r & 0xFF; // particle type
				_ri = _r >> 8; // particle ID
				if (_r)
				{
					if ((_rt) == PT_SPRK)
					{
						_rt = sim->parts[_ri].ctype & 0xFF; // reserved by sparked adamantium
					}
					if (_rt == PT_E189 && sim->parts[_ri].life == 13)
					{
						if (~ba & 3) // ba & 3 != 3, also only ba == 1 or ba == 2
						{
							int k = sim->parts[_ri].ctype;
							int olda = (k >> 24) & 0xFF;
							int oldr, oldg, oldb;
							if (olda == 255)
							{
								oldr = (k >> 16) & 0xFF;
								oldg = (k >> 8) & 0xFF;
								oldb = k & 0xFF;
							}
							else
							{
								oldr = (olda * ((k >> 16) & 0xFF)) >> 8;
								oldg = (olda * ((k >> 8) & 0xFF)) >> 8;
								oldb = (olda * (k & 0xFF)) >> 8;
							}
							olda = (ba & 3) * 0x55;
							int newr = (olda * ((rgb >> 16) & 0xFF) + (0xFF - olda) * oldr) & ~0xFF;
							int newg = (olda * ((rgb >> 8) & 0xFF) + (0xFF - olda) * oldg) & 0xFF00;
							int newb = (olda * (rgb & 0xFF) + (0xFF - olda) * oldb) >> 8;
							sim->parts[_ri].ctype = 0xFF000000 | newr << 8 | newg | newb;
						}
						else
							sim->parts[_ri].ctype = 0xFF000000 | (rgb & 0x00FFFFFF);
					}
					else if (!(sim->elements[_rt].Properties & PROP_NODESTRUCT))
					{
						goto _E189_recreatePixel;
					}
				}
				else
				{
				_E189_recreatePixel:
					if (!Element_E189::useDefaultPart)
						_r = sim->create_part(-1, xi, yj, PT_E189, 13); // type = 65549 (0x0001000D)
					else
						_r = sim->create_part(-1, xi, yj, color_parts[ba & 3]);
				}
				if (!Element_E189::useDefaultPart && _r >= 0)
				{
					sim->parts[_r].ctype = ((ba & 3) * 0x55000000) | (rgb & 0x00FFFFFF);
				}
			}
			ba >>= 2;
			bn -= 2;
		}
	return x + w;
}