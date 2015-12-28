#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/timer.h>

struct foo_dev {
	struct timer_list timer;
	char *name;
};

static struct foo_dev foo;

static void do_timer(unsigned long data)
{
	struct foo_dev *temp = (struct foo_dev *)data;
	printk("name: %s\n", temp->name);

	//modfied timer delay time
	mod_timer(&temp->timer, jiffies + HZ);
}

static int __init timer_init(void)
{
	
	foo.name = "foo timer";

	//init timer and set function
	//also use DEFINE_TIMER(_name, _function, _expires, _data)
	init_timer(&foo.timer);
	foo.timer.function = do_timer;
	foo.timer.data = (unsigned long)&foo; //use to pass device's struct
	foo.timer.expires = jiffies + HZ;

	//start timer
	add_timer(&foo.timer);

	return 0;
}

static void __exit timer_exit(void)
{
	//delete timer 
	del_timer(&foo.timer);
	printk("%s", __func__);	
}

module_init(timer_init);
module_exit(timer_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Paul#cjok.liao@gmail.com");
