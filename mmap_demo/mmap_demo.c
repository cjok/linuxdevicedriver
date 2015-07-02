/*
* for learn mmap
  mmap: 作用是将用户空间的一段虚拟地址映射到设备的IO空间(物理地址)，
  这样一来，用户空间进程将可以直接访问设备的内存，比如Framebuffer。
  嵌入式设备中没有显存，所以在主内存中分配一块内存作为显存，通过mmap
  映射到用户空间，用户空间可以直接访问FB。
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

	//在高端物理内存区分配一个页面
	pg = alloc_pages(GFP_HIGHUSER, 0);
	//SetPageReserved(pg);
	
	//因为物理页面来自高端内存，所以在使用前需要调用kmap为该物理
	//页面简历映射关系
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

	//创建定时器每隔10秒打印一次被映射的物理页面中的内容
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
