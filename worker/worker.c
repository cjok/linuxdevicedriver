#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/workqueue.h>

static struct work_struct ws;

//work 处理函数
static void handler(struct work_struct *work)
{
	printk("handler...\n");
}

static int __init test_init(void)
{
	//初始化work
	INIT_WORK(&ws, handler);

	//调度执行work
	schedule_work(&ws);

	printk("%s", __func__);	
	return 0;
}

static void __exit test_exit(void)
{
	cancel_work_sync(&ws);
	printk("%s", __func__);	
}

module_init(test_init);
module_exit(test_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Paul#cjok.liao@gmail.com");
