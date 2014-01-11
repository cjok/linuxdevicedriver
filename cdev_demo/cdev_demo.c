#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>

static 	dev_t devno;
static 	struct cdev chr_dev;

static int chr_open(struct inode *node, struct file *file)
{
	int major = MAJOR(node->i_rdev);
	int minor = MINOR(node->i_rdev);

	printk("%s: major = %d, minor = %d\n", __func__, major, minor);

	return 0;
}

static ssize_t chr_read(struct file *file, char *buffer, size_t size,
		loff_t *off)
{
	printk("%s\n", __func__);
	return 0;
}

struct file_operations chr_ops = {
	.owner = THIS_MODULE,
	.open = chr_open,
	.read = chr_read,
};

static int __init demo_init(void)
{
	int ret;	

	cdev_init(&chr_dev, &chr_ops);
	
	ret = alloc_chrdev_region(&devno, 0, 1, "chr_dev");
	if (ret){
		printk("alloc device number fail\n");
		return ret;
	}

	printk("%s: major = %d, minor = %d\n", __func__, 
			MAJOR(devno), MINOR(devno));

	ret = cdev_add(&chr_dev, devno, 1);
	if (ret){
		printk("cdev add fail\n");
		return ret;
	}

	return 0;
}

static void __exit demo_exit(void)
{
	printk("%s", __func__);	
	cdev_del(&chr_dev);
	unregister_chrdev_region(devno, 1);
}

module_init(demo_init);
module_exit(demo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Paul#cjok.liao@gmail.com");
