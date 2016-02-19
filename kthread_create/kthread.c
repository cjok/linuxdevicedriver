#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/kthread.h>

static struct task_struct *foo_thread;

static int foo_handler(void *data)
{
	int count = 0;

	while (1) {
		set_current_state(TASK_UNINTERRUPTIBLE);

		//receive stop signal from call kthread_stop()
		if (kthread_should_stop()) 
			break;
		printk("count = %d\n", count++);

		schedule_timeout(1*HZ);
	}
}

static int __init kthread_init(void)
{
	//kthread create
	//struct task_struct *kthread_create(int (*threadfn)(void *data), void *data, const char *namefmt);
	foo_thread = kthread_create(foo_handler, NULL, "foo_thread");	
	if (IS_ERR(foo_thread)) {
		printk("kthread create error : %ld\n", PTR_ERR(foo_thread));
		return -1;
	}

	//kthread run
	wake_up_process(foo_thread);
	

	printk("%s", __func__);	
	return 0;
}

static void __exit kthread_exit(void)
{
	//kthread stop
	kthread_stop(foo_thread);

	printk("%s", __func__);	
}

module_init(kthread_init);
module_exit(kthread_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Paul#cjok.liao@gmail.com");
