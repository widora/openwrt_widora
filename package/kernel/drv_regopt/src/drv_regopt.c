#include <linux/module.h>  
#include <linux/types.h>  
#include <linux/fs.h>  
#include <linux/errno.h>  
#include <linux/mm.h>  
#include <linux/sched.h>  
#include <linux/init.h>  
#include <linux/cdev.h>  
#include <asm/io.h>  
#include <asm/uaccess.h>  
#include <linux/timer.h>  
#include <asm/atomic.h>  
#include <linux/slab.h>  
#include <linux/device.h>  

#define DEV_NAME    "regopt"

static struct class             *reg_opt_class;
static struct class_device	*reg_opt_class_dev;

struct s_reg_param{
	volatile unsigned long reg_add;	//寄存器物理地址
	volatile unsigned long reg_val;	//寄存器值
	volatile unsigned long *vm_add;	//寄存器映射地址
};

/*user 和kenel交互数据的结构体，
 * reg_add 寄存器地址
 * reg_val 寄存器的值
 * vm_add  映射到虚拟地址
 */
static struct s_reg_param reg_param = {
	.reg_add = 0,
	.reg_val = 0,
	.vm_add  = NULL,
};

static int reg_opt_open(struct inode *inode, struct file *file)
{
	//printk("reg_opt_open\n");
	return 0;
}

static int reg_opt_close(struct inode *inode, struct file *file)
{
	//printk("reg_opt_close\n");
	return 0;
}

static int reg_opt_read(struct file *filp, char __user *buff, size_t count, loff_t *offp)
{
	//一定要先设置用户区的buff（即应用层read函数到第二个参数），就是要修改到物理寄存器地址
	copy_from_user(&reg_param, buff, 4);	//物理地址4字节

//	printk("reg_add = %x\n", reg_param.reg_add);
	reg_param.vm_add = (volatile unsigned long *)ioremap(reg_param.reg_add, 0x4);	//映射寄存器
	
//	printk("reg_vmadd = %x\n", reg_param.vm_add);
	reg_param.reg_val = *(reg_param.vm_add);	//取出寄存器
//	printk("reg_val = %x\n", reg_param.reg_val);
	copy_to_user(buff,&reg_param, 8);	//物理地址和值，2个4字节
	iounmap(reg_param.vm_add);	//取消映射
    	return 0;
}

static int reg_opt_write(struct file *filp, char __user *buff, size_t count, loff_t *offp)
{
	//把要修改的参数复制到内核reg_param中
	copy_from_user(&reg_param, buff, 8);	//物理地址和值，2个4字节
//	printk("reg_add = %x\n", reg_param.reg_add);
//	printk("reg_val = %x\n", reg_param.reg_val);
	reg_param.vm_add = (volatile unsigned long *)ioremap(reg_param.reg_add, 0x4);	//映射寄存器
//	printk("reg_vmadd = %x\n", reg_param.vm_add);
	*(reg_param.vm_add) = reg_param.reg_val;
//	printk("reg_val = %x\n", *(reg_param.vm_add));
	iounmap(reg_param.vm_add);	//取消映射
	return 0;
}

static struct file_operations reg_opt_fops = {
	.owner   =  THIS_MODULE,    /* 这是一个宏，推向编译模块时自动创建的__this_module变量 */
	.open    =  reg_opt_open,
	.release =  reg_opt_close,
	.read    =  reg_opt_read,
	.write	  =  reg_opt_write,	   
};

static int major;
static int reg_opt_init(void)
{
	major = register_chrdev(0, DEV_NAME, &reg_opt_fops); // 注册, 告诉内核
	reg_opt_class = class_create(THIS_MODULE, DEV_NAME);
	reg_opt_class_dev = device_create(reg_opt_class, NULL, MKDEV(major, 0), NULL, DEV_NAME); /* /dev/reg_opt */

	printk("reg_opt_init\n");
	return 0;
}

static void reg_opt_exit(void)
{
	device_destroy(reg_opt_class, MKDEV(major, 0));	//删除设备节点
    	class_destroy(reg_opt_class);	//删除类

	unregister_chrdev(major, DEV_NAME); // 注销设备
	printk("reg_opt_exit\n");
}

module_init(reg_opt_init);
module_exit(reg_opt_exit);


MODULE_LICENSE("GPL");

