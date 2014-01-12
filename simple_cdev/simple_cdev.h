#ifndef __SIMPLE_CDEV_H_
#define __SIMPLE_CDEV_H_

#define MAX_SIZE   1024
#define SIMPLE_MAJOR 256
#define CLASS_NAME "simple_class"
#define CHRDEV_NAME	"simple_chrdev"

struct info {
	int number;
	char author_name[20];
};

struct simple_chrdev {
         struct cdev cdev;
         char mem[MAX_SIZE];
	 char name[256];
	 struct info info;
	 int version;
	 struct mutex mutex;
};


//ioctl commands define
#define SIMPLE_MAGIC	's'
#define SIMPLE_GET_NAME	_IOR(SIMPLE_MAGIC, 1, int)
#define SIMPLE_SET_NAME	_IOW(SIMPLE_MAGIC, 2, int)
#define SIMPLE_GET_INFO	_IOR(SIMPLE_MAGIC, 3, struct info)
#define SIMPLE_SET_VERSION_BY_POINTER	_IOW(SIMPLE_MAGIC, 4, int)
#define SIMPLE_SET_VERSION_BY_NUMBER	_IOW(SIMPLE_MAGIC, 5, int)
#define SIMPLE_GET_VERSION	_IOR(SIMPLE_MAGIC, 6, int)

#define SIMPLE_MAX	16



#endif
