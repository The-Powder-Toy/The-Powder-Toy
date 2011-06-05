#include <powder.h>
#include <console.h>

char pyready=1;
char pygood=1;
char console_more=0;
char console_error[255] = "";
int file_script = 0;

//takes a a string and compares it to element names, and puts it value into element.
int console_parse_type(char *txt, int *element, char *err)
{
	int i = -1;
	if (strcasecmp(txt,"WIND")==0)
	{
		if (err) strcpy(err, "Particle type not recognised");
		return 0;
	}
	// alternative names for some elements
	if (strcasecmp(txt,"C4")==0) i = PT_PLEX;
	else if (strcasecmp(txt,"C5")==0) i = PT_C5;
	else if (strcasecmp(txt,"NONE")==0) i = PT_NONE;
	if (i>=0)
	{
		*element = i;
		if (err) strcpy(err,"");
		return 1;
	}
	for (i=1; i<PT_NUM; i++) {
		if (strcasecmp(txt,ptypes[i].name)==0)
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



#ifdef PYCONSOLE
/*
 * PYTHON FUNCTIONS
 * instructions on making a function callable from python:
 * first you make a function that accepts (PyObject *self, PyObject *args) as arguments
 * then you use PyArg_ParseTuple to parse the arguments, handle everything the function should do.
 * register the function by adding a line to static PyMethodDef EmbMethods[] = {
 * the line should look like this:
 * {"PyFunctionname", Functionname, METH_VARARGS,"short help string"},
 * for more information on the PyWhatever functions look here:
 * http://docs.python.org/extending/extending.html
 */

//functions callable from python:

static PyObject* emb_create(PyObject *self, PyObject *args, PyObject *keywds)
{
	int x,y,t;
	char *name = "";
	char *kwlist[] = {"x","y","t","name", NULL};
	if (!PyArg_ParseTupleAndKeywords(args, keywds, "II|Is:create",kwlist, &x,&y,&t,&name))
		return NULL;
	//
	if (strcmp(name,"")!=0)
		console_parse_type(name, &t, console_error);
	return Py_BuildValue("i",create_part(-1,x,y,t));
}

static PyObject* emb_pause(PyObject *self, PyObject *args)
{
	int x,y,t;
	if (!PyArg_ParseTuple(args, ":unpause"))
		return NULL;
	//
	sys_pause=1;
	return Py_BuildValue("i",1);
}

static PyObject* emb_unpause(PyObject *self, PyObject *args)
{
	int x,y,t;
	if (!PyArg_ParseTuple(args, ":pause"))
		return NULL;
	//
	sys_pause=0;
	return Py_BuildValue("i",1);
}

static PyObject* emb_toggle_pause(PyObject *self, PyObject *args)
{
	int x,y,t;
	if (!PyArg_ParseTuple(args, ":toggle_pause"))
		return NULL;
	//
	sys_pause=!sys_pause;
	return Py_BuildValue("i",1);
}

//console_mode
static PyObject* emb_toggle_console(PyObject *self, PyObject *args)
{
	int x,y,t;
	if (!PyArg_ParseTuple(args, ":toggle_console"))
		return NULL;
	//
	console_mode=!console_mode;
	return Py_BuildValue("i",1);
}

static PyObject* emb_open_console(PyObject *self, PyObject *args)
{
	int x,y,t;
	if (!PyArg_ParseTuple(args, ":toggle_console"))
		return NULL;
	//
	console_mode=1;
	return Py_BuildValue("i",1);
}

static PyObject* emb_close_console(PyObject *self, PyObject *args)
{
	int x,y,t;
	if (!PyArg_ParseTuple(args, ":toggle_console"))
		return NULL;
	//
	console_mode=0;
	return Py_BuildValue("i",1);
}

static PyObject* emb_log(PyObject *self, PyObject *args)
{
	char *buffer;
	if (!PyArg_ParseTuple(args, "s:log",&buffer))
		return NULL;
	//
	strcpy(console_error,buffer);
	puts(buffer);
	return Py_BuildValue("i",1);
}

static PyObject* emb_console_more(PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple(args, ":log"))
		return NULL;
	//
	console_more=1;
	return Py_BuildValue("i",1);
}

static PyObject* emb_console_less(PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple(args, ":log"))
		return NULL;
	//
	console_more=0;
	return Py_BuildValue("i",1);
}

static PyObject* emb_reset_pressure(PyObject *self, PyObject *args)
{
	int nx, ny;
	if (!PyArg_ParseTuple(args, ":reset_pressure"))
		return NULL;
	//
	for (nx = 0; nx<XRES/CELL; nx++)
		for (ny = 0; ny<YRES/CELL; ny++)
		{
			pv[ny][nx] = 0;
		}
	return Py_BuildValue("i",1);
}

static PyObject* emb_reset_velocity(PyObject *self, PyObject *args)
{
	int nx, ny;
	if (!PyArg_ParseTuple(args, ":reset_velocity"))
		return NULL;
	//
	for (nx = 0; nx<XRES/CELL; nx++)
		for (ny = 0; ny<YRES/CELL; ny++)
		{
			vx[ny][nx] = 0;
			vy[ny][nx] = 0;
		}
	return Py_BuildValue("i",1);
}

static PyObject* emb_reset_sparks(PyObject *self, PyObject *args)
{
	int i;
	if (!PyArg_ParseTuple(args, ":reset_sparks"))
		return NULL;
	//
	for (i=0; i<NPART; i++)
	{
		if (parts[i].type==PT_SPRK)
		{
			parts[i].type = parts[i].ctype;
			parts[i].life = 4;
		}
	}
	return Py_BuildValue("i",1);
}

static PyObject* emb_set_life(PyObject *self, PyObject *args, PyObject *keywds)
{
	int i = -1,life,j,x=-1,y=-1;
	char *name = "";
	char *kwlist[] = {"setto", "setfrom", "i", "x", "y", NULL};
	if (!PyArg_ParseTupleAndKeywords(args, keywds, "I|sIII:set_type",kwlist ,&life,&name,&i,&x,&y))
		return NULL;
	//
	if (strcmp(name,"")==0 && x==-1 && y==-1 && i==-1)
		return Py_BuildValue("s","Need more args(coords,i,or a particle name)");
	if (strcmp(name,"all")==0)
	{
		for (i=0; i<NPART; i++)
		{
			if (parts[i].type)
				parts[i].life = life;
		}
	}
	else if (console_parse_type(name, &j, console_error))
	{
		for (i=0; i<NPART; i++)
		{
			if (parts[i].type == j)
				parts[i].life = life;
		}
	}
	else if (i!=-1)
	{
		if (parts[i].type != PT_NONE)
			parts[i].life = life;

	}
	else if (x!=-1 && y!=-1 && x>=0 && x<XRES && y>=0 && y<YRES)
	{
		if (parts[pmap[y][x]>>8].type != PT_NONE)
			parts[pmap[y][x]>>8].life = life;
	}
	return Py_BuildValue("i",1);
}

static PyObject* emb_set_type(PyObject *self, PyObject *args, PyObject *keywds)
{
	int i = -1,life,j=-1,x=-1,y=-1;
	char *name = "";
	char *type = "";
	char *kwlist[] = {"setto", "settoint", "setfrom", "i", "x", "y", NULL};
	if (!PyArg_ParseTupleAndKeywords(args, keywds, "|sIsIII:set_type",kwlist ,&type,&life,&name,&i,&x,&y))
		return NULL;
	//
	if (strcmp(name,"")==0 && x==-1 && y==-1 && i==-1 && j==-1)
		return Py_BuildValue("s","Need more args(coords,i,or a particle name)");
	console_parse_type(type, &life, console_error);
	if (strcmp(name,"all")==0)
	{
		for (i=0; i<NPART; i++)
		{
			if (parts[i].type)
				parts[i].type = life;
		}
	}
	else if (console_parse_type(name, &j, console_error))
	{
		for (i=0; i<NPART; i++)
		{
			if (parts[i].type == j)
				parts[i].type = life;
		}
	}
	else if (i!=-1)
	{
		if (parts[i].type != PT_NONE)
			parts[i].type = life;

	}
	else if (x!=-1 && y!=-1 && x>=0 && x<XRES && y>=0 && y<YRES)
	{
		if (parts[pmap[y][x]>>8].type != PT_NONE)
			parts[pmap[y][x]>>8].type = life;
	}
	return Py_BuildValue("i",1);
}

static PyObject* emb_set_temp(PyObject *self, PyObject *args, PyObject *keywds)
{
	int i = -1,j,x=-1,y=-1;
	float newval;
	char *name = "";
	char *kwlist[] = {"setto", "setfrom", "i", "x", "y", NULL};
	if (!PyArg_ParseTupleAndKeywords(args, keywds, "f|sIII:set_type", kwlist, &newval, &name, &i, &x, &y))
		return NULL;
	//
	if (strcmp(name,"")==0 && x==-1 && y==-1 && i==-1)
		return Py_BuildValue("s","Need more args(coords,i,or a particle name)");
	if (strcmp(name,"all")==0)
	{
		for (i=0; i<NPART; i++)
		{
			if (parts[i].type)
				parts[i].temp = newval;
		}
	}
	else if (console_parse_type(name, &j, console_error))
	{
		for (i=0; i<NPART; i++)
		{
			if (parts[i].type == j)
				parts[i].temp = newval;
		}
	}
	else if (i!=-1)
	{
		if (parts[i].type != PT_NONE)
			parts[i].temp = newval;

	}
	else if (x!=-1 && y!=-1 && x>=0 && x<XRES && y>=0 && y<YRES)
	{
		if (parts[pmap[y][x]>>8].type != PT_NONE)
			parts[pmap[y][x]>>8].temp = newval;
	}
	return Py_BuildValue("i",1);
}

static PyObject* emb_set_tmp(PyObject *self, PyObject *args, PyObject *keywds)
{
	int i = -1,life,j,x=-1,y=-1;
	char *name = "";
	char *kwlist[] = {"setto", "setfrom", "i", "x", "y", NULL};
	if (!PyArg_ParseTupleAndKeywords(args, keywds, "I|sIII:set_type",kwlist ,&life,&name,&i,&x,&y))
		return NULL;
	//
	if (strcmp(name,"")==0 && x==-1 && y==-1 && i==-1)
		return Py_BuildValue("s","Need more args(coords,i,or a particle name)");
	if (strcmp(name,"all")==0)
	{
		for (i=0; i<NPART; i++)
		{
			if (parts[i].type)
				parts[i].tmp = life;
		}
	}
	else if (console_parse_type(name, &j, console_error))
	{
		for (i=0; i<NPART; i++)
		{
			if (parts[i].type == j)
				parts[i].tmp = life;
		}
	}
	else if (i!=-1)
	{
		if (parts[i].type != PT_NONE)
			parts[i].tmp = life;

	}
	else if (x!=-1 && y!=-1 && x>=0 && x<XRES && y>=0 && y<YRES)
	{
		if (parts[pmap[y][x]>>8].type != PT_NONE)
			parts[pmap[y][x]>>8].tmp = life;
	}
	return Py_BuildValue("i",1);
}

static PyObject* emb_set_x(PyObject *self, PyObject *args, PyObject *keywds)
{
	int i = -1,life,j,x=-1,y=-1;
	char *name = "";
	char *type = "";
	char *kwlist[] = {"setto", "setfrom", "i", "x", "y", NULL};
	if (!PyArg_ParseTupleAndKeywords(args, keywds, "I|sIII:set_type",kwlist ,&life,&name,&i,&x,&y))
		return NULL;
	//
	if (strcmp(name,"")==0 && x==-1 && y==-1 && i==-1)
		return Py_BuildValue("s","Need more args(coords,i,or a particle name)");
	if (strcmp(name,"all")==0)
	{
		for (i=0; i<NPART; i++)
		{
			if (parts[i].type)
				parts[i].x = life;
		}
	}
	else if (console_parse_type(name, &j, console_error))
	{
		for (i=0; i<NPART; i++)
		{
			if (parts[i].type == j)
				parts[i].x = life;
		}
	}
	else if (i!=-1)
	{
		if (parts[i].type != PT_NONE)
			parts[i].x = life;

	}
	else if (x!=-1 && y!=-1 && x>=0 && x<XRES && y>=0 && y<YRES)
	{
		if (parts[pmap[y][x]>>8].type != PT_NONE)
			parts[pmap[y][x]>>8].x = life;
	}
	return Py_BuildValue("i",1);
}

static PyObject* emb_set_y(PyObject *self, PyObject *args, PyObject *keywds)
{
	int i = -1,life,j,x=-1,y=-1;
	char *name = "";
	char *kwlist[] = {"setto", "setfrom", "i", "x", "y", NULL};
	if (!PyArg_ParseTupleAndKeywords(args, keywds, "I|sIII:set_type",kwlist ,&life,&name,&i,&x,&y))
		return NULL;
	//
	if (strcmp(name,"")==0 && x==-1 && y==-1 && i==-1)
		return Py_BuildValue("s","Need more args(coords,i,or a particle name)");
	if (strcmp(name,"all")==0)
	{
		for (i=0; i<NPART; i++)
		{
			if (parts[i].type)
				parts[i].y = life;
		}
	}
	else if (console_parse_type(name, &j, console_error))
	{
		for (i=0; i<NPART; i++)
		{
			if (parts[i].type == j)
				parts[i].y = life;
		}
	}
	else if (i!=-1)
	{
		if (parts[i].type != PT_NONE)
			parts[i].y = life;

	}
	else if (x!=-1 && y!=-1 && x>=0 && x<XRES && y>=0 && y<YRES)
	{
		if (parts[pmap[y][x]>>8].type != PT_NONE)
			parts[pmap[y][x]>>8].y = life;
	}
	return Py_BuildValue("i",1);
}

static PyObject* emb_set_ctype(PyObject *self, PyObject *args, PyObject *keywds)
{
	int i = -1,life,j,x=-1,y=-1;
	char *name = "";
	char *type = "";
	char *kwlist[] = {"setto", "settoint", "setfrom", "i", "x", "y", NULL};
	if (!PyArg_ParseTupleAndKeywords(args, keywds, "|sIsIII:set_type",kwlist ,&type, &life, &name,&i,&x,&y))
		return NULL;
	//
	if (strcmp(name,"")==0 && x==-1 && y==-1 && i==-1)
		return Py_BuildValue("s","Need more args(coords,i,or a particle name)");
	if (!life)
		console_parse_type(type, &life, console_error);
	if (strcmp(name,"all")==0)
	{
		for (i=0; i<NPART; i++)
		{
			if (parts[i].type)
				parts[i].ctype = life;
		}
	}
	else if (console_parse_type(name, &j, console_error))
	{
		for (i=0; i<NPART; i++)
		{
			if (parts[i].type == j)
				parts[i].ctype = life;
		}
	}
	else if (i!=-1)
	{
		if (parts[i].type != PT_NONE)
			parts[i].ctype = life;

	}
	else if (x!=-1 && y!=-1 && x>=0 && x<XRES && y>=0 && y<YRES)
	{
		if (parts[pmap[y][x]>>8].type != PT_NONE)
			parts[pmap[y][x]>>8].ctype = life;
	}
	return Py_BuildValue("i",1);
}

static PyObject* emb_set_vx(PyObject *self, PyObject *args, PyObject *keywds)
{
	int i = -1,j,x=-1,y=-1;
	float life;
	char *name = "";
	char *kwlist[] = {"setto", "setfrom", "i", "x", "y", NULL};
	if (!PyArg_ParseTupleAndKeywords(args, keywds, "f|sIII:set_type",kwlist ,&life,&name,&i,&x,&y))
		return NULL;
	//
	if (strcmp(name,"")==0 && x==-1 && y==-1 && i==-1)
		return Py_BuildValue("s","Need more args(coords,i,or a particle name)");
	if (strcmp(name,"all")==0)
	{
		for (i=0; i<NPART; i++)
		{
			if (parts[i].type)
				parts[i].vx = life;
		}
	}
	else if (console_parse_type(name, &j, console_error))
	{
		for (i=0; i<NPART; i++)
		{
			if (parts[i].type == j)
				parts[i].vx = life;
		}
	}
	else if (i!=-1)
	{
		if (parts[i].type != PT_NONE)
			parts[i].vx = life;

	}
	else if (x!=-1 && y!=-1 && x>=0 && x<XRES && y>=0 && y<YRES)
	{
		if (parts[pmap[y][x]>>8].type != PT_NONE)
			parts[pmap[y][x]>>8].vx = life;
	}
	return Py_BuildValue("i",1);
}

static PyObject* emb_set_vy(PyObject *self, PyObject *args, PyObject *keywds)
{
	int i = -1,j,x=-1,y=-1;
	float life;
	char *name = "";
	char *kwlist[] = {"setto", "setfrom", "i", "x", "y", NULL};
	if (!PyArg_ParseTupleAndKeywords(args, keywds, "f|sIII:set_type",kwlist ,&life,&name,&i,&x,&y))
		return NULL;
	//
	if (strcmp(name,"")==0 && x==-1 && y==-1 && i==-1)
		return Py_BuildValue("s","Need more args(coords,i,or a particle name)");
	if (strcmp(name,"all")==0)
	{
		for (i=0; i<NPART; i++)
		{
			if (parts[i].type)
				parts[i].vy = life;
		}
	}
	else if (console_parse_type(name, &j, console_error))
	{
		for (i=0; i<NPART; i++)
		{
			if (parts[i].type == j)
				parts[i].vy = life;
		}
	}
	else if (i!=-1)
	{
		if (parts[i].type != PT_NONE)
			parts[i].vy = life;

	}
	else if (x!=-1 && y!=-1 && x>=0 && x<XRES && y>=0 && y<YRES)
	{
		if (parts[pmap[y][x]>>8].type != PT_NONE)
			parts[pmap[y][x]>>8].vy = life;
	}
	return Py_BuildValue("i",1);
}

static PyObject* emb_get_pmap(PyObject *self, PyObject *args)
{
	int x,y;
	if (!PyArg_ParseTuple(args, "II:get_pmap",&x,&y))
		return NULL;
	//
	if (x<0 || y<0 || x>=XRES || y>=YRES)
		return Py_BuildValue("i",-1);

	return Py_BuildValue("I",pmap[y][x]);
}

static PyObject* emb_get_prop(PyObject *self, PyObject *args)
{
	int i;
	char *prop = "";
	if (!PyArg_ParseTuple(args, "Is:get_pmap",&i,&prop))
		return NULL;
	//
	if (parts[i].type)
	{
		if (strcmp(prop,"type")==0)
			return Py_BuildValue("i",parts[i].type);
		if (strcmp(prop,"life")==0)
			return Py_BuildValue("i",parts[i].life);
		if (strcmp(prop,"ctype")==0)
			return Py_BuildValue("i",parts[i].ctype);
		if (strcmp(prop,"temp")==0)
			return Py_BuildValue("i",parts[i].temp);
		if (strcmp(prop,"tmp")==0)
			return Py_BuildValue("i",parts[i].tmp);
		if (strcmp(prop,"vy")==0)
			return Py_BuildValue("f",parts[i].vy);
		if (strcmp(prop,"vx")==0)
			return Py_BuildValue("f",parts[i].vx);
		if (strcmp(prop,"x")==0)
			return Py_BuildValue("i",parts[i].x);
		if (strcmp(prop,"y")==0)
			return Py_BuildValue("i",parts[i].y);
	}

	return Py_BuildValue("i",-1);
}

static PyObject* emb_draw_pixel(PyObject *self, PyObject *args)
{
	int x,y,r,g,b,a;
	a=255;
	if (!PyArg_ParseTuple(args, "IIIII|I:draw_pixel",&x,&y,&r,&g,&b,&a))
		return NULL;

	if (vid_buf!=NULL)
	{
		drawpixel(vid_buf,x,y,r,g,b,a);
		return Py_BuildValue("i",1);
	}
	return Py_BuildValue("i",-1);

}

static PyObject* emb_draw_text(PyObject *self, PyObject *args)
{
	int x,y,r,g,b,a;
	char *txt;
	a=255;
	if (!PyArg_ParseTuple(args, "IIsIII|I:draw_text",&x,&y,&txt,&r,&g,&b,&a))
		return NULL;
	if (vid_buf!=NULL)
	{
		drawtext(vid_buf,x,y,txt,r,g,b,a);
		return Py_BuildValue("i",1);
	}
	return Py_BuildValue("i",-1);
}

static PyObject* emb_draw_rect(PyObject *self, PyObject *args)
{
	int x,y,w,h,r,g,b,a;
	a=255;
	if (!PyArg_ParseTuple(args, "IIIIIII|I:draw_rect",&x,&y,&w,&h,&r,&g,&b,&a))
		return NULL;
	if (vid_buf!=NULL)
	{
		drawrect(vid_buf,x,y,w,h,r,g,b,a);
		//fillrect
		return Py_BuildValue("i",1);
	}
	return Py_BuildValue("i",-1);
}

static PyObject* emb_draw_fillrect(PyObject *self, PyObject *args)
{
	int x,y,w,h,r,g,b,a;
	a=255;
	if (!PyArg_ParseTuple(args, "IIIIIII|I:draw_fillrect",&x,&y,&w,&h,&r,&g,&b,&a))
		return NULL;
	if (vid_buf!=NULL)
	{
		fillrect(vid_buf,x,y,w,h,r,g,b,a);
		//fillrect
		return Py_BuildValue("i",1);
	}
	return Py_BuildValue("i",-1);
}

static PyObject* emb_get_width(PyObject *self, PyObject *args)
{
	char *txt;
	if (!PyArg_ParseTuple(args, "s:get_width",&txt))
		return NULL;
	return Py_BuildValue("i",textwidth(txt));
}

static PyObject* emb_get_mouse(PyObject *self, PyObject *args)
{
	int x,y,mask,b1,b2,b3;
	if (!PyArg_ParseTuple(args, ":get_mouse"))
		return NULL;
	mask=SDL_GetMouseState(&x, &y);
	b1=mask&SDL_BUTTON(1);
	b2=mask&SDL_BUTTON(2);
	b3=mask&SDL_BUTTON(3);
	return Py_BuildValue("(ii(iii))",x,y,b1,b2,b3);
}

static PyObject* emb_get_name(PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple(args, ":get_name"))
		return NULL;
	if (svf_login)
		return Py_BuildValue("s",svf_user);
	else
		return Py_BuildValue("s","");
}

static PyObject* emb_shortcuts_disable(PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple(args, ":shortcuts_disable"))
		return NULL;
	//
	sys_shortcuts=0;
	return Py_BuildValue("i",1);
}

static PyObject* emb_shortcuts_enable(PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple(args, ":shortcuts_enable"))
		return NULL;
	//
	sys_shortcuts=1;
	return Py_BuildValue("i",1);
}

static PyObject* emb_get_modifier(PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple(args, ":get_modifier"))
		return NULL;
	return Py_BuildValue("(iiiiii)",sdl_mod&KMOD_LCTRL,sdl_mod&KMOD_RCTRL,sdl_mod&KMOD_LALT,sdl_mod&KMOD_RALT,sdl_mod&KMOD_LSHIFT,sdl_mod&KMOD_RSHIFT);
}

static PyObject* emb_set_keyrepeat(PyObject *self, PyObject *args)
{
	int keydelay,keyinterval;
	keydelay=SDL_DEFAULT_REPEAT_DELAY;
	keyinterval=SDL_DEFAULT_REPEAT_INTERVAL;
	if (!PyArg_ParseTuple(args, "|ii:set_keyrepeat",&keydelay,&keyinterval))
		return NULL;
	return Py_BuildValue("i",SDL_EnableKeyRepeat(keydelay,keyinterval));
}

//delete_part
static PyObject* emb_delete(PyObject *self, PyObject *args)
{
	int x,y;
	if (!PyArg_ParseTuple(args, "ii:delete",&x,&y))
		return NULL;
	delete_part(x,y);
	return Py_BuildValue("i",1);
}

static PyObject* emb_set_pressure(PyObject *self, PyObject *args)
{
	int x,y,press;
	if (!PyArg_ParseTuple(args, "iii:set_pressure",&x,&y,&press))
		return NULL;
	pv[y/CELL][x/CELL]=press;
	return Py_BuildValue("i",1);
}

static PyObject* emb_set_velocity(PyObject *self, PyObject *args)
{
	int x,y,xv,yv;
	if (!PyArg_ParseTuple(args, "iiii:set_velocity",&x,&y,&xv,&yv))
		return NULL;
	vx[y/CELL][x/CELL]=xv;
	vy[y/CELL][x/CELL]=yv;
	return Py_BuildValue("i",1);
}

static PyObject* emb_disable_python(PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple(args, ":disable_python"))
		return NULL;
	pyready=0;
	return Py_BuildValue("i",1);
}

int bsx = 2, bsy = 2, sl=1, sr=0;
static PyObject*
emb_get_tool(PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple(args, ":get_tool"))
		return NULL;
	return Py_BuildValue("((ii)(ii)i)",bsx,bsy,sl,sr,CURRENT_BRUSH);
}

static PyObject*
emb_set_tool(PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple(args, "((ii)(ii)i):set_tool",&bsx,&bsy,&sl,&sr,&CURRENT_BRUSH))
		return NULL;
	return Py_BuildValue("i",1);
}

/*
static PyObject*
emb_press_mouse(PyObject *self, PyObject *args)
{
    int x,y,b;
    SDL_Event* ev;
    b=0;
    if(!PyArg_ParseTuple(args, "ii|i:handle_tool",&x,&y,&b))
        return NULL;
    ev.type=SDL_MOUSEBUTTONDOWN;
    if(b==2)
        ev.button.button=SDL_BUTTON_RIGHT;
    else
        ev.button.button=SDL_BUTTON_LEFT;
    ev.button.state=SDL_PRESSED;
    ev.button.x=x;
    ev.button.y=y;
    return Py_BuildValue("i",SDL_PushEvent(ev));
}

static PyObject*
emb_release_mouse(PyObject *self, PyObject *args)
{
    int x,y,b;
    SDL_MouseButtonEvent ev;
    b=0;
    if(!PyArg_ParseTuple(args, "ii|i:handle_tool",&x,&y,&b))
        return NULL;
    ev.type=SDL_MOUSEBUTTONUP;
    if(b==2)
        ev.button.button=SDL_BUTTON_RIGHT;
    else
        ev.button.button=SDL_BUTTON_LEFT;
    ev.button.state=SDL_RELEASED;
    ev.button.x=x;
    ev.button.y=y;
    return Py_BuildValue("i",SDL_PushEvent(ev));
}*/
PyMethodDef EmbMethods[] = { //WARNING! don't forget to register your function here!
	{"create",		    (PyCFunction)emb_create, 		METH_VARARGS|METH_KEYWORDS,	"create a particle."},
	{"log", 		    (PyCFunction)emb_log, 		METH_VARARGS,			"logs an error string to the console."},
	{"reset_pressure", 	(PyCFunction)emb_reset_pressure, 	METH_VARARGS,			"resets all the pressure."},
	{"reset_velocity", 	(PyCFunction)emb_reset_velocity, 	METH_VARARGS,			"resets all the velocity."},
	{"reset_sparks", 	(PyCFunction)emb_reset_sparks, 	METH_VARARGS,			"resets all the sparks."},
	{"set_life",	    (PyCFunction)emb_set_life, 		METH_VARARGS|METH_KEYWORDS,	"sets life of a specified particle."},
	{"set_type", 	    (PyCFunction)emb_set_type, 		METH_VARARGS|METH_KEYWORDS,	"sets type of a specified particle."},
	{"set_temp", 	    (PyCFunction)emb_set_temp, 		METH_VARARGS|METH_KEYWORDS,	"sets temp of a specified particle."},
	{"set_tmp", 	    (PyCFunction)emb_set_tmp, 		METH_VARARGS|METH_KEYWORDS,	"sets tmp of a specified particle."},
	{"set_x", 		    (PyCFunction)emb_set_x, 		METH_VARARGS|METH_KEYWORDS,	"sets x of a specified particle."},
	{"set_y", 		    (PyCFunction)emb_set_y, 		METH_VARARGS|METH_KEYWORDS,	"sets y of a specified particle."},
	{"set_ctype",   	(PyCFunction)emb_set_ctype, 		METH_VARARGS|METH_KEYWORDS,	"sets ctype of a specified particle."},
	{"set_vx", 		    (PyCFunction)emb_set_vx, 		METH_VARARGS|METH_KEYWORDS,	"sets vx of a specified particle."},
	{"set_vy", 		    (PyCFunction)emb_set_vy, 		METH_VARARGS|METH_KEYWORDS,	"sets vy of a specified particle."},
	{"pause", 		    (PyCFunction)emb_pause, 		METH_VARARGS,			"pause the game."},
	{"unpause", 	    (PyCFunction)emb_unpause, 		METH_VARARGS,			"unpause the game."},
	{"pause_toggle", 	(PyCFunction)emb_toggle_pause, 	METH_VARARGS,			"toggle game pause."},
	{"console_open", 	(PyCFunction)emb_open_console, 	METH_VARARGS,			"open the game console."},
	{"console_close", 	(PyCFunction)emb_close_console, 	METH_VARARGS,			"close the game console."},
	{"console_toggle", 	(PyCFunction)emb_toggle_console, 	METH_VARARGS,			"toggle the game console."},
	{"console_more", 	(PyCFunction)emb_console_more, 	METH_VARARGS,			"turns the more indicator on."},
	{"console_less", 	(PyCFunction)emb_console_less, 	METH_VARARGS,			"turns the more indicator off."},
	{"get_pmap", 	    (PyCFunction)emb_get_pmap, 		METH_VARARGS,			"get the pmap value."},
	{"get_prop", 	    (PyCFunction)emb_get_prop, 		METH_VARARGS,			"get some properties."},
	{"draw_pixel",      (PyCFunction)emb_draw_pixel,       METH_VARARGS,           "draw a pixel."},
	{"draw_text",       (PyCFunction)emb_draw_text,       METH_VARARGS,           "draw some text."},
	{"draw_rect",       (PyCFunction)emb_draw_rect,       METH_VARARGS,           "draw a rect."},
	{"draw_fillrect",   (PyCFunction)emb_draw_fillrect,       METH_VARARGS,           "draw a rect."},
	{"get_width",       (PyCFunction)emb_get_width,       METH_VARARGS,           "get string width."},
	{"get_mouse",       (PyCFunction)emb_get_mouse,       METH_VARARGS,           "get mouse status."},
	{"get_name",        (PyCFunction)emb_get_name,       METH_VARARGS,           "get name of logged in user"},
	{"shortcuts_disable",    (PyCFunction)emb_shortcuts_disable,       METH_VARARGS,           "disable keyboard shortcuts"},
	{"shortcuts_enable",     (PyCFunction)emb_shortcuts_enable,       METH_VARARGS,           "enable keyboard shortcuts"},
	{"get_modifier",         (PyCFunction)emb_get_modifier,       METH_VARARGS,           "get pressed modifier keys"},
	{"set_keyrepeat",        (PyCFunction)emb_set_keyrepeat,       METH_VARARGS,           "set key repeat rate."},
	{"delete",        (PyCFunction)emb_delete,       METH_VARARGS,           "delete a particle"},
	{"set_pressure",        (PyCFunction)emb_set_pressure,       METH_VARARGS,           "set pressure"},
	{"set_velocity",        (PyCFunction)emb_set_velocity,       METH_VARARGS,           "set velocity"},
	{"disable_python",        (PyCFunction)emb_disable_python,       METH_VARARGS,           "switch back to the old console."},
	{"get_tool",        (PyCFunction)emb_get_tool,       METH_VARARGS,           "get tool size/type and selected particles"},
	{"set_tool",        (PyCFunction)emb_set_tool,       METH_VARARGS,           "set tool size/type and selected particles"},
	{NULL, NULL, 0, NULL}
};
#endif

#ifdef PYCONSOLE
int process_command(pixel *vid_buf, char *console, char *console_error, PyObject *pfunc)
{
	int y,x,nx,ny,i,j,k,m;
	int do_next = 1;
	char xcoord[10];
	char ycoord[10];
	char console2[15];
	char console3[15];
	char console4[15];
	char console5[15];
	PyObject *pvalue,*pargs;
	//sprintf(console_error, "%s", console);
	if (console && strcmp(console, "")!=0 && strncmp(console, " ", 1)!=0)
	{
		sscanf(console,"%14s %14s %14s %14s", console2, console3, console4, console5);//why didn't i know about this function?!
		if (strcmp(console2, "quit")==0)
		{
			return -1;
		} else {
			//handle them command
			pargs=Py_BuildValue("(s)",console);
			pvalue = PyObject_CallObject(pfunc, pargs);
			Py_DECREF(pargs);
			pargs=NULL;
			if (pvalue==NULL)
				strcpy(console_error,"failed to execute code.");
			pvalue=NULL;
		}
	}
	return 1;
}
#endif
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
		else if (strcmp(console2, "python")==0)
			if (pygood==1)
				pyready=1;
			else
				strcpy(console_error, "python not ready. check stdout for more info.");
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
