#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/workqueue.h>
#include <linux/timer.h>

static struct delayed_work dw;
static int count = 0;

void handler(struct work_struct *work)
{
	printk("handler...%d\n", count++);

	schedule_delayed_work(&dw, 1 * HZ);
}

static int __init test_init(void)
{
	INIT_DELAYED_WORK(&dw, handler);

	schedule_delayed_work(&dw, 1*HZ);

	printk("%s", __func__);	

	return 0;
}

static void __exit test_exit(void)
{
	cancel_delayed_work(&dw);
	printk("%s", __func__);	
}

module_init(test_init);
module_exit(test_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Paul#cjok.liao@gmail.com");
