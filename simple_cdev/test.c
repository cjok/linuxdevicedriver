#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#define PATH	"/dev/simple_chrdev"

int main(void)
{
	int fd;
	int ret = 0;
	char buff[20] = {0};

	fd = open(PATH, O_RDWR);
	if (fd == -1) {
		perror(PATH);
		return -1;
	}

	write(fd, "hello world", 16);
	
	lseek(fd, SEEK_SET, 0);

	read(fd, buff, 20);

	printf("%s\n", buff);

	close(fd);

	return 0;
}
