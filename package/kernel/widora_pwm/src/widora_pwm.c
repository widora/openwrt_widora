#include <linux/err.h>
#include <linux/io.h>
#include <linux/ioport.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/pwm.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/aio.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#define DEVICE_NAME "widora_pwm"
#define WIDORA_PWM_MAJOR	0
int major;
static struct class *widora_pwm_class;


struct pwm_device *pwm0 = NULL;
struct pwm_device *pwm1 = NULL;
struct pwm_device *pwm2 = NULL;
struct pwm_device *pwm3 = NULL;
#define PWM_0 0
#define PWM_1 1
#define PWM_2 2
#define PWM_3 3

struct pwm_cfg{
	unsigned int enable;
	unsigned int duty_ns;
	unsigned int period_ns;
};

struct pwm_dev{
	struct cdev cdev;
	struct pwm_cfg cfg[4];
};

struct pwm_dev *my_dev;

static int widora_pwm_open(struct inode *inode,struct file *filp)
{
	filp->private_data = my_dev;
	printk("%s:open\n",__FUNCTION__);
	return 0;
}
static int widora_pwm_close(struct inode *indoe,struct file *filp)
{
	printk("%s:close\n",__FUNCTION__);
	return 0;
}
long widora_pwm_ioctl(struct file *filp,unsigned int cmd,unsigned long arg)
{
	switch(cmd)
	{
	case PWM_0:
		if(((struct pwm_cfg *)arg)->enable)
			pwm_enable(pwm0);
		else
			pwm_disable(pwm0);
	pwm_config(pwm0,((struct pwm_cfg *)arg)->duty_ns,((struct pwm_cfg *)arg)->period_ns);
	break;
	case PWM_1:
		if(((struct pwm_cfg *)arg)->enable)
			pwm_enable(pwm1);
		else
			pwm_disable(pwm1);
	pwm_config(pwm1,((struct pwm_cfg *)arg)->duty_ns,((struct pwm_cfg *)arg)->period_ns);
	break;
	case PWM_2:
		if(((struct pwm_cfg *)arg)->enable)
			pwm_enable(pwm2);
		else
			pwm_disable(pwm2);
	pwm_config(pwm2,((struct pwm_cfg *)arg)->duty_ns,((struct pwm_cfg *)arg)->period_ns);
	break;
	case PWM_3:
		if(((struct pwm_cfg *)arg)->enable)
			pwm_enable(pwm3);
		else
			pwm_disable(pwm3);
	pwm_config(pwm3,((struct pwm_cfg *)arg)->duty_ns,((struct pwm_cfg *)arg)->period_ns);
	break;
	default:
		return -ENOIOCTLCMD;
	}
	return 0;
}

static ssize_t widora_pwm_write(struct file *filp,const char __user *buf,size_t count,loff_t *f_pos)
{
	unsigned long p = *f_pos;
	int ret	= 0;
	struct pwm_dev *devp = filp->private_data;
	if(copy_from_user(((void *)devp),buf,count))
		ret= -EFAULT;
	else
		{


		}
	
	return ret;
}

static struct file_operations widora_pwm_drv_ops = {
.owner	=	THIS_MODULE,
.open	=	widora_pwm_open,
.release=	widora_pwm_close,
.unlocked_ioctl=widora_pwm_ioctl,
.write	=	widora_pwm_write,
};




static int __init widora_pwm_init(void)
{
	major = register_chrdev(WIDORA_PWM_MAJOR, DEVICE_NAME, &widora_pwm_drv_ops);
	if(major < 0)
	{
	printk("widora_pwm register error\n");
	return major;
	}
	my_dev = kmalloc(sizeof(struct pwm_dev),GFP_KERNEL);	
	if(!my_dev)
		{	
		unregister_chrdev(major,"widora_pwm");	
		return -ENOMEM;
		}
	pwm0 = pwm_request(0,"mt7628-pwm");
	pwm1 = pwm_request(1,"mt7628-pwm");
	widora_pwm_class = class_create(THIS_MODULE,"widora_pwm");
	device_create(widora_pwm_class,NULL,MKDEV(major,0),NULL,"widora_pwm");
	printk("%s,hello widora_pwm\n",__FUNCTION__);
	return 0;
}

static void __exit widora_pwm_exit(void)
{
	pwm_disable(pwm0);
	pwm_disable(pwm1);
	pwm_free(pwm0);
	pwm_free(pwm1);
	unregister_chrdev(major,"widora_pwm");
	device_destroy(widora_pwm_class,MKDEV(major,0));
	class_destroy(widora_pwm_class);
	printk("%s,exit widora_pwm\n",__FUNCTION__);
	return 0;

}


module_init(widora_pwm_init);
module_exit(widora_pwm_exit);
MODULE_LICENSE("GPL");
