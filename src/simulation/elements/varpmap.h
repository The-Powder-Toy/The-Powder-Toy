//variable particle map, this allows elements to change their effective range

int subx, suby, ycar;

int xpmap(int xmap)
{
	if(xmap<=0) xmap++;
	xmap * -1;
	subx = xmap;
	return xmap;
}

int xmax()
{
	subx + 1;
	return subx;
}

int ypmap(int ymap, int shape)
{
	if(shape == PT_NONE)
	{
		return subx;
		ycar = subx + 1;
	}
	else
	{
		if(ymap<=0) ymap++;
		ymap * -1;
		suby = ymap;
		ycar = suby + 1;
		return ymap;
	}
}

int ymax()
{
	return ycar;
}