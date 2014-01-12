#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/ioctl.h>
#include <unistd.h>

#define PATH	"/dev/simple_fifo"

//ioctl commands define
#define SIMPLE_MAGIC	's'
#define SIMPLE_GET_NAME	_IOR(SIMPLE_MAGIC, 1, int)
#define SIMPLE_SET_NAME	_IOW(SIMPLE_MAGIC, 2, int)
#define SIMPLE_GET_INFO	_IOR(SIMPLE_MAGIC, 3, struct info)
#define SIMPLE_SET_VERSION_BY_POINTER	_IOW(SIMPLE_MAGIC, 4, int)
#define SIMPLE_SET_VERSION_BY_NUMBER	_IOW(SIMPLE_MAGIC, 5, int)
#define SIMPLE_GET_VERSION	_IOR(SIMPLE_MAGIC, 6, int)

#define SIMPLE_MAX	16

struct info {
	int number;
	char author_name[20];
};



int main(void)
{
	int fd;
	int ret = 0;
	char buff[2048] = {0};
	char name[20] = "NAMEXXX";
	struct info info_data;
	int version;
	int n = 9;

	fd = open(PATH, O_RDWR);
	if (fd == -1) {
		perror(PATH);
		return -1;
	}
	

	memset(buff, 'A', sizeof(buff));
//	write(fd, "hello world", 16);
	write(fd, buff, sizeof(buff));
	printf("fd = %d write: AAAAAAAA\n", fd);

//	read(fd, buff, 20);

//	printf("%s\n", buff);

	close(fd);

	return 0;
}
