#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/ioctl.h>

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
	char buff[20] = {0};
	fd_set rfds, wfds;

	fd = open(PATH, O_RDWR);
	if (fd == -1) {
		perror(PATH);
		return -1;
	}

	FD_ZERO(&rfds);
	FD_ZERO(&wfds);
	FD_SET(fd, &rfds);
	FD_SET(fd, &wfds);

	ret = select(fd+1, &rfds, &wfds, NULL, NULL);
	if (ret <= 0) {
		perror("select");
		return 0;
	}

	if (FD_ISSET(fd, &rfds)) {
		read(fd, buff, 20);
		printf("%s\n", buff);
	}

	if (FD_ISSET(fd, &wfds)) {
		memset(buff, 'A', sizeof(buff));
		write(fd, buff, 20);
		printf("WRITE:%s\n", buff);
	}

	close(fd);

	return 0;
}
