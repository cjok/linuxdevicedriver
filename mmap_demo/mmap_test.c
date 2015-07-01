#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#define MAP_SIZE (1024 * 4)
#define USTR_DEF "string changed from the user space"

int main(int argc, char *argv[])
{
	int fd;
	char *pdata;

	if (argc <= 1) {
		printf("Usage: mmap_test devfile pamapped\n");
		return -1;
	}

	fd = open(argv[1], O_RDWR | O_NDELAY);
	if (fd < 0) {
		printf("cannot open\n");
		return -1;
	}

	pdata = (char *)mmap(0, MAP_SIZE, PROT_READ|PROT_WRITE,
		MAP_SHARED, fd, strtoul(argv[2], 0, 16));

	printf("UserAddr = %p, Data from kernel: %s\n", pdata, pdata);
	printf("Writing a string to the kernel space...\n");

	strcpy(pdata, USTR_DEF);
	printf("done\n");

	munmap(pdata, MAP_SIZE);
	close(fd);

	return 0;
}
