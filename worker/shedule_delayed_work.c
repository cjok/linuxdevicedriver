#include <linux/workqueue.h>

 struct delayed_work resume_delayed_work;


 static void resume_work(struct work_struct *work)
 {
        int err;
 
       printk("paul ##schedule %s\n", __func__);
 
        err = ltr559_als_enable(MODE_ACTIVE);
     if (err < 0)
      {
          APS_ERR("enable als fail: %d\n", err);        
      }
 }


 INIT_DELAYED_WORK(&resume_delayed_work, resume_work);


 schedule_delayed_work(&resume_delayed_work, msecs_to_jiffies(200));
