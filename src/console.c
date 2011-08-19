#include <powder.h>
#include <console.h>
#include <math.h>

#ifdef PYCONSOLE
#include <pythonconsole.h>
#endif

char console_more=0;
char console_error[255] = "";
int file_script = 0;

//takes a a string and compares it to element names, and puts it value into element.
int console_parse_type(char *txt, int *element, char *err)
{
	int i = -1;
	// alternative names for some elements
	if (strcasecmp(txt,"C4")==0) i = PT_PLEX;
	else if (strcasecmp(txt,"C5")==0) i = PT_C5;
	else if (strcasecmp(txt,"NONE")==0) i = PT_NONE;
	if (i>=0 && i<PT_NUM && ptypes[i].enabled)
	{
		*element = i;
		if (err) strcpy(err,"");
		return 1;
	}
	for (i=1; i<PT_NUM; i++) {
		if (strcasecmp(txt,ptypes[i].name)==0 && ptypes[i].enabled)
		{
			*element = i;
			if (err) strcpy(err,"");
			return 1;
		}
	}
	if (err) strcpy(err, "Particle type not recognised");
	return 0;
}
//takes a string of coords "x,y" and puts the values into x and y.
int console_parse_coords(char *txt, int *x, int *y, char *err)
{
	int nx = -1, ny = -1;
	if (sscanf(txt,"%d,%d",&nx,&ny)!=2 || nx<0 || nx>=XRES || ny<0 || ny>=YRES)
	{
		if (err) strcpy(err,"Invalid coordinates");
		return 0;
	}
	*x = nx;
	*y = ny;
	return 1;
}
//takes a string of either coords or a particle number, and puts the particle number into *which
int console_parse_partref(char *txt, int *which, char *err)
{
	int i = -1, nx, ny;
	if (err) strcpy(err,"");
	if (strchr(txt,',') && console_parse_coords(txt, &nx, &ny, err))
	{
		i = pmap[ny][nx];
		if (!i || (i>>8)>=NPART)
			i = -1;
		else
			i = i>>8;
	}
	else if (txt)
	{
		char *num = (char*)malloc(strlen(txt)+3);
		i = atoi(txt);
		sprintf(num,"%d",i);
		if (!txt || strcmp(txt,num)!=0)
			i = -1;
		free(num);
	}
	if (i>=0 && i<NPART && parts[i].type)
	{
		*which = i;
		if (err) strcpy(err,"");
		return 1;
	}
	if (err && strcmp(err,"")==0) strcpy(err,"Particle does not exist");
	return 0;
}

int process_command_old(pixel *vid_buf, char *console, char *console_error)
{
	int y,x,nx,ny,i,j,k,m;
	float f;
	int do_next = 1;
	char xcoord[10] = "";
	char ycoord[10] = "";
	char console2[15] = "";
	char console3[15] = "";
	char console4[15] = "";
	char console5[15] = "";
	//sprintf(console_error, "%s", console);
	if (console && strcmp(console, "")!=0 && strncmp(console, " ", 1)!=0)
	{
		sscanf(console,"%14s %14s %14s %14s", console2, console3, console4, console5);//why didn't i know about this function?!
		if (strcmp(console2, "quit")==0)
		{
			return -1;
		}
		else if (strcmp(console2, "file")==0 && console3[0])
		{
			if (file_script) {
				int filesize;
				char *fileread = file_load(console3, &filesize);
				nx = 0;
				ny = 0;
				if (console4[0] && !console_parse_coords(console4, &nx , &ny, console_error))
				{
					free(fileread);
					return 1;
				}
				if (fileread)
				{
					char pch[501];
					char tokens[31];
					int tokensize;
					j = 0; // line start position in fileread
					m = 0; // token start position in fileread
					memset(pch,0,sizeof(pch));
					for (i=0; i<filesize; i++)
					{
						if (fileread[i] != '\n' && i-m<30)
						{
							pch[i-j] = fileread[i];
							if (fileread[i] != ' ')
								tokens[i-m] = fileread[i];
						}
						if ((fileread[i] == ' ' || fileread[i] == '\n') && i-j<400)
						{
							if (sregexp(tokens,"^x.\\{0,1\\}[0-9]*,y.\\{0,1\\}[0-9]*")==0)
							{
								int starty = 0;
								tokensize = strlen(tokens);
								x = 0;
								y = 0;
								if (tokens[1]!=',')
									sscanf(tokens,"x%d,y%d",&x,&y);
								else
									sscanf(tokens,"x,y%d",&y);
								x += nx;
								y += ny;
								sprintf(xcoord,"%d",x);
								sprintf(ycoord,"%d",y);
								for (k = 0; k<strlen(xcoord); k++)//rewrite pch with numbers
								{
									pch[i-j-tokensize+k] = xcoord[k];
									starty = k+1;
								}
								pch[i-j-tokensize+starty] = ',';
								starty++;
								for (k=0; k<strlen(ycoord); k++)
								{
									pch[i-j-tokensize+starty+k] = ycoord[k];
								}
								pch[i-j-tokensize +strlen(xcoord) +1 +strlen(ycoord)] = ' ';
								j = j -tokensize +strlen(xcoord) +1 +strlen(ycoord);
							}
							memset(tokens,0,sizeof(tokens));
							m = i+1;
						}
						if (fileread[i] == '\n')
						{
							
							if (do_next)
							{
								if (strcmp(pch,"else")==0)
									do_next = 0;
								else
									do_next = process_command_old(vid_buf, pch, console_error);
							}
							else if (strcmp(pch,"endif")==0 || strcmp(pch,"else")==0)
								do_next = 1;
							memset(pch,0,sizeof(pch));
							j = i+1;
						}
					}
					free(fileread);
				}
				else
				{
					sprintf(console_error, "%s does not exist", console3);
				}
			}
			else
			{
				sprintf(console_error, "Scripts are not enabled");
			}
			
		}
		else if (strcmp(console2, "sound")==0 && console3[0])
		{
			if (sound_enable) play_sound(console3);
			else strcpy(console_error, "Audio device not available - cannot play sounds");
		}
#ifdef PYCONSOLE
		else if (strcmp(console2, "python")==0)
			if (pygood==1)
				pyready=1;
			else
				strcpy(console_error, "python not ready. check stdout for more info.")
#endif
			else if (strcmp(console2, "load")==0 && console3[0])
			{
				j = atoi(console3);
				if (j)
				{
					open_ui(vid_buf, console3, NULL);
					console_mode = 0;
				}
			}
			else if (strcmp(console2, "if")==0 && console3[0])
			{
				if (strcmp(console3, "type")==0)//TODO: add more than just type, and be able to check greater/less than
				{
					if (console_parse_partref(console4, &i, console_error)
				        && console_parse_type(console5, &j, console_error))
					{
						if (parts[i].type==j)
							return 1;
						else
							return 0;
					}
					else
						return 0;
				}
			}
			else if (strcmp(console2, "create")==0 && console3[0] && console4[0])
			{
				if (console_parse_type(console3, &j, console_error)
			        && console_parse_coords(console4, &nx, &ny, console_error))
				{
					if (!j)
						strcpy(console_error, "Cannot create particle with type NONE");
					else if (create_part(-1,nx,ny,j)<0)
						strcpy(console_error, "Could not create particle");
				}
			}
			else if (strcmp(console2, "bubble")==0 && console3[0])
			{
				if (console_parse_coords(console3, &nx, &ny, console_error))
				{
					int first, rem1, rem2;

					first = create_part(-1, nx+18, ny, PT_SOAP);
					rem1 = first;

					for (i = 1; i<=30; i++)
					{
						rem2 = create_part(-1, nx+18*cosf(i/5.0), ny+18*sinf(i/5.0), PT_SOAP);

						parts[rem1].ctype = 7;
						parts[rem1].tmp = rem2;
						parts[rem2].tmp2 = rem1;

						rem1 = rem2;
					}

					parts[rem1].ctype = 7;
					parts[rem1].tmp = first;
					parts[first].tmp2 = rem1;
					parts[first].ctype = 7;
				}
			}
			else if ((strcmp(console2, "delete")==0 || strcmp(console2, "kill")==0) && console3[0])
			{
				if (console_parse_partref(console3, &i, console_error))
					kill_part(i);
			}
			else if (strcmp(console2, "reset")==0 && console3[0])
			{
				if (strcmp(console3, "pressure")==0)
				{
					for (nx = 0; nx<XRES/CELL; nx++)
						for (ny = 0; ny<YRES/CELL; ny++)
						{
							pv[ny][nx] = 0;
						}
				}
				else if (strcmp(console3, "velocity")==0)
				{
					for (nx = 0; nx<XRES/CELL; nx++)
						for (ny = 0; ny<YRES/CELL; ny++)
						{
							vx[ny][nx] = 0;
							vy[ny][nx] = 0;
						}
				}
				else if (strcmp(console3, "sparks")==0)
				{
					for (i=0; i<NPART; i++)
					{
						if (parts[i].type==PT_SPRK)
						{
							parts[i].type = parts[i].ctype;
							parts[i].life = 4;
						}
					}
				}
				else if (strcmp(console3, "temp")==0)
				{
					for (i=0; i<NPART; i++)
					{
						if (parts[i].type)
						{
							parts[i].temp = ptypes[parts[i].type].heat;
						}
					}
				}
			}
			else if (strcmp(console2, "set")==0 && console3[0] && console4[0] && console5[0])
			{
				if (strcmp(console3, "life")==0)
				{
					if (strcmp(console4, "all")==0)
					{
						j = atoi(console5);
						for (i=0; i<NPART; i++)
						{
							if (parts[i].type)
								parts[i].life = j;
						}
					}
					else if (console_parse_type(console4, &j, console_error))
					{
						k = atoi(console5);
						for (i=0; i<NPART; i++)
						{
							if (parts[i].type == j)
								parts[i].life = k;
						}
					}
					else
					{
						if (console_parse_partref(console4, &i, console_error))
						{
							j = atoi(console5);
							parts[i].life = j;
						}
					}
				}
				if (strcmp(console3, "type")==0)
				{
					if (strcmp(console4, "all")==0)
					{
						if (console_parse_type(console5, &j, console_error))
							for (i=0; i<NPART; i++)
							{
								if (parts[i].type)
									parts[i].type = j;
							}
					}
					else if (console_parse_type(console4, &j, console_error)
							 && console_parse_type(console5, &k, console_error))
					{
						for (i=0; i<NPART; i++)
						{
							if (parts[i].type == j)
								parts[i].type = k;
						}
					}
					else
					{
						if (console_parse_partref(console4, &i, console_error)
					        && console_parse_type(console5, &j, console_error))
						{
							parts[i].type = j;
						}
					}
				}
				if (strcmp(console3, "temp")==0)
				{
					if (strcmp(console4, "all")==0)
					{
						f = atof(console5);
						for (i=0; i<NPART; i++)
						{
							if (parts[i].type)
								parts[i].temp = f;
						}
					}
					else if (console_parse_type(console4, &j, console_error))
					{
						f = atof(console5);
						for (i=0; i<NPART; i++)
						{
							if (parts[i].type == j)
								parts[i].temp= f;
						}
					}
					else
					{
						if (console_parse_partref(console4, &i, console_error))
						{
							f = atof(console5);
							parts[i].temp = f;
						}
					}
				}
				if (strcmp(console3, "tmp")==0)
				{
					if (strcmp(console4, "all")==0)
					{
						j = atoi(console5);
						for (i=0; i<NPART; i++)
						{
							if (parts[i].type)
								parts[i].tmp = j;
						}
					}
					else if (console_parse_type(console4, &j, console_error))
					{
						k = atoi(console5);
						for (i=0; i<NPART; i++)
						{
							if (parts[i].type == j)
								parts[i].tmp = k;
						}
					}
					else
					{
						if (console_parse_partref(console4, &i, console_error))
						{
							j = atoi(console5);
							parts[i].tmp = j;
						}
					}
				}
				if (strcmp(console3, "x")==0)
				{
					if (strcmp(console4, "all")==0)
					{
						j = atoi(console5);
						for (i=0; i<NPART; i++)
						{
							if (parts[i].type)
								parts[i].x = j;
						}
					}
					else if (console_parse_type(console4, &j, console_error))
					{
						k = atoi(console5);
						for (i=0; i<NPART; i++)
						{
							if (parts[i].type == j)
								parts[i].x = k;
						}
					}
					else
					{
						if (console_parse_partref(console4, &i, console_error))
						{
							j = atoi(console5);
							parts[i].x = j;
						}
					}
				}
				if (strcmp(console3, "y")==0)
				{
					if (strcmp(console4, "all")==0)
					{
						j = atoi(console5);
						for (i=0; i<NPART; i++)
						{
							if (parts[i].type)
								parts[i].y = j;
						}
					}
					else if (console_parse_type(console4, &j, console_error))
					{
						k = atoi(console5);
						for (i=0; i<NPART; i++)
						{
							if (parts[i].type == j)
								parts[i].y = k;
						}
					}
					else
					{
						if (console_parse_partref(console4, &i, console_error))
						{
							j = atoi(console5);
							parts[i].y = j;
						}
					}
				}
				if (strcmp(console3, "ctype")==0)
				{
					if (strcmp(console4, "all")==0)
					{
						if (console_parse_type(console5, &j, console_error))
							for (i=0; i<NPART; i++)
							{
								if (parts[i].type)
									parts[i].ctype = j;
							}
					}
					else if (console_parse_type(console4, &j, console_error)
							 && console_parse_type(console5, &k, console_error))
					{
						for (i=0; i<NPART; i++)
						{
							if (parts[i].type == j)
								parts[i].ctype = k;
						}
					}
					else
					{
						if (console_parse_partref(console4, &i, console_error)
					        && console_parse_type(console5, &j, console_error))
						{
							parts[i].ctype = j;
						}
					}
				}
				if (strcmp(console3, "vx")==0)
				{
					if (strcmp(console4, "all")==0)
					{
						f = atof(console5);
						for (i=0; i<NPART; i++)
						{
							if (parts[i].type)
								parts[i].vx = f;
						}
					}
					else if (console_parse_type(console4, &j, console_error))
					{
						f = atof(console5);
						for (i=0; i<NPART; i++)
						{
							if (parts[i].type == j)
								parts[i].vx = f;
						}
					}
					else
					{
						if (console_parse_partref(console4, &i, console_error))
						{
							f = atof(console5);
							parts[i].vx = f;
						}
					}
				}
				if (strcmp(console3, "vy")==0)
				{
					if (strcmp(console4, "all")==0)
					{
						f = atof(console5);
						for (i=0; i<NPART; i++)
						{
							if (parts[i].type)
								parts[i].vy = f;
						}
					}
					else if (console_parse_type(console4, &j, console_error))
					{
						f = atof(console5);
						for (i=0; i<NPART; i++)
						{
							if (parts[i].type == j)
								parts[i].vy = f;
						}
					}
					else
					{
						if (console_parse_partref(console4, &i, console_error))
						{
							f = atof(console5);
							parts[i].vy = f;
						}
					}
				}
			}
			else
				strcpy(console_error, "Invalid Command");
	}
	return 1;
}
