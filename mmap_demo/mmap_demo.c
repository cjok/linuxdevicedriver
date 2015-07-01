/*
* for learn mmap
*/
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/fcntl.h>
#include <linux/gfp.h>
#include <linux/string.h>
#include <linux/mm_types.h>
#include <linux/mm.h>
#include <linux/highmem.h>

#define KSTR_DEF	"hello world form kernel virtual space"

static struct cdev *pcdev;
static dev_t ndev;
static struct page *pg;
static struct timer_list timer;

static void timer_func(unsigned long data)
{
	printk("timer_func: %s\n", (char *)data);
	timer.expires = jiffies + HZ*10;
	add_timer(&timer);
}

static int demo_open(struct inode *inode, struct file *filp)
{
	return 0;
}

static int demo_release(struct inode *inode, struct file *filp)
{
	return 0;
}

static int demo_mmap(struct file *filp, struct vm_area_struct *vma)
{
	int err = 0;
	unsigned long start = vma->vm_start;
	unsigned long size = vma->vm_end - vma->vm_start;
	
	err = remap_pfn_range(vma, start, vma->vm_pgoff, size, vma->vm_page_prot);
	
	return err;
}

static struct file_operations mmap_fops = {
	.owner = THIS_MODULE,
	.open = demo_open,
	.release = demo_release,
	.mmap = demo_mmap,
};

static int __init demo_mmap_init(void)
{
	int err = 0;
	char *kstr;

	pg = alloc_pages(GFP_HIGHUSER, 0);
	//SetPageReserved(pg);
	
	kstr = (char *)kmap(pg);
	strcpy(kstr, KSTR_DEF);
	printk("kpa = 0x%X, kernel string = %s\n", 
		page_to_phys(pg), kstr);

	pcdev = cdev_alloc();
	cdev_init(pcdev, &mmap_fops);
	alloc_chrdev_region(&ndev, 0, 1, "mmap_dev");
	printk("major = %d, minor = %d\n", MAJOR(ndev),
		MINOR(ndev));
	pcdev->owner = THIS_MODULE;
	cdev_add(pcdev, ndev, 1);

	init_timer(&timer);
	timer.function = timer_func;
	timer.data = (unsigned long)kstr;
	timer.expires = jiffies + HZ*10;
	add_timer(&timer);

	return err;
}


static void __exit demo_mmap_exit(void)
{
	del_timer_sync(&timer);
	cdev_del(pcdev);
	unregister_chrdev_region(ndev, 1);
	kunmap(pg);
//	ClearPageRerserved(pg);
	__free_pages(pg, 0);
}
module_init(demo_mmap_init);
module_exit(demo_mmap_exit);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("CJOK <cjok.liao@gmail.com>");
MODULE_DESCRIPTION("mmap_demo");
