#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

static int file_descrip;

static void signal_handler(int signum)
{
	char buf[64] = {0};

	read(file_descrip, buf, sizeof(buf));

	printf("signal num = %d\n", signum);
	printf("buf[%s]\n", buf);
	
}

int main(void)
{
	int fd, oflags;
	
	fd = open("/dev/simple_async", O_RDWR, S_IRUSR | S_IWUSR);
	if (fd != -1) {
		file_descrip = fd;
		signal(SIGIO, signal_handler);
		fcntl(fd, F_SETOWN, getpid());
		oflags = fcntl(fd, F_GETFL);
		fcntl(fd, F_SETFL, oflags | FASYNC);
		while(1) {
			sleep(1000);
		}
	} else {
		printf("device open error\n");
	}

	return 0;
}

