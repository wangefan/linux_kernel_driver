#include <linux/init.h>
#include <linux/module.h>
#include <linux/kthread.h> 
#include <linux/delay.h>

/* For worker thread */
static struct task_struct *thread;
int thread_function(void *pv) {
  char *str = (char *)pv;
  printk(KERN_INFO "Thread Function, get param : %s\n", str);
  int i = 0;
  while (!kthread_should_stop()) {
    pr_info("In Thread Function, counting: %d\n", i++);
    msleep(1000);
  }
  return 0;
}

/* Entry */
static int __init kthread_module_init(void) {
  printk(KERN_INFO "kthread_module_init\n");
  char *str_param = "This is a string parameter";
  thread = kthread_create(thread_function, str_param, "thread name");
  if (thread) {
    wake_up_process(thread);
  } else {
    pr_err("Cannot create kthread\n");
    return -1;
  }
#if 0
        /* You can use this method also to create and run the thread */
        etx_thread = kthread_run(thread_function,NULL,"eTx Thread");
        if(etx_thread) {
            pr_info("Kthread Created Successfully...\n");
        } else {
            pr_err("Cannot create kthread\n");
             goto r_device;
        }
#endif

  return 0;
}

/* Exit */
static void __exit kthread_module_exit(void) {
  kthread_stop(thread);
  printk(KERN_INFO "kthread_module_exit ok\n");
}

module_init(kthread_module_init);
module_exit(kthread_module_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Yifan");