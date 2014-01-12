#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/ioctl.h>

#define PATH	"/dev/simple_chrdev"

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
	char buff[20] = {0};
	char name[20] = "NAMEXXX";
	struct info info_data;
	int version;
	int n = 9;

	fd = open(PATH, O_RDWR);
	if (fd == -1) {
		perror(PATH);
		return -1;
	}

	write(fd, "hello world", 16);
	
	lseek(fd, SEEK_SET, 0);

	sleep(10);

	read(fd, buff, 20);

	printf("%s\n", buff);

	ioctl(fd, SIMPLE_SET_NAME, (unsigned long *)name);
	ioctl(fd, SIMPLE_GET_NAME, (unsigned long *)buff);
	printf("%s\n", (char *)buff);

	ioctl(fd, SIMPLE_GET_INFO, (unsigned long *)&info_data);
	printf("number = %d, name = %s\n", info_data.number, info_data.author_name);

	ioctl(fd, SIMPLE_SET_VERSION_BY_POINTER, &n);
	ioctl(fd, SIMPLE_GET_VERSION, (unsigned long *)&version);
	printf("version = %d\n", version);

	ioctl(fd, SIMPLE_SET_VERSION_BY_NUMBER, 6);
	ioctl(fd, SIMPLE_GET_VERSION, (unsigned long *)&version);
	printf("version = %d\n", version);

	close(fd);

	return 0;
}
