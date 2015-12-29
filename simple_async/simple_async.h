#ifndef __SIMPLE_FIFO_H_
#define __SIMPLE_FIFO_H_

#define MAX_SIZE   1024
#define SIMPLE_MAJOR 256
#define CLASS_NAME "simple_class"
#define CHRDEV_NAME	"simple_async"

struct info {
	int number;
	char author_name[20];
};

struct simple_chrdev {
         struct cdev cdev;
         char mem[MAX_SIZE];
	 int current_len;
	 char name[256];
	 struct info info;
	 int version;
	 struct mutex mutex;
	 struct timer_list timer;
	 struct fasync_struct *async_queue;
};


#define SIMPLE_MAX	16

#endif
