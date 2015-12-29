/*
*       A simple character driver for learn
*/
#include <linux/module.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/mm.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/cdev.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/slab.h>    /* kmalloc */
#include <linux/device.h>         /* device_create, class_create */
#include <linux/poll.h>

#include "simple_async.h"

struct simple_chrdev *dev;
static unsigned int major_no = 0;
static struct class *simple_class;

//fasync callback
static int simple_async(int fd, struct file *filp, int mode)
{
	struct simple_chrdev *dev = filp->private_data;
	
	return fasync_helper(fd, filp, mode, &dev->async_queue);
}

static int simple_fifo_open(struct inode *inode,struct file *filp)
{
         struct simple_chrdev *dev;
 
         dev= container_of(inode->i_cdev, struct simple_chrdev, cdev);
         filp->private_data= dev;
         return 0;
}

static int simple_release(struct inode *inode, struct file *filp)
{
	//release 
	 simple_async(-1, filp, 0);
         return 0;
}

static ssize_t simple_read(struct file *filp, char __user *buf, size_t count,
                                                                 loff_t *f_pos)
{
         struct simple_chrdev *dev = filp->private_data;
         unsigned long pos = *f_pos;
         int ret = 0;
 
         mutex_lock(&dev->mutex);
         if(pos >= MAX_SIZE) {
                ret= - EFAULT;
                goto out;
         }
 
         if(count > MAX_SIZE - pos)
                   count= MAX_SIZE - pos;
        
         if(copy_to_user(buf, (void *)(dev->mem + pos), count)) {
                   ret= -EFAULT;
                   goto out;
         }
       //  *f_pos+= count;
         ret= count;
 
out:
         mutex_unlock(&dev->mutex);
         return ret;
}

static void do_timer(unsigned long data)
{
	struct simple_chrdev *dev = (struct simple_chrdev *)data;
	printk("name: %s\n", dev->name);

	//send async signal
	if (dev->async_queue)
		kill_fasync(&dev->async_queue, SIGIO, POLL_IN);

	//modfied timer delay time
	mod_timer(&dev->timer, jiffies + 5*HZ);
}

struct file_operations simple_fops = {
         .owner     = THIS_MODULE,
         .read         = simple_read,
         .open        = simple_fifo_open,
         .release   = simple_release,
	 //fasync callback
	 .fasync	= simple_async,
};

static int __init simple_chrdev_init(void)
{
         int minor_no = 0;
         dev_t dev_no; 
         int ret;
 
         if(major_no) {
                   dev_no= MKDEV(major_no, minor_no);
                   ret= register_chrdev_region(dev_no, 1, CHRDEV_NAME);
         }else {
                   ret= alloc_chrdev_region(&dev_no, minor_no, 1, CHRDEV_NAME);
                   major_no= MAJOR(dev_no);
         }
         if(ret) {
                   printk("cannot get major %d\n", major_no);
                   return ret;
         }
         printk("get major %d\n", major_no);
 
         dev= kmalloc(sizeof(struct simple_chrdev), GFP_KERNEL);
         if(dev == NULL) {
                   ret= - ENOMEM;
                   goto err_malloc;
         }
         memset(dev,0, sizeof(struct simple_chrdev));

	 strcpy(dev->name, "simple_chrdev");
	 dev->info.number= 1;
	 strcpy(dev->info.author_name, "paul");

	 strcpy(dev->mem, "hello world!");
	
	 mutex_init(&dev->mutex);	//init mutex
 
         cdev_init(&dev->cdev, &simple_fops);
         dev->cdev.owner= THIS_MODULE;
         dev->cdev.ops= &simple_fops;
         ret= cdev_add(&dev->cdev, dev_no, 1);
         if(ret) {
                   printk("cannotcdev add\n");
                   goto err_cdev_add;
         }

	//init timer and set function
	//also use DEFINE_TIMER(_name, _function, _expires, _data)
	init_timer(&dev->timer);
	dev->timer.function = do_timer;
	dev->timer.data = (unsigned long)dev; //use to pass device's struct
	dev->timer.expires = jiffies + HZ;

	//start timer
	add_timer(&dev->timer);

 
         /*autocreate device inode file*/
         simple_class= class_create(THIS_MODULE, CLASS_NAME);
         if(IS_ERR(simple_class)) {
                   printk("ERR:cannot create a simple_class");
                   goto err_class_crt;
         }
         device_create(simple_class,NULL, MKDEV(major_no, 0), 0, CHRDEV_NAME);
 
         printk("Iam in\n");
         return 0;

err_class_crt:
         cdev_del(&dev->cdev);
err_cdev_add:
         kfree(dev);
err_malloc:
         unregister_chrdev_region(MKDEV(major_no,0), 1);
         return ret;
}

static void __exit simple_chrdev_exit(void)
{
         device_destroy(simple_class,MKDEV(major_no, 0));
         class_destroy(simple_class);
	//delete timer 
	del_timer(&dev->timer);

         cdev_del(&dev->cdev);
	 mutex_destroy(&dev->mutex);
         kfree(dev);
         unregister_chrdev_region(MKDEV(major_no,0), 1);
         printk("Iam exit\n");
}

module_init(simple_chrdev_init);
module_exit(simple_chrdev_exit);
 
MODULE_AUTHOR("CJOK<cjok.liao@gmail.com>");
MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("A simple characterdriver for learn");
