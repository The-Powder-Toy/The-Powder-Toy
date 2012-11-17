int g;

int trapp(int a, int b);
float compl(float* a, unsigned int n);
void spam();

int main()
{
	int l, r;
	int a, b, m, d, s, as;
	int b1, b2, b3;
	float f, dd;
	int i;
	int k, j, o;
	unsigned int ua, ub, um, ud, us, uas;
	int re;
	float arr[] = { 1.6f, -3.0f, 1.41f, -0.01f };
	float* point;
	float res;

	trap_Print("System calls work\n");

	if (!1)
		return -1;

	trap_Print("Negation works\n");

	if (!(9 == 9 && 6!=4 && 100>10 && 34<85))
		return -1;

	trap_Print("Basic comparisons work\n");
	
	b1 = 9 & 3;
	b2 = 7 | 8;
	b3 = 10 ^ 3;

	if (!(b1 == 1 && b2 == 15 && b3 == 9))
		return -1;

	trap_Print("Boolean functions work\n");

	l = 2;
	l = l<<2;

	r = 9;
	r = r>>2;

	if (!(l == 8 && r == 2))
		return -1;

	trap_Print("Bit shifts work\n");

	a = 6;
	b = 2*2;

	m = a*b;
	d = a/b;
	s = a+b;
	as = a-b;

	if (!(m == 24 && d == 1 && s == 10 && as == 2))
		return -1;

	trap_Print("Arithmetics works\n");

	f = 5.1f;
	dd = 3.1415f;

	if (!((f*dd)>15.0f && (f*dd)<17.0f && (f-dd)<2.0f && (f-dd)>1.0f))
		return -1;

	trap_Print("Floating point arithmetics works\n");

	i = f*dd;

	if (!(i==16))
		return -1;

	trap_Print("Float to Int works\n");

	f = i+1;
	
	if (!(f>16.0f && f<18.0f))
		return -1;

	trap_Print("Int to Float works\n");

	j = 0; o = 0;
	for(k=0; k<5; k++)
		j++;

	for(k=5; k>0; k--)
		o++;

	if (!(o == 5 && j == 5))
		return -1;

	trap_Print("Cycles work\n");

	g = 11;
	g++;

	if (!(g == 12))
		return -1;

	trap_Print("Global variables work\n");

	ua = 6;
	ub = 2*2;

	um = ua*ub;
	ud = ua/ub;
	us = ua+ub;
	uas = ua-ub;

	if (!(um == 24 && ud == 1 && us == 10 && uas == 2))
		return -1;

	trap_Print("Unsigned ints work\n");

	point = &arr[0];
	point += 2;
	if (!(*point == 1.41f))
		return -1;

	trap_Print("Pointer arithmetics works\n");

	re = trapp(12, 4);

	if (!(re == 8))
		return -1;

	trap_Print("Function calls work\n");

	res = compl(arr, 4);

	if (!(res<0.01f && res>(-0.01f)))
		return -1;

	trap_Print("Complex function call works\n");

	for(i=0; i<1000000; i++)
		spam();

	trap_Print("Stack spam check passed\n");

	trap_Print("All checks are passed!\n");

	return 10;
}

int trapp(int a, int b)
{
	int c;
	c = (a+b)/2;
	return c;
}

float compl(float* a, unsigned int n)
{
	int s, i;
	for(i=0; i<n; i++)
		s+=a[i];
	return s;
}

void spam()
{
	return;
}

