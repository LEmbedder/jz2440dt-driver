/**
 * 
*/
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/uaccess.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <linux/of.h>
//#include <asm-generic/io.h>
#include <linux/of_device.h>
#include <linux/of_platform.h>
#include <linux/platform_device.h>
#include <linux/types.h>
#include <linux/cdev.h>

#define S3C2440_GPA(n)  (0<<16 | n)
#define S3C2440_GPB(n)  (1<<16 | n)
#define S3C2440_GPC(n)  (2<<16 | n)
#define S3C2440_GPD(n)  (3<<16 | n)
#define S3C2440_GPE(n)  (4<<16 | n)
#define S3C2440_GPF(n)  (5<<16 | n)
#define S3C2440_GPG(n)  (6<<16 | n)
#define S3C2440_GPH(n)  (7<<16 | n)
#define S3C2440_GPI(n)  (8<<16 | n)
#define S3C2440_GPJ(n)  (9<<16 | n)

struct Led_cdev{
	dev_t dev_node;
	struct cdev cdev;
    int led_pin;
    unsigned int *gpio_con;
    unsigned int *gpio_dat;
};
static struct Led_cdev led_cdev;

#define DEVICE_NAME "jz2440-leds"
/* 123. 分配/设置/注册file_operations 
 * 4. 入口
 * 5. 出口
 */
static unsigned int gpio_base[] = {
	0x56000000, /* GPACON */
	0x56000010, /* GPBCON */
	0x56000020, /* GPCCON */
	0x56000030, /* GPDCON */
	0x56000040, /* GPECON */
	0x56000050, /* GPFCON */
	0x56000060, /* GPGCON */
	0x56000070, /* GPHCON */
	0,          /* GPICON */
	0x560000D0, /* GPJCON */
};

static int led_open (struct inode *node, struct file *filp)
{
    struct Led_cdev *led_cdevp;
    led_cdevp = container_of(node->i_cdev,struct Led_cdev,cdev);
    filp->private_data = led_cdevp;


	/* 把LED引脚配置为输出引脚 */
	/* GPF5 - 0x56000050 */
    int bank = led_cdevp->led_pin >> 16;
    printk("0x%x\n",led_cdev.led_pin);
    int base = gpio_base[bank];

    int pin = led_cdevp->led_pin & 0xffff;
    led_cdevp->gpio_con = ioremap(base, 8);
    if (led_cdevp->gpio_con) {
        printk("ioremap(0x%x) = 0x%x\n", base, led_cdevp->gpio_con);
    }
    else {
        return -EINVAL;
    }
	
    led_cdevp->gpio_dat = led_cdevp->gpio_con + 1;

    *(led_cdevp->gpio_con) &= ~(3<<(pin * 2));
    *(led_cdevp->gpio_con) |= (1<<(pin * 2));

	return 0;
}

static ssize_t led_write (struct file *filp, const char __user *buf, size_t size, loff_t *off)
{
    struct Led_cdev *led_cdevp = filp->private_data;
	/* 根据APP传入的值来设置LED引脚 */
    unsigned char val;
    int pin = led_cdevp->led_pin & 0xffff;
	
    copy_from_user(&val, buf, 1);

    if (val)
    {
        /* 点灯 */
        *(led_cdevp->gpio_dat) &= ~(1<<pin);
    }
    else
    {
        /* 灭灯 */
        *(led_cdevp->gpio_dat) |= (1<<pin);
    }

	return 1; /* 已写入1个数据 */
}

static int led_release (struct inode *node, struct file *filp)
{
    struct Led_cdev *led_cdevp = filp->private_data;
    printk("iounmap(0x%x)\n", led_cdevp->gpio_con);
    iounmap(led_cdevp->gpio_con);
	return 0;
}


static struct file_operations myled_oprs = {
	.owner = THIS_MODULE,/* 驱动程序模块地址 */
	.open  = led_open,
	.write = led_write,
	.release = led_release,
};

static struct class *led_class;
static int led_cdev_init(void)
{
	int ret;

    /* 1:分配设备号 */
	if(alloc_chrdev_region(&led_cdev.dev_node,0,1,DEVICE_NAME) < 0)
	{
		printk("%s\n","can't alloc chrdev region!");
		return -1;
	}
    /* 2:为此构建了sysfs入口点 */
    led_class = class_create(THIS_MODULE,DEVICE_NAME);
    /* 3:将cdev和myled_oprs关联 */
	cdev_init(&led_cdev.cdev,&myled_oprs);
	led_cdev.cdev.owner = THIS_MODULE;
    /* 4:将主次设备号和cdev关联 */
    ret = cdev_add(&led_cdev.cdev,led_cdev.dev_node,1);
    if(ret)
    {
        printk("bad cdev\n");
        return ret;
    }
    /* 5:生成设备节点 */
    device_create(led_class,NULL,led_cdev.dev_node,NULL,"led");
	return 0;
}
static int led_cdev_exit(void)
{
    unregister_chrdev_region(led_cdev.dev_node,1);

    device_destroy(led_class,led_cdev.dev_node);

	cdev_del(&led_cdev.cdev);
	class_destroy(led_class);
	return 0;
}

static int led_probe(struct platform_device *pdev)
{
    struct resource	*res;
	
	/* 根据platform_device的资源进行ioremap */
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    if (res) {
        led_cdev.led_pin = res->start;
	}
	else {
		/* 获得pin属性 */
        of_property_read_s32(pdev->dev.of_node, "pin", &led_cdev.led_pin);
	}

    if (!led_cdev.led_pin)
    {
        printk("can not get pin for led\n");
        return -EINVAL;
    }

    led_cdev_init();
	return 0;
}

static int led_remove(struct platform_device *pdev)
{
    led_cdev_exit();
	return 0;
}


static const struct of_device_id of_match_leds[] = {
	{ .compatible = "jz2440_led", .data = NULL },
	{ /* sentinel */ }
};


struct platform_driver led_drv = {
	.probe		= led_probe,
	.remove		= led_remove,
	.driver		= {
		.name	= "myled",
		.of_match_table = of_match_leds, /* 能支持哪些来自于dts的platform_device */
	}
};


static int myled_init(void)
{
	platform_driver_register(&led_drv);
	return 0;
}

static void myled_exit(void)
{
	platform_driver_unregister(&led_drv);
}



module_init(myled_init);
module_exit(myled_exit);
MODULE_LICENSE("GPL");
MODULE_VERSION ("v0.1");
MODULE_VERSION ("2019.03.20");
MODULE_AUTHOR("l814699480@gmail.com");
MODULE_DESCRIPTION("jz2440-led driver");

