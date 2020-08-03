#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/io.h>

//1.定义一个cdev
struct cdev beep_dev;

static unsigned int beep_major = 0;//主设备号
static unsigned int beep_minor = 0;//次设备号
static dev_t beep_dev_num;//设备号

static struct class *beep_class;
static struct device *gec6818_beep_dev;


static volatile unsigned int __iomem *GPIOC_BASE= NULL;
static volatile unsigned int __iomem *GPIOCALTFN0 = NULL;
static volatile unsigned int __iomem *GPIOCOUTENB = NULL;
static volatile unsigned int __iomem *GPIOCOUT = NULL;



//3.定义并初始化文件操作集    
//int (*open) (struct inode *, struct file *);
static int gec6818_beep_open(struct inode *inode, struct file *filp)
{
	printk("beep driver is openning\n");
	return 0;
}

//ssize_t (*read) (struct file *, char __user *, size_t, loff_t *)
static ssize_t gec6818_beep_write(struct file *filp, const char __user *buf , size_t len , loff_t * off)
{
	int beep_flag,ret;
	if(len != 1)
		return -EINVAL;
	ret = copy_from_user(&beep_flag, buf, len);//从应用程序拷贝数据到驱动程序。
	if(ret != 0)
		return -EFAULT;
	if(beep_flag == 1) *GPIOCOUT |= (1<<14);
	else if(beep_flag == 0) *GPIOCOUT &= ~(1<<14);
	else return -EINVAL;
	return len;
}

//int (*release) (struct inode *, struct file *)
int gec6818_beep_close(struct inode *inode, struct file *filp)
{
	printk("beep driver closed\n");	
	return 0;	
}


static const struct file_operations gec6818_beep_fops = {
	.owner = THIS_MODULE,
	.write = gec6818_beep_write,
	.open = gec6818_beep_open,
	.release = gec6818_beep_close
};

static int __init gec6818_beep_init(void)
{
	int ret;
	//2.申请设备号
	if(beep_major != 0)
	{
		beep_dev_num = MKDEV(beep_major,beep_minor);
		ret = register_chrdev_region(beep_dev_num,1,"beep_device");
	}
	else
		ret = alloc_chrdev_region(&beep_dev_num,beep_minor,1,
								  "beep_device");
	if(ret != 0)
	{
		printk("can not get a device number\n");
		return ret;
	}
	
	//4.初始化cdev
	cdev_init(&beep_dev,&gec6818_beep_fops);
	
	//5.将cedv加入内核
	ret = cdev_add(&beep_dev, beep_dev_num, 1);
	if(ret < 0)
	{
		printk("cdev add error\n");
		goto cdev_add_error;
	}
	
	//6.创建class
	beep_class = class_create(THIS_MODULE,"beeps_class");
	if(IS_ERR(beep_class))
	{
		ret = PTR_ERR(beep_class);
		printk("class create error\n");
		goto class_create_error;
	}
	
	//7.创建device
	gec6818_beep_dev = device_create(beep_class,NULL,beep_dev_num,
								   NULL,"beep_drv");
	if (IS_ERR(gec6818_beep_dev))
	{
		ret = PTR_ERR(gec6818_beep_dev);
		printk("device create error\n");
		goto device_create_error;
	}
	

	//9.得到物理地址对应的虚拟地址(C组)
	GPIOC_BASE = ioremap(0xC001C000, 0x1000);
	if(GPIOC_BASE == NULL){
		printk("ioremap gpioc failed\n");
		ret = -EFAULT;
		goto ioremap_gpioc_err;
		
	}
	GPIOCALTFN0 = GPIOC_BASE + 8; //0x20（指针+1，地址值+4）
	GPIOCOUTENB = GPIOC_BASE + 1;
	GPIOCOUT = GPIOC_BASE + 0;
	
	
	*GPIOCALTFN0 |= (1<<28);
	*GPIOCALTFN0 &= ~(1<<29);
	
	*GPIOCOUTENB |= (1<<14); 
	*GPIOCOUT &= ~(1<<14);
		
	printk("gec6818 beep_drv init ......\n");
	return 0;
	
	
	
ioremap_gpioc_err:
	device_destroy(beep_class,beep_dev_num);
device_create_error:
	class_destroy(beep_class);
class_create_error:
	cdev_del(&beep_dev);
cdev_add_error:
	unregister_chrdev_region(beep_dev_num, 1);
	return ret;
	
}

static void __exit gec6818_beep_exit(void)
{	
	iounmap(GPIOC_BASE);
    device_destroy(beep_class,beep_dev_num);
	class_destroy(beep_class);
	cdev_del(&beep_dev);
	unregister_chrdev_region(beep_dev_num, 1);
	printk("gec6818 beep_drv exit ......\n");
}

module_init(gec6818_beep_init);
module_exit(gec6818_beep_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("1846136525@qq.com");
MODULE_VERSION("v1.0");
MODULE_DESCRIPTION("Beep device driver for gec6818");