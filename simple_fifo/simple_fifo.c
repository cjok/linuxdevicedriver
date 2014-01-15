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

#include "simple_fifo.h"

struct simple_chrdev *dev;
static unsigned int major_no = 0;
static struct class *simple_class;

static int simple_fifo_open(struct inode *inode,struct file *filp)
{
         struct simple_chrdev *dev;
 
         dev= container_of(inode->i_cdev, struct simple_chrdev, cdev);
         filp->private_data= dev;
         return 0;
}

static int simple_release(struct inode *inode, struct file *filp)
{
         return 0;
}

static ssize_t simple_read(struct file *filp, char __user *buf, size_t count,
                                                                 loff_t *f_pos)
{
         struct simple_chrdev *dev = filp->private_data;
         int ret = 0;
 
	 mutex_lock(&dev->mutex);

	 while (dev->current_len == 0) {
		mutex_unlock(&dev->mutex);
	  	if (filp->f_flags & O_NONBLOCK)
			return -EAGAIN;
		printk("%s: reading going to sleep\n", current->comm);
		if (wait_event_interruptible(dev->r_wait, dev->current_len != 0)) {
			return -ERESTARTSYS;
		}

		mutex_lock(&dev->mutex);
 	 }
 
         if(count > dev->current_len)
                   count= dev->current_len;
        
         if(copy_to_user(buf, (void *)(dev->mem), count)) {
                   ret= -EFAULT;
                   goto out;
         } else {
		memcpy(dev->mem, dev->mem + count, dev->current_len - count);
	 	dev->current_len -= count;
         	ret = count;
		wake_up_interruptible(&dev->w_wait);
		printk("%s: read %d bytes\n", current->comm, count);
	 }

out:
	 mutex_unlock(&dev->mutex);
         return ret;
}

static ssize_t simple_write(struct file *filp, const char __user *buf,
                                     size_t count, loff_t *f_pos)
{
         struct simple_chrdev *dev = filp->private_data;
         int ret = 0;
 
	 mutex_lock(&dev->mutex);

	 while (dev->current_len >= MAX_SIZE) {
		mutex_unlock(&dev->mutex);
	  	if (filp->f_flags & O_NONBLOCK)
			return -EAGAIN;
		printk("%s: writing going to sleep\n", current->comm);
		if (wait_event_interruptible(dev->w_wait, dev->current_len < MAX_SIZE)) {
			return -ERESTARTSYS;
		}

		mutex_lock(&dev->mutex);
	 }
 
         if(count > MAX_SIZE - dev->current_len)
                   count = MAX_SIZE - dev->current_len;
 
         if(copy_from_user(dev->mem + dev->current_len, buf, count)) {
                   ret= - EFAULT;
                   goto out;
         } else {
	 	dev->current_len += count;
         	ret= count;
		wake_up_interruptible(&dev->r_wait);
		printk("%s: write %d bytes\n", current->comm, count);
	 }
 
out:
	 mutex_unlock(&dev->mutex);
         return ret;
}

long simple_unlocked_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
		int err = 0;
		int ret = 0;
		struct simple_chrdev *dev = filp->private_data;

		//check magic code
		if (_IOC_TYPE(cmd) != SIMPLE_MAGIC)	
				return -ENOTTY;
		if (_IOC_NR(cmd) > SIMPLE_MAX)
				return -ENOTTY;

		//check the user space memory is available,
		//access_ok() return value: non zero is available, zero is unavailable
		//if not need check userspace pointer or add under code, 
		//use __copy_to_user,__copy_from_user, __get_user, __put_user
		if (_IOC_DIR(cmd) & _IOC_READ)
				err = !access_ok(VERIFY_WIRTE, (void __user *)arg, _IOC_SIZE(cmd));
		if (_IOC_DIR(cmd) & _IOC_WRITE)
				err = !access_ok(VERIFY_READ, (void __user *)arg, _IOC_SIZE(cmd));
		if (err) return -EFAULT;
		//////

		switch(cmd) {
			case SIMPLE_GET_NAME:	
				ret = __copy_to_user((void __user *)arg, dev->name, strlen(dev->name));
				printk("GET dev name = %s\n", dev->name);
				break;
			case SIMPLE_SET_NAME:	
				printk("SET dev name = %s\n", (char *)arg);
				ret = __copy_from_user(dev->name, (void __user *)arg, sizeof(dev->name));
				break;
			case SIMPLE_GET_INFO:
				printk("GET INFO\n");
				ret = __copy_to_user((void __user *)arg, &dev->info, sizeof(dev->info));
				break;
			case SIMPLE_SET_VERSION_BY_POINTER:
				printk("SET VERSION BY POINTER = %lu\n", arg);
				ret = __get_user(dev->version, (int __user *)arg);
				break;
			case SIMPLE_SET_VERSION_BY_NUMBER:
				printk("SET VERSION BY NUMBER = %lu\n", arg);
				dev->version = arg;
				break;

			case SIMPLE_GET_VERSION:
				printk("GET VERSION = %d\n", dev->version);
				ret = __put_user(dev->version, (int __user *)arg);
				break;

			default: return -ENOTTY;
		}

		return ret;
}
 
static unsigned int simple_poll(struct file *filp, poll_table *wait)
{
	unsigned int mask = 0;
	struct simple_chrdev *dev = filp->private_data;

	mutex_lock(&dev->mutex);
	/*
	*这里值得注意的是：poll_wait()不会阻塞，因为poll_wait()并没有调用schedule簇函数，
	*只是把当前进程加入到等待队列头上(由__pollwait()来完成)，会继续往下执行。阻塞是
	*由更上层的代码来处理(do_poll()来完成)。
	*
	*当事件可用、被信号唤醒或timeout时，会再次执行poll函数。
	*/
	poll_wait(filp, &dev->r_wait, wait);
	poll_wait(filp, &dev->w_wait, wait);

	if (dev->current_len != 0)
		mask |= POLLIN | POLLRDNORM;
	if (dev->current_len < MAX_SIZE)
		mask |= POLLOUT | POLLWRNORM;

	mutex_unlock(&dev->mutex);
	return mask;	
}
 
struct file_operations simple_fops = {
         .owner     = THIS_MODULE,
         .read         = simple_read,
         .write       = simple_write,
         .open        = simple_fifo_open,
         .release   = simple_release,
	 .unlocked_ioctl = simple_unlocked_ioctl,
	 .poll	= simple_poll,
};

static int __init simple_chrdev_init(void)
{
         int minor_no = 0;
         dev_t dev_no; 
         int ret;
 
         if(major_no) {
                   dev_no= MKDEV(major_no, minor_no);
                   ret= register_chrdev_region(dev_no, 1, CLASS_NAME);
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
	
	 mutex_init(&dev->mutex);	//init mutex
	 init_waitqueue_head(&dev->r_wait);
	 init_waitqueue_head(&dev->w_wait);
 
         cdev_init(&dev->cdev, &simple_fops);
         dev->cdev.owner= THIS_MODULE;
         dev->cdev.ops= &simple_fops;
         ret= cdev_add(&dev->cdev, dev_no, 1);
         if(ret) {
                   printk("cannotcdev add\n");
                   goto err_cdev_add;
         }
 
         /*autocreate device inode file*/
         simple_class= class_create(THIS_MODULE, CHRDEV_NAME);
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
