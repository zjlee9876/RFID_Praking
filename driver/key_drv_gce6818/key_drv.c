#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/io.h>

//1.定义一个cdev
struct cdev key_dev;

static unsigned int key_major = 0;//主设备号
static unsigned int key_minor = 0;//次设备号
static dev_t key_dev_num;//设备号

static struct class *key_class;
static struct device *gec6818_key_dev;

static struct resource *key_res;

static volatile unsigned int __iomem *GPIOA_BASE= NULL;
static volatile unsigned int __iomem *GPIOAALTFN1 = NULL;
static volatile unsigned int __iomem *GPIOAOUTENB = NULL;
static volatile unsigned int __iomem *GPIOAPAD = NULL;


static volatile unsigned int __iomem *GPIOB_BASE = NULL;
static volatile unsigned int __iomem *GPIOBALTFN0 = NULL;
static volatile unsigned int __iomem *GPIOBALTFN1 = NULL;
static volatile unsigned int __iomem *GPIOBOUTENB = NULL;
static volatile unsigned int __iomem *GPIOBPAD = NULL;

//3.定义并初始化文件操作集    
//int (*open) (struct inode *, struct file *);
static int gec6818_key_open(struct inode *inode, struct file *filp)
{
	printk("key driver is openning\n");
	return 0;
}

//ssize_t (*read) (struct file *, char __user *, size_t, loff_t *)
ssize_t gec6818_key_read(struct file *filp, char __user *buf, size_t len, loff_t *offp)
{
	int ret;
	char key_buf[4];
	if(len != 4)
		return -EINVAL;
	key_buf[0] = (*GPIOAPAD>>28) & 1;
	key_buf[1] = (*GPIOBPAD>>9) & 1;
	key_buf[2] = (*GPIOBPAD>>30) & 1;
	key_buf[3] = (*GPIOBPAD>>31) & 1;

	ret =  copy_to_user(buf, key_buf, len);
	if(ret != 0)
		return -EFAULT;
	return len;
}

//int (*release) (struct inode *, struct file *)
int gec6818_key_close(struct inode *inode, struct file *filp)
{
	printk("key driver closed\n");	
	return 0;	
}


static const struct file_operations gec6818_key_fops = {
	.owner = THIS_MODULE,
	.read = gec6818_key_read,
	.open = gec6818_key_open,
	.release = gec6818_key_close
};

static int __init gec6818_key_init(void)
{
	int ret;
	//2.申请设备号
	if(key_major != 0)
	{
		key_dev_num = MKDEV(key_major,key_minor);
		ret = register_chrdev_region(key_dev_num,1,"key_device");
	}
	else
		ret = alloc_chrdev_region(&key_dev_num,key_minor,1,
								  "key_device");
	if(ret != 0)
	{
		printk("can not get a device number\n");
		return ret;
	}
	
	//4.初始化cdev
	cdev_init(&key_dev,&gec6818_key_fops);
	
	//5.将cedv加入内核
	ret = cdev_add(&key_dev, key_dev_num, 1);
	if(ret < 0)
	{
		printk("cdev add error\n");
		goto cdev_add_error;
	}
	
	//6.创建class
	key_class = class_create(THIS_MODULE,"keys_class");
	if(IS_ERR(key_class))
	{
		ret = PTR_ERR(key_class);
		printk("class create error\n");
		goto class_create_error;
	}
	
	//7.创建device
	gec6818_key_dev = device_create(key_class,NULL,key_dev_num,
								   NULL,"key_drv");
	if (IS_ERR(gec6818_key_dev))
	{
		ret = PTR_ERR(gec6818_key_dev);
		printk("device create error\n");
		goto device_create_error;
	}
	
	//8.申请寄存器的物理地址作为一个资源GPIOA
	key_res = request_mem_region(0xC001A000,0x1000,"GPIOA");
	if(key_res == NULL)
	{
		printk("request GPIOA mem failed\n");
		ret = -EBUSY;
		goto request_gpioa_mem_err;
	}
	
	//9.得到物理地址对应的虚拟地址GPIOA
	GPIOA_BASE = ioremap(0xC001A000,0x1000);
	if(GPIOA_BASE == NULL)
	{
		printk("ioremap GPIOA failed\n");
		ret = -EFAULT;
		goto ioremap_gpioa_err;
	}
	
	//10.通过虚拟地址访问寄存器GPIOA
	GPIOAOUTENB = GPIOA_BASE + 1;
	GPIOAPAD = GPIOA_BASE + 6;
	GPIOAALTFN1 = GPIOA_BASE + 9;
	
	*GPIOAALTFN1 &= ~(3<<24);
	*GPIOAOUTENB &= ~(1<<28);
	
	
	
	//8.申请寄存器的物理地址作为一个资源GPIOB
	key_res = request_mem_region(0xC001B000,0x1000,"GPIOB");
	if(key_res == NULL)
	{
		printk("request GPIOA mem failed\n");
		ret = -EBUSY;
		goto request_gpiob_mem_err;
	}
	
	//9.得到物理地址对应的虚拟地址GPIOB
	GPIOB_BASE = ioremap(0xC001B000,0x1000);
	if(GPIOB_BASE == NULL)
	{
		printk("ioremap GPIOA failed\n");
		ret = -EFAULT;
		goto ioremap_gpiob_err;
	}
	
	GPIOBOUTENB = GPIOB_BASE + 1;
	GPIOBPAD = GPIOB_BASE + 6;
	GPIOBALTFN0 = GPIOB_BASE + 8;
	GPIOBALTFN1 = GPIOB_BASE + 9;
	
	*GPIOBOUTENB &= ~((1<<9)|(1<<30)|(1<<31));
	*GPIOBALTFN0 &= ~(3<<18); 
	*GPIOBALTFN1 |= (1<<28);
	*GPIOBALTFN1 &= ~(1<<29);
	*GPIOBALTFN1 |= (1<<30);
	*GPIOBALTFN1 &= ~(1<<31);
	
	
	printk("gec6818 key_drv init ......\n");
	return 0;
	
ioremap_gpiob_err:
	release_mem_region(0xC001B000,0x1000);
request_gpiob_mem_err:
	iounmap(GPIOA_BASE);
ioremap_gpioa_err:
	release_mem_region(0xC001A000,0x1000);
request_gpioa_mem_err:
	device_destroy(key_class,key_dev_num);
device_create_error:
	class_destroy(key_class);
class_create_error:
	cdev_del(&key_dev);
cdev_add_error:
	unregister_chrdev_region(key_dev_num, 1);
	return ret;
	
}

static void __exit gec6818_key_exit(void)
{	
	iounmap(GPIOB_BASE);
	release_mem_region(0xC001B000,0x1000);
	iounmap(GPIOA_BASE);
	release_mem_region(0xC001A000,0x1000);
    device_destroy(key_class,key_dev_num);
	class_destroy(key_class);
	cdev_del(&key_dev);
	unregister_chrdev_region(key_dev_num, 1);
	printk("gec6818 key_drv exit ......\n");
}

module_init(gec6818_key_init);
module_exit(gec6818_key_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("1846136525@qq.com");
MODULE_VERSION("v1.0");
MODULE_DESCRIPTION("KEY device driver for gec6818");