#include <linux/init.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/ioctl.h>
#include <linux/uaccess.h>
#include <linux/delay.h>
#include <linux/string.h>
#include <linux/wait.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/pinctrl/consumer.h>
#include <linux/of_gpio.h>
#include <linux/delay.h>
#include <linux/types.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>

#define BUFSIZE 1024
static struct proc_dir_entry *Our_Proc_File;

static char *buf;
static unsigned int len;
int tid;
char tmp[128] = {0};
static int my_info_show(struct seq_file *m, void *v)
{
	seq_printf(m, "%s", tmp);
    return 0;
}

static int my_info_open(struct inode *inode, struct  file *file)
{
	//return single_open(file, NULL, NULL);
	printk("[%s][%d]\n", __func__, __LINE__);
	return 0;    

}

// #define current (current_thread_info()->task)
static ssize_t mydev_read(struct file *filp, char __user *buf, size_t count, loff_t *pos)
{
	struct task_struct *p;
	p = pid_task(find_vpid(tid), PIDTYPE_PID);
	// p = pid_task(tid, PIDTYPE_PID);
	char tmp[] = "Kernel says hello";
	unsigned long context = p->nvcsw + p->nivcsw;
	unsigned long exetime = p->utime;
	exetime = exetime/1000000;
	// sprintf(tmp, "%ld",context);[\t] ThreadID : [TID number] time : [utime](ms) context switch times : [Context switches]
	// sprintf(tmp, "ThreadID : %d time : %ld(ms) context switch times : %ld",tid,exetime,context);
	sprintf(tmp, "%ld %ld",exetime,context);
	// sprintf(tmp, "%ld",tid);
	copy_to_user(buf, tmp, sizeof(tmp));
	*pos = 0;
	return 0;
}


static ssize_t mydev_write(struct file *filp, const char __user *buf, size_t count, loff_t *pos)
{
	
	printk("[%s][%d]\n", __func__, __LINE__);
	copy_from_user(tmp, buf, sizeof(tmp));
	printk("%s", tmp);
	// tid = &tmp;
	// single_open(filp, my_info_show, NULL);
	return count;
}

struct miscdev_data {
    int val;
    char data[64];
};

#define IOCTL_MISCDEV_SET 0x00
#define IOCTL_MISCDEV_GET 0x04
struct miscdev_data data;
static long mydev_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	switch(cmd) {
		case IOCTL_MISCDEV_SET:
				if( copy_from_user(&tid , arg, sizeof(tid)) )
				{
						pr_err("Data Write : Err!\n");
				}
				pr_info("Value = %d\n", tid);
				break;
		case IOCTL_MISCDEV_GET:
				//tid = 1;//copy_from_user(tmp, buf, sizeof(tmp));
				if( copy_to_user(arg, &tid, sizeof(tid)) )
				{
						pr_err("Data Read : Err!\n");
				}
				break;
		default:
				pr_info("Default\n");
				break;
}
return 0;
}

static const struct proc_ops test_fops = {
    // .proc_lseek=seq_lseek,
    // .proc_release=single_release,
    .proc_open = my_info_open,
    //.release = single_release,
    .proc_read = mydev_read,
    //.llseek = seq_lseek,
    .proc_write = mydev_write,
    .proc_ioctl = mydev_ioctl,
};

static int __init test_init(void)
{
    struct proc_dir_entry* file;

    //創建proc文件並關聯file_operations
    file = proc_create("thread_info", 0644, NULL, &test_fops);
    if (!file)
    return -ENOMEM;
    printk("test_rw init success!\n");
    return 0;
}

static void __exit test_exit(void)
{
    remove_proc_entry("thread_info", NULL);
    printk("test_exit\n");

}

module_init(test_init);
module_exit(test_exit);

MODULE_LICENSE("GPL");