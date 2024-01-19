#include <stdio.h>

main ()
{
	int	n;

	for (n = 1000000; n > 0; n--)
		putchar (getchar ());
}
