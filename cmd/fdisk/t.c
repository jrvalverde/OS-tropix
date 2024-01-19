#include <sys/syscall.h>

int
main ()
{
	int fd = creat ("tst", 0666);

	lseek (fd, 512 * 1024 * 1024 - 1, 0);

	write (fd, "", 1);

	return (0);
}
