#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/io.h>
/*
	LED1 （D7）----- GPIOE13
	LED2 （D8）----- GPIOC17
	LED3 (D9)  ----- GPIOC8
	LED4 (D10) ----- GPIOC7
*/

//1.定义一个cdev
static struct cdev led_dev;

//2.设备号的定义和申请
static unsigned int led_major = 0;//主设备号
static unsigned int led_minor = 0;//主次设备号
static dev_t led_dev_num;//设备号

static struct class *led_class;
static struct device *gec6818_led_dev;

static struct resource * led_res;

static volatile unsigned int __iomem *GPIOE_BASE = NULL;
static volatile unsigned int __iomem *GPIOEALTFN0 = NULL;
static volatile unsigned int __iomem *GPIOEOUTENB = NULL;
static volatile unsigned int __iomem *GPIOEOUT = NULL;

static volatile unsigned int __iomem *GPIOC_BASE = NULL;
static volatile unsigned int __iomem *GPIOCALTFN0 = NULL;
static volatile unsigned int __iomem *GPIOCALTFN1 = NULL;
static volatile unsigned int __iomem *GPIOCOUTENB = NULL;
static volatile unsigned int __iomem *GPIOCOUT = NULL;



//3.定义并初始化文件操作集
//给应用程序的open()提供的接口
static int gec6818_led_open(struct inode *inode, struct file *filp)
{
	printk("led driver is openning\n");
	return 0;
}

//给应用程序的write()提供的接口，我们在这个函数中，要接收应用程序下来的数据。
//应用程序写下来的数据有2个字节：buf[1]--哪一盏的灯（1、2、3、4）；buf[0]--灯的状态，1--on，0--off
static ssize_t gec6818_led_write (struct file *filp, const char __user *buf , size_t len , loff_t * off)
{
	int ret;
	char led_buf[2];
	//接收应用程序写下来的数据，并用这些数据控制硬件。
	if(len != 2)
		return -EINVAL;
	ret = copy_from_user(led_buf, buf, len);//从应用程序拷贝数据到驱动程序。
	if(ret != 0)
		return -EFAULT;
	switch(led_buf[1]){
	case 1:	//led1---GPIOE13(D7)
		if(led_buf[0] == 1)//led1 on
			*GPIOEOUT &= ~(1<<13);
		else if(led_buf[0] == 0)//led1 off
			*GPIOEOUT |= (1<<13);
		else
			return -EINVAL;
		break;
		
	case 2:	//led2---GPIOC17(D8)
		if(led_buf[0] == 1)//led2 on
			*GPIOCOUT &= ~(1<<17);
		else if(led_buf[0] == 0)//led2 off
			*GPIOCOUT |= (1<<17);
		else
			return -EINVAL;	
		break;
		
	case 3:	//led3----GPIOC8(D9)
		if(led_buf[0] == 1)//led3 on
			*GPIOCOUT &= ~(1<<8);
		else if(led_buf[0] == 0)//led3 off
			*GPIOCOUT|= (1<<8);
		else
			return -EINVAL;	
		break;

	case 4:	//led4----GPIOC7(D10)
		if(led_buf[0] == 1)//led4 on
			*GPIOCOUT &= ~(1<<7);
		else if(led_buf[0] == 0)//led4 off
			*GPIOCOUT |= (1<<7);
		else
			return -EINVAL;	
		break;
	default:
		return -EINVAL;
		
	}
	return len;
}
//给应用程序的close函数提供的接口。
static int gec6818_led_release(struct inode *inode, struct file *filp)
{
	printk("led driver closed\n");	
	return 0;	
}

static const struct file_operations gec6818_led_fops = {
	.owner = THIS_MODULE,
	.open   = gec6818_led_open,
	.write = gec6818_led_write,
	.release = gec6818_led_release,
};


//模块的入口和出口
static int __init gec6818_led_init(void)  //入口---驱动的安装函数
{
	int ret;
	//2.申请设备号
	if(led_major != 0){
		led_dev_num = MKDEV(led_major,led_minor);
		ret = register_chrdev_region(led_dev_num, 1, "led_device");//静态注册
	}else{
		ret = alloc_chrdev_region(&led_dev_num, led_minor, 1,
				"led_device");//动态分配
	}
	if(ret != 0){
		printk("can not get a device number\n");
		return ret;
	}
	
	//4.cdev的初始化
	cdev_init(&led_dev, &gec6818_led_fops);
	
	//5.将cdev加入内核
	ret = cdev_add(&led_dev, led_dev_num, 1);
	if(ret < 0){
		printk("cdev add error\n");
		goto cdev_add_error;
	}
	
	//6.创建class
	led_class = class_create(THIS_MODULE, "leds_class");
	if (IS_ERR(led_class)) {
		ret = PTR_ERR(led_class);
		printk("class create error\n");
		goto class_create_error;
	}
	
	//7.创建device
	gec6818_led_dev = device_create(led_class, NULL, 
						led_dev_num, NULL, "led_drv");
	if (IS_ERR(gec6818_led_dev)) {
		ret = PTR_ERR(gec6818_led_dev);
		printk("device create error\n");
		goto device_create_error;
	}
	
	//8.申请物理内存区作为一个资源(E组)
	led_res = request_mem_region(0xC001E000, 0x1000, "GPIOE");
	if(led_res == NULL){
		printk("request GPIOE mem failed\n");
		ret = -EBUSY;
		goto request_gpioe_mem_err;
	}
	
	//9.得到物理地址对应的虚拟地址(E组)
	GPIOE_BASE = ioremap(0xC001E000, 0x1000);
	if(GPIOE_BASE == NULL){
		printk("ioremap gpioe failed\n");
		ret = -EFAULT;
		goto ioremap_gpioe_err;
		
	}
	GPIOEALTFN0 = GPIOE_BASE + 8; //0x20（指针+1，地址值+4）
	GPIOEOUTENB = GPIOE_BASE + 1;
	GPIOEOUT = GPIOE_BASE + 0;
	
	
	//8.申请物理内存区作为一个资源(C组)
	led_res = request_mem_region(0xC001C000, 0x1000, "GPIOE");
	if(led_res == NULL){
		printk("request GPIOC mem failed\n");
		ret = -EBUSY;
		goto request_gpioc_mem_err;
	}
	
	//9.得到物理地址对应的虚拟地址(C组)
	GPIOC_BASE = ioremap(0xC001C000, 0x1000);
	if(GPIOC_BASE == NULL){
		printk("ioremap gpioc failed\n");
		ret = -EFAULT;
		goto ioremap_gpioc_err;
		
	}
	GPIOCALTFN0 = GPIOC_BASE + 8; //0x20（指针+1，地址值+4）
	GPIOCALTFN1 = GPIOC_BASE + 9;
	GPIOCOUTENB = GPIOC_BASE + 1;
	GPIOCOUT = GPIOC_BASE + 0;
	
	//10.通过虚拟地址访问寄存器，设置LED1-4都是off。
	//GPIOE13 设置为OUTPUT 高电平
	*GPIOEALTFN0 &= ~(3<<26); //function0
	*GPIOEOUTENB |= (1<<13);//output
	*GPIOEOUT |= (1<<13);
	
	//GPIOC7/8/17设置为OUTPUT 高电平
	*GPIOCALTFN1 &= ~(3<<2);
	*GPIOCALTFN1 |=  (1<<2);  //GPIOC17--function1
	*GPIOCALTFN0 &= ~(3<<14);
	*GPIOCALTFN0 |=  (1<<14);//GPIOC7---function1	
	*GPIOCALTFN0 &= ~(3<<16);
	*GPIOCALTFN0 |=  (1<<16);//GPIOC8---function1	
	*GPIOCOUTENB |= ((1<<17) + (1<<8) + (1<<7));//output
	*GPIOCOUT |= ((1<<17) + (1<<8) + (1<<7));//output 1
	
	printk("GEC6818 led_drv init ......\n");

	return 0;
	
ioremap_gpioc_err:	
	release_mem_region(0xC001C000, 0x1000);	
request_gpioc_mem_err:
	iounmap(GPIOE_BASE);	
ioremap_gpioe_err:
	release_mem_region(0xC001E000, 0x1000);	
request_gpioe_mem_err:
	device_destroy(led_class, led_dev_num);
device_create_error:
	class_destroy(led_class);	
class_create_error:
	cdev_del(&led_dev);
cdev_add_error:
	unregister_chrdev_region(led_dev_num, 1);
	return ret;	
}

static void __exit gec6818_led_exit(void) //出口---驱动的卸载函数
{
	iounmap(GPIOC_BASE);
	iounmap(GPIOE_BASE);
	release_mem_region(0xC001C000, 0x1000);
	release_mem_region(0xC001E000, 0x1000);
	device_destroy(led_class, led_dev_num);
	class_destroy(led_class);
	//删除cdev
	cdev_del(&led_dev);
	//注销设备号
	unregister_chrdev_region(led_dev_num, 1);	
	printk("GEC6818 led_dev exit .....\n");
}

module_init(gec6818_led_init);//入口--->#insmod *.ko-->moudle_init()-->mod_init()
module_exit(gec6818_led_exit);//出口--->#rmmod *.ko -->module_exit()-->mod_exit()

//模块的描述，是可有可无的。
MODULE_LICENSE("GPL"); //符合的协议
MODULE_AUTHOR("bobeyfeng@163.com");
MODULE_VERSION("V1.0");
MODULE_DESCRIPTION("LED device driver for GEC6818");
