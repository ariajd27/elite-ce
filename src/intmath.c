unsigned int intsqrt(unsigned int const a)
{
	unsigned int x = a;
	unsigned int c = 0;

	unsigned int d = 1 << 22;
	while (d > a) d >>= 2;

	while (d != 0)
	{
		if (x >= c + d)
		{
			x -= c + d;
			c = (c >> 1) + d;
		}
		else c >>= 1;

		d >>= 2;
	}

	return c;
}

unsigned int intpow(unsigned int a, unsigned int b)
{
	if (b == 0) return 1;
	else return a * intpow(a, b - 1);
}
