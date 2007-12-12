/*
  Copyright (C) 2007 Oleg Fedorov
 */
#define	M	((1U<<31) -1)
#define	A	48271
#define	Q	44488		// M/A
#define	R	3399		// M%A; R < Q !!!
static unsigned int sd = 0;
long int random(void)
{
   unsigned long X;

    X = sd;
    X = A*(X%Q) - R * (unsigned long) (X/Q);
    if (X < 0)
	X += M;

    sd = X;
	return X;
}

void srandom(unsigned int seed) {
	sd = seed;
}
