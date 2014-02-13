#ifdef DEBUG
#define pr_debug(fmt, arg...)		\
			printk(KERN_ERR "[qpnp-bms]%s@%d " fmt, __func__, __LINE__, ##arg)
#else
#define pr_debug(fmt, arg...)	do{}while(0)
#endif
