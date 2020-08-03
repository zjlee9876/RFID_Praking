#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/io.h>
/*
	LED1 ��D7��----- GPIOE13
	LED2 ��D8��----- GPIOC17
	LED3 (D9)  ----- GPIOC8
	LED4 (D10) ----- GPIOC7
*/

//1.����һ��cdev
static struct cdev led_dev;

//2.�豸�ŵĶ��������
static unsigned int led_major = 0;//���豸��
static unsigned int led_minor = 0;//�����豸��
static dev_t led_dev_num;//�豸��

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



//3.���岢��ʼ���ļ�������
//��Ӧ�ó����open()�ṩ�Ľӿ�
static int gec6818_led_open(struct inode *inode, struct file *filp)
{
	printk("led driver is openning\n");
	return 0;
}

//��Ӧ�ó����write()�ṩ�Ľӿڣ���������������У�Ҫ����Ӧ�ó������������ݡ�
//Ӧ�ó���д������������2���ֽڣ�buf[1]--��һյ�ĵƣ�1��2��3��4����buf[0]--�Ƶ�״̬��1--on��0--off
static ssize_t gec6818_led_write (struct file *filp, const char __user *buf , size_t len , loff_t * off)
{
	int ret;
	char led_buf[2];
	//����Ӧ�ó���д���������ݣ�������Щ���ݿ���Ӳ����
	if(len != 2)
		return -EINVAL;
	ret = copy_from_user(led_buf, buf, len);//��Ӧ�ó��򿽱����ݵ���������
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
//��Ӧ�ó����close�����ṩ�Ľӿڡ�
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


//ģ�����ںͳ���
static int __init gec6818_led_init(void)  //���---�����İ�װ����
{
	int ret;
	//2.�����豸��
	if(led_major != 0){
		led_dev_num = MKDEV(led_major,led_minor);
		ret = register_chrdev_region(led_dev_num, 1, "led_device");//��̬ע��
	}else{
		ret = alloc_chrdev_region(&led_dev_num, led_minor, 1,
				"led_device");//��̬����
	}
	if(ret != 0){
		printk("can not get a device number\n");
		return ret;
	}
	
	//4.cdev�ĳ�ʼ��
	cdev_init(&led_dev, &gec6818_led_fops);
	
	//5.��cdev�����ں�
	ret = cdev_add(&led_dev, led_dev_num, 1);
	if(ret < 0){
		printk("cdev add error\n");
		goto cdev_add_error;
	}
	
	//6.����class
	led_class = class_create(THIS_MODULE, "leds_class");
	if (IS_ERR(led_class)) {
		ret = PTR_ERR(led_class);
		printk("class create error\n");
		goto class_create_error;
	}
	
	//7.����device
	gec6818_led_dev = device_create(led_class, NULL, 
						led_dev_num, NULL, "led_drv");
	if (IS_ERR(gec6818_led_dev)) {
		ret = PTR_ERR(gec6818_led_dev);
		printk("device create error\n");
		goto device_create_error;
	}
	
	//8.���������ڴ�����Ϊһ����Դ(E��)
	led_res = request_mem_region(0xC001E000, 0x1000, "GPIOE");
	if(led_res == NULL){
		printk("request GPIOE mem failed\n");
		ret = -EBUSY;
		goto request_gpioe_mem_err;
	}
	
	//9.�õ������ַ��Ӧ�������ַ(E��)
	GPIOE_BASE = ioremap(0xC001E000, 0x1000);
	if(GPIOE_BASE == NULL){
		printk("ioremap gpioe failed\n");
		ret = -EFAULT;
		goto ioremap_gpioe_err;
		
	}
	GPIOEALTFN0 = GPIOE_BASE + 8; //0x20��ָ��+1����ֵַ+4��
	GPIOEOUTENB = GPIOE_BASE + 1;
	GPIOEOUT = GPIOE_BASE + 0;
	
	
	//8.���������ڴ�����Ϊһ����Դ(C��)
	led_res = request_mem_region(0xC001C000, 0x1000, "GPIOE");
	if(led_res == NULL){
		printk("request GPIOC mem failed\n");
		ret = -EBUSY;
		goto request_gpioc_mem_err;
	}
	
	//9.�õ������ַ��Ӧ�������ַ(C��)
	GPIOC_BASE = ioremap(0xC001C000, 0x1000);
	if(GPIOC_BASE == NULL){
		printk("ioremap gpioc failed\n");
		ret = -EFAULT;
		goto ioremap_gpioc_err;
		
	}
	GPIOCALTFN0 = GPIOC_BASE + 8; //0x20��ָ��+1����ֵַ+4��
	GPIOCALTFN1 = GPIOC_BASE + 9;
	GPIOCOUTENB = GPIOC_BASE + 1;
	GPIOCOUT = GPIOC_BASE + 0;
	
	//10.ͨ�������ַ���ʼĴ���������LED1-4����off��
	//GPIOE13 ����ΪOUTPUT �ߵ�ƽ
	*GPIOEALTFN0 &= ~(3<<26); //function0
	*GPIOEOUTENB |= (1<<13);//output
	*GPIOEOUT |= (1<<13);
	
	//GPIOC7/8/17����ΪOUTPUT �ߵ�ƽ
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

static void __exit gec6818_led_exit(void) //����---������ж�غ���
{
	iounmap(GPIOC_BASE);
	iounmap(GPIOE_BASE);
	release_mem_region(0xC001C000, 0x1000);
	release_mem_region(0xC001E000, 0x1000);
	device_destroy(led_class, led_dev_num);
	class_destroy(led_class);
	//ɾ��cdev
	cdev_del(&led_dev);
	//ע���豸��
	unregister_chrdev_region(led_dev_num, 1);	
	printk("GEC6818 led_dev exit .....\n");
}

module_init(gec6818_led_init);//���--->#insmod *.ko-->moudle_init()-->mod_init()
module_exit(gec6818_led_exit);//����--->#rmmod *.ko -->module_exit()-->mod_exit()

//ģ����������ǿ��п��޵ġ�
MODULE_LICENSE("GPL"); //���ϵ�Э��
MODULE_AUTHOR("bobeyfeng@163.com");
MODULE_VERSION("V1.0");
MODULE_DESCRIPTION("LED device driver for GEC6818");
