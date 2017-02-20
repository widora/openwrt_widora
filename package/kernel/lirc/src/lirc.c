/***************************** 
*
*   驱动程序模板
*   版本：V1
*   使用方法(末行模式下)：
*   :%s/LIRC/"你的驱动名称"/g
*
*******************************/


#include <linux/mm.h>
#include <linux/miscdevice.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/mman.h>
#include <linux/random.h>
#include <linux/init.h>
#include <linux/raw.h>
#include <linux/tty.h>
#include <linux/capability.h>
#include <linux/ptrace.h>
#include <linux/device.h>
#include <linux/highmem.h>
#include <linux/crash_dump.h>
#include <linux/backing-dev.h>
#include <linux/bootmem.h>
#include <linux/splice.h>
#include <linux/pfn.h>
#include <linux/export.h>
#include <linux/io.h>
#include <linux/aio.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <asm/uaccess.h>
#include <linux/ioctl.h>
#include <linux/gpio.h>  //----- gpio_to_irq()
#include <linux/interrupt.h> //---request_irq()
//#include <asm/irq.h> //---disable_irq, enable_irq()
//#include <linux/workqueue.h>

#define  GPIO_PIN_NUM 17
#define  GINT_STATUS  (((*GINT_STAT_0)>>GPIO_PIN_NUM)&0x1)  // --confirm the Interrupt
u32 LIRC_DATA; //--readin data
unsigned int LIRC_INT_NUM=0;  //--- irq interrupt number

volatile unsigned long *GPIO_CTRL_0;   //--- GPIO0 to GPIO31 direction control register  0-input 1-output
volatile unsigned long *GPIO_DATA_0;   //-----GPIO0 to GPIO31 data register 
volatile unsigned long *GPIO_POL_0;   //---GPIO0 to GPIO31 polarity control register
volatile unsigned long *GINT_REDGE_0;  //--GPIO0 to GPIO31 rising edge interrupt enable register
volatile unsigned long *GINT_FEDGE_0;  //--GPIO0 to GPIO31 falling edge interrupt enable register
volatile unsigned long *GINT_STAT_0;  //---GPIO0 to GPIO31 interrupt status register 1-int  0 -no int
volatile unsigned long *GINT_EDGE_0;  //---GPIO0 to GPIO31 interrupt edge status register 1-rising 0-falling
volatile unsigned long *GPIO1_MODE; // GPIO1 purpose selection register,for SPIS or GPIO14-17 mode selection
volatile unsigned long *AGPIO_CFG; // analog GPIO configuartion,GPIO14-17 purpose 


/*
volatile unsigned long *GPIO_CTRL_0;// GPIO0-GPIO31 direction control register
volatile unsigned long *GPIO_DATA_0;// GPIO0-GPIO31 data register
volatile unsigned long *GPIO1_MODE; // GPIO1 purpose selection register
volatile unsigned long *AGPIO_CFG; // analog GPIO configuartion,GPIO14-17 purpose 
*/

/****************  基本定义 **********************/
//内核空间缓冲区定义
#if 0
	#define KB_MAX_SIZE 20
	#define kbuf[KB_MAX_SIZE];
#endif


//加密函数参数内容： _IOW(IOW_CHAR , IOW_NUMn , IOW_TYPE)
//加密函数用于LIRC_ioctl函数中
//使用举例：ioctl(fd , _IOW('L',0x80,long) , 0x1);
//#define NUMn LIRC , if you need!
#define IOW_CHAR 'L'
#define IOW_TYPE  long
#define IOW_NUM1  0x80


//初始化函数必要资源定义
//用于初始化函数当中
//device number;
	dev_t dev_num;
//struct dev
	struct cdev LIRC_cdev;
//auto "mknode /dev/LIRC c dev_num minor_num"
struct class *LIRC_class = NULL;
struct device *LIRC_device = NULL;


/*------------------    map GPIO register      --------------------*/
static void map_gpio_register(void)
{
   GPIO1_MODE=(volatile unsigned long *)ioremap(0x10000060,4); // GPIO1 purpose selection register,for SPIS or GPIO14-17 mode selection
   GPIO_DATA_0=(volatile unsigned long *)ioremap(0x10000620,4);   //--- GPIO0 to GPIO31 data register  
   //GPIO_POL_0=(volatile unsigned long *)ioremap(0x10000610,4); //--GPIO0 to GPIO31 plarity control register    
   //AGPIO_CFG; // analog GPIO configuartion,GPIO14-17 purpose 
   GPIO_CTRL_0=(volatile unsigned long *)ioremap(0x10000600,4);   //--- GPIO0 to GPIO31 direction control register  0-input 1-output
   GINT_REDGE_0=(volatile unsigned long *)ioremap(0x10000650,4);  //--GPIO0 to GPIO31 rising edge interrupt enable register
   GINT_FEDGE_0=(volatile unsigned long *)ioremap(0x10000660,4);  //--GPIO0 to GPIO31 falling edge interrupt enable register
   GINT_STAT_0=(volatile unsigned long *)ioremap(0x10000690,4);  //---GPIO0 to GPIO31 interrupt status register 1-int  0 -no int
   //GINT_EDGE_0;  //---GPIO0 to GPIO31 interrupt edge status register 1-rising 0-falling
}

/*------------------    ummap GPIO register      --------------------*/
static void unmap_gpio_register(void)
{
     iounmap(GPIO1_MODE);
     iounmap(GPIO_DATA_0);
     iounmap(GPIO_CTRL_0);
     iounmap(GINT_REDGE_0);
     iounmap(GINT_FEDGE_0);
     iounmap(GINT_STAT_0);
}

/*------------------      init GPIO purpose       ---------------- */
static void init_gpio(void)
{
   *GPIO1_MODE |=(0x1<<2); //---set SPIS as GPIO14-17	
   *GPIO1_MODE &=~(0x1<<3);	
   *GPIO_CTRL_0 &=~(0x1<<GPIO_PIN_NUM); //---bit set 0, input mode
  // *GPIO_POL_0 |=(0x1<<GPIO_PIN_NUM); ---set polarity 1 as inverted ----crack!!!!!!
}

/* ----------------     set and  enable gpio interrupt  rise_int() and fall_int() to be interlocked  ------------*/
static void enable_gpio_rise_int(void)
{
   
   *GINT_REDGE_0 |=(0x1<<GPIO_PIN_NUM); //--bit set 1,Enable Rising Edge interrupt  
   *GINT_FEDGE_0 &=~(0x1<<GPIO_PIN_NUM); //--bit set 0,disable falling Edge interrupt  
}

static void enable_gpio_fall_int(void)
{
  /*GPIO_DATA_0 &=~(0x1<<GPIO_PIN_NUM); //--PRESET 0 */
   *GINT_FEDGE_0 |=(0x1<<GPIO_PIN_NUM); //--bit set 1,Enable falling Edge interrupt  
   *GINT_REDGE_0 &=~(0x1<<GPIO_PIN_NUM); //--bit set 0,disable Rising Edge interrupt  
}

/*----------------------  disable gpio interrupt ----------------------*/
static void disable_gpio_int(void)
{
   *GINT_REDGE_0 &=~(0x1<<GPIO_PIN_NUM); //--bit set 1,disable Rising Edge interrupt  
   *GINT_FEDGE_0 &=~(0x1<<GPIO_PIN_NUM); //--bit set 1,disable falling Edge interrupt  
}


/*-------------------    get gpio data   --------------------*/
static int get_gpio_data(void)
{
  return ((*GPIO_DATA_0)>>GPIO_PIN_NUM)&0x01; 
}


/*-------------------    confirm interrupt  --------------------*/
/*-----     NOTE: this function call will cost ~3ms, use MACRO if possible  -----*/
static int confirm_interrupt(void)
{
  if(((*GINT_STAT_0)>>GPIO_PIN_NUM)&0x1)  //-----confirm the Interrupt
    {  

       printk("GPIO Interrupt confirmed!\n"); 
       return 1;
    }
  return 0;
}

/*   ------------------   tasklet function    ----------------    */
/*   WARNING!!!!  tasklet is atomic operation, sleep function is forbidden   */
/*
static void enable_irq_tasklet(unsigned long param)
{
   printk("Entering tasklet......\n");
   msleep_interruptible(200); //------------sleep NOT ALLOWED in atomic operation !!!!!
   printk("Starting enable_irq.....\n");
   enable_irq(LIRC_INT_NUM);
}
DECLARE_TASKLET(lirc_tasklet,enable_irq_tasklet,0);
 */


/*   ------------------   schedule work function    ----------------    */
static struct work_struct lirc_wq;
static void enable_irq_wq(struct work_struct *data)
{
   printk("Entering work_queue and start msleep......\n");
   msleep_interruptible(150); //----deter re-enabling irq,avoid key-jitter
   printk("Starting enable_irq.....\n");
   enable_irq(LIRC_INT_NUM); //---re-enable_irq allowed in work-queue, NOT in tasklet!!!
   enable_gpio_fall_int();  //---re-enable gpio interrupt
} 
//---INIT_WORK(&lirc_wq,enable_irq_wq) ---!!!!!WARNING: INIT_WORK must run during Module Init,just afer irq_ register -------

/*------------------   get gpio IRQ number    ---------------- */
static void get_irq_num(void)
{
        LIRC_INT_NUM=gpio_to_irq(GPIO_PIN_NUM);  
        if(LIRC_INT_NUM!=0)
          printk("GPIO%d get IRQ= %d successfully!\n",GPIO_PIN_NUM,LIRC_INT_NUM); 
        else
          printk("Get LIRC_INT_NUM failed!\n");
}


/*-----------------------    interrupt handler     ----------------------*/
//static void LIRC_int_handle(int irq,void *dev_id,struct pt_regs *regs)
static irqreturn_t LIRC_int_handle(int irq,void *dev_id,struct pt_regs *regs)
{
  int k=0,k1=0,k2=0,k3=0,k4=0;
  int j,n;
  //u32 data=0;
  
  if(GINT_STATUS)
  //if(confirm_interrupt()) //---- NOTE: call this operation will cost ~3ms !!!  
  //if(1)
  {
    printk("midas_GPIO Interrupt triggered! ------\n");   
   //-------------- irq handle function -------------------
      disable_irq_nosync(LIRC_INT_NUM);
      //disable_irq(LIRC_INT_NUM); !!!!!!! WARNING: disable_irq() MUST NOT use in irq handler or in IRQF_SHARED mode,This will cause dead-loop !!!!!.
      disable_gpio_int();
       
      while(!get_gpio_data())
       {
        udelay(1);
        k++;
       }
       if(k<1500) //---- if noise signal
        {
           printk("k=%d  ------\n",k);
           enable_gpio_fall_int();  //--- enable gpio interrupt,it will trigger int stat register however disable_irq()?????
           enable_irq(LIRC_INT_NUM); //----will not re-enable gpio int
           return IRQ_HANDLED;
        }

       else
          while(get_gpio_data()) //---let 1 pass
          {
              udelay(1);
              k1++;

              if(k1 > 5000) //---check if wrong signal
              {        
                  printk("-----------Read LIRC data error! Second preamble too long!\n");
                  //---------- Before every RETURN , you have to enable gpio-int 
                  enable_gpio_fall_int();  //--- enable gpio interrupt,it will trigger int stat register however disable_irq()???
                  enable_irq(LIRC_INT_NUM); //----will not re-enable gpio int
                  return IRQ_HANDLED;
               } 
          }

        //---------------------   RECEVING 32bits DATA -----------------
        
        for(j=0;j<32;j++)
         {
            n=0;
            while(!get_gpio_data())
            {
                udelay(1);
                k2++;
/*
                if(k2 > 1000) //---check if wrong signal
                {        
                  printk("--------------Read LIRC data error!\n");
                  break;
                }
*/
             }
                if(k2 < 100) //---check if wrong signal
                {        
                  printk("--------------Read LIRC data error! first bit signal too shor!\n");
                  break;
                }

            while(get_gpio_data())
               {
                   udelay(1); 
                   n++;

                   if(n > 2000) //---check if wrong signal
                   {        
                      printk("--------------Read LIRC data error! second bit signal too long!\n");
                      //---------- Before every RETURN , you have to re-enable gpio-int 
                      enable_gpio_fall_int();  //--- enable gpio interrupt,it will trigger int stat register however disable_irq()???
                      enable_irq(LIRC_INT_NUM); //----will not re-enable gpio int
                      return IRQ_HANDLED;
                    }
               }   
            if(n>200 && n<800)           
                //data=(data<<1);
                //data=(data>>1);
                  LIRC_DATA=(LIRC_DATA>>1);
            else if(n>=800)            
                //data=(data<<1)|0x1;
                //data=(data>>1)|0x80000000;
                  LIRC_DATA=(LIRC_DATA>>1)|0x80000000;
            else
               {
                 printk("--------------Read LIRC data error!\n");
                 break;
               }
          }            
         printk("-------Preamble:K=%dus K1=%dus  LIRC DATA:0x%08x ----------\n",k,k1,LIRC_DATA);

         //--------------  LOW PART IRQ HANDLER -------------------------------------
         /*  !!!!!!   LOW PART MUST NOT PUT IN UN-ATOMIC CODE STRUCTURE  !!!!!!!!   */              
         //tasklet_schedule(&lirc_tasklet); //------  atomic -----TASKLET SCHEDULE-----------
          schedule_work(&lirc_wq); //--msleep-----  interruptible ---SCHEDULE WORK-QUEUE -------

         //enable_gpio_fall_int();  //---NOTE!!! enable gpio interrupt,it will trigger int stat register nomatter disable_irq()
         //enable_irq(LIRC_INT_NUM); //----will not re-enable gpio int

     printk("LIRC IRQ_handler finish! --------\n"); 
     return IRQ_HANDLED;
  }

  return IRQ_NONE;
}


/*------------------   register IRQ    ---------------- */
static int lirc_register_irq(void)
{
     int intc_result;

     intc_result=request_irq(LIRC_INT_NUM,LIRC_int_handle,IRQF_DISABLED,"LIRC_midas",NULL);
     //intc_result=request_irq(LIRC_INT_NUM,LIRC_int_handle,IRQF_SHARED,"LIRC_midas",(void *)&LIRC_cdev);
 /*--------------------------   IRQF_DISABLED or IRQF_SHARED   -------------------------------------
  WARNING!!!!!!!   dev_id in request_irq() must be the same as in free_irq(), otherwise it will crash the system  
  WARNING!!!!!!!   disable_irq() must not be used in SHARED IRQ module or in interrupter handler
  IRQF_TRIGGER_FALLING also OK
  ------------------------------------------------------------------------------------------------*/
     if(intc_result!=0)
      {
          printk("GPIO Interrupt request_irq fail!\n"); 
          return 1;
       }
      else
       {
          printk("GPIO Interrupt request_irq success!\n"); 
          return 0;     
       }
}



/****************--------    结构体 file_operations 成员函数    ---------*****************/
//open
static int LIRC_open(struct inode *inode, struct file *file)
{
   int ret_v=0;
   printk("LIRC drive open..., \n");

  /*--------------- Init LIRC IRQ ----------------------------------*/
         //-----------    Map GPIO register
         map_gpio_register();
         //---------    init gpio and enable interrupt   
         init_gpio(); 
         enable_gpio_fall_int();
         //----------   get gpio INT number  
         get_irq_num();
         //----------   register IRQ  
         ret_v=lirc_register_irq();
         if(ret_v)
            printk("------ LIRC: register IRQ faile! ------\n");
         //----------   init work-queue  for irq handler low part
         INIT_WORK(&lirc_wq,enable_irq_wq);        
               
	return 0;
}

//close
static int LIRC_close(struct inode *inode , struct file *file)
{
	printk("LIRC drive close...\n");

  /*--------------- Release mmap and IRQ resource  -----------------------------*/
         //-------------- free irq -------
         //free_irq(LIRC_INT_NUM,(void *)&LIRC_cdev); //------ for IRQF_SHARED ---
         free_irq(LIRC_INT_NUM,NULL); //---for IRQF_DISABLED-- WARNING!!! if dev_ID in request_irq() is not NULL, this will crash the system!!!!
         printk("LIRC: free IRQ resuorce...\n"); 
	return 0;
}

//read
static ssize_t LIRC_read(struct file *file, char __user *buffer,
			size_t len, loff_t *pos)
{
	int ret_v = 0;
	printk("LIRC driver read...\n");

        copy_to_user(buffer,&LIRC_DATA,4);
        LIRC_DATA=0; //--clear data after read
        ret_v=4;

	return ret_v;
}

//write
static ssize_t LIRC_write( struct file *file , const char __user *buffer,
			   size_t len , loff_t *offset )
{
	int ret_v = 0;
	printk("LIRC drive write...\n");


	return ret_v;
}

//unlocked_ioctl
static int LIRC_ioctl (struct file *filp , unsigned int cmd , unsigned long arg)
{
	int ret_v = 0;
	printk("LIRC drive ioctl...\n");

	switch(cmd)
	{
		//常规：
		//cmd值自行进行修改

                
	   	case 1:
		break;


 		//带密码保护：
		//请在"基本定义"进行必要的定义
		case _IOW(IOW_CHAR,IOW_NUM1,IOW_TYPE):
		{
			if(arg == 0x1) //第二条件
			{
				
			}

		}
		break;

		default:
			break;
	}

	return ret_v;
}


/***************** 结构体： file_operations ************************/
//struct
static const struct file_operations LIRC_fops = {
	.owner   = THIS_MODULE,
	.open	 = LIRC_open,
	.release = LIRC_close,	
	.read	 = LIRC_read,
	.write   = LIRC_write,
	.unlocked_ioctl	= LIRC_ioctl,
};


/*************  functions: init , exit*******************/
//条件值变量，用于指示资源是否正常使用
unsigned char init_flag = 0;
unsigned char add_code_flag = 0;

//init
static __init int LIRC_init(void)
{
	int ret_v = 0;

	printk("-------------  LIRC driver init  ----------\n");

	//函数alloc_chrdev_region主要参数说明：
	//参数2： 次设备号
	//参数3： 创建多少个设备
	if( ( ret_v = alloc_chrdev_region(&dev_num,0,1,"LIRC_proc") ) < 0 )
	{
		goto dev_reg_error;
	}
	init_flag = 1; //标示设备创建成功；

	printk("The drive info of LIRC:\nmajor: %d\nminor: %d\n",
		MAJOR(dev_num),MINOR(dev_num));

	cdev_init(&LIRC_cdev,&LIRC_fops);
	if( (ret_v = cdev_add(&LIRC_cdev,dev_num,1)) != 0 )
	{
		goto cdev_add_error;
	}

	LIRC_class = class_create(THIS_MODULE,"LIRC_class");
	if( IS_ERR(LIRC_class) )
	{
		goto class_c_error;
	}

	LIRC_device = device_create(LIRC_class,NULL,dev_num,NULL,"LIRC_dev");
	if( IS_ERR(LIRC_device) )
	{
		goto device_c_error;
	}
	printk("auto mknod success!\n");

	//------------   请在此添加您的初始化程序  --------------------------//

/*   put following codes in modle OPEN operation
         //-----------    Map GPIO register ---------   
         map_gpio_register();
         //---------    init gpio and enable interrupt   -----
         init_gpio(); 
         enable_gpio_fall_int();
         //----------   get gpio INT number   --------
         get_irq_num();
         //----------   register IRQ  ----------------
         ret_v=lirc_register_irq();
         //----------   init work-queue  for irq handler low part-----
         INIT_WORK(&lirc_wq,enable_irq_wq);
*/ 
        //如果需要做错误处理，请：goto LIRC_error;	

	 add_code_flag = 1;
	//----------------------  END  ---------------------------// 

	goto init_success;

dev_reg_error:
	printk("alloc_chrdev_region failed\n");	
	return ret_v;

cdev_add_error:
	printk("cdev_add failed\n");
 	unregister_chrdev_region(dev_num, 1);
	init_flag = 0;
	return ret_v;

class_c_error:
	printk("class_create failed\n");
	cdev_del(&LIRC_cdev);
 	unregister_chrdev_region(dev_num, 1);
	init_flag = 0;
	return PTR_ERR(LIRC_class);

device_c_error:
	printk("device_create failed\n");
	cdev_del(&LIRC_cdev);
 	unregister_chrdev_region(dev_num, 1);
	class_destroy(LIRC_class);
	init_flag = 0;
	return PTR_ERR(LIRC_device);

//------------------ 请在此添加您的错误处理内容 ----------------//
LIRC_error:
         		



	add_code_flag = 0;
	return -1;
//--------------------          END         -------------------//
    
init_success:
	printk("LIRC init success!\n");
	return 0;
}

//exit
static __exit void LIRC_exit(void)
{
	printk("LIRC drive exit...\n");	

	if(add_code_flag == 1)
 	{   
           //----------   请在这里释放您的程序占有的资源   ---------//
	    printk("free your resources...\n");	               

            //--------------  unmap gpio register ----
            unmap_gpio_register();
             printk("LIRC: gpio register mmap release....\n");
/*   put following codes in module CLOSE operation             
            //-------------- free irq -------
             //free_irq(LIRC_INT_NUM,(void *)&LIRC_cdev); //------ for IRQF_SHARED ---
             free_irq(LIRC_INT_NUM,NULL); //---for IRQF_DISABLED-- WARNING!!! if dev_ID in request_irq() is not NULL, this will crash the system!!!!
*/             
	    printk("free finish\n");		               
	    //----------------------     END      -------------------//
	}					            

	if(init_flag == 1)
	{
		//释放初始化使用到的资源;
		cdev_del(&LIRC_cdev);
 		unregister_chrdev_region(dev_num, 1);
		device_unregister(LIRC_device);
		class_destroy(LIRC_class);
	}
}


/**************** module operations**********************/
//module loading
module_init(LIRC_init);
module_exit(LIRC_exit);

//some infomation
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("from Midas");
MODULE_DESCRIPTION("LIRC driverqw");


/*********************  The End ***************************/
