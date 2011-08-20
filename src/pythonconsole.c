#ifdef PYCONSOLE
#include <powder.h>
#include <console.h>
#include <pythonconsole.h>

char pyready=1;
char pygood=1;
PyObject *pname,*pmodule,*pfunc,*pvalue,*pargs,*pstep,*pkey;
PyObject *tpt_console_obj;
void pycon_open(){
	//initialise python console
	Py_Initialize();
	PyRun_SimpleString("print 'python present.'");
	Py_InitModule("tpt", EmbMethods);

	//change the path to find all the correct modules
	PyRun_SimpleString("import sys\nsys.path.append('./tptPython.zip')\nsys.path.append('.')");
	//load the console module and whatnot
#ifdef PYEXT
	PyRun_SimpleString(tpt_console_py);
	printf("using external python console file.\n");
	pname=PyString_FromString("tpt_console");//create string object
	pmodule = PyImport_Import(pname);//import module
	Py_DECREF(pname);//throw away string
#else
	tpt_console_obj = PyMarshal_ReadObjectFromString(tpt_console_pyc+8, sizeof(tpt_console_pyc)-8);
	pmodule=PyImport_ExecCodeModule("tpt_console", tpt_console_obj);
#endif
	
	if (pmodule!=NULL)
	{
		pfunc=PyObject_GetAttrString(pmodule,"handle");//get the handler function
		if (pfunc && PyCallable_Check(pfunc))//check if it's really a function
		{
			printf("python console ready to go.\n");
		}
		else
		{
			PyErr_Print();
			printf("unable to find handle function, mangled console.py?\n");
			pyready = 0;
			pygood = 0;
		}

		pstep=PyObject_GetAttrString(pmodule,"step");//get the handler function
		if (pstep && PyCallable_Check(pstep))//check if it's really a function
		{
			printf("step function found.\n");
		}
		else
		{
			printf("unable to find step function. ignoring.\n");
		}

		pkey=PyObject_GetAttrString(pmodule,"keypress");//get the handler function
		if (pstep && PyCallable_Check(pkey))//check if it's really a function
		{
			printf("key function found.\n");
		}
		else
		{
			printf("unable to find key function. ignoring.\n");
		}
	}
	else
	{
		//sys.stderr
		PyErr_Print();
		printf("unable to find console module, missing file or mangled console.py?\n");
		pyready = 0;
		pygood = 0;
	}
}
void pycon_step(){
	if (pyready==1 && pygood==1)
		if (pstep!=NULL)
		{
			pargs=Py_BuildValue("()");
			pvalue = PyObject_CallObject(pstep, pargs);
			Py_DECREF(pargs);
			pargs=NULL;
			if (pvalue==NULL)
				strcpy(console_error,"failed to execute step code.");
			//Py_DECREF(pvalue);
			//puts("a");
			pvalue=NULL;
		}
}
void pycon_close(){

	PyRun_SimpleString("import os,tempfile,os.path\ntry:\n    os.remove(os.path.join(tempfile.gettempdir(),'tpt_console.py'))\nexcept:\n    pass");
	PyRun_SimpleString("import os,tempfile,os.path\ntry:\n    os.remove(os.path.join(tempfile.gettempdir(),'tpt_console.pyo'))\nexcept:\n    pass");
	PyRun_SimpleString("import os,tempfile,os.path\ntry:\n    os.remove(os.path.join(tempfile.gettempdir(),'tpt_console.pyc'))\nexcept:\n    pass");

	Py_Finalize();//cleanup any python stuff.
}
int process_command_py(pixel *vid_buf, char *console, char *console_error)
{
	int y,x,nx,ny,i,j,k,m;
	int do_next = 1;
	char xcoord[10];
	char ycoord[10];
	char console2[15];
	char console3[15];
	char console4[15];
	char console5[15];
	//PyObject *pvalue,*pargs;
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
void pycon_keypress(char key, int modifier){
	if (pyready==1 && pygood==1)
		if (pkey!=NULL && sdl_key!=NULL)
		{
			pargs=Py_BuildValue("(c)",sdl_key);
			pvalue = PyObject_CallObject(pkey, pargs);
			Py_DECREF(pargs);
			pargs=NULL;
			if (pvalue==NULL)
				strcpy(console_error,"failed to execute key code.");
			pvalue=NULL;
		}
}

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
	delete_part(x,y, 0);
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
