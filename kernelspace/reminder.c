#include <linux/errno.h>
#include <linux/types.h>
#include <linux/string.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/printk.h>
#include <asm/uaccess.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/slab.h> 
#include <linux/device.h>
#include <linux/err.h>
#include <linux/keyboard.h>
#include <linux/reboot.h>

MODULE_LICENSE("GPL");

static struct cdev *reminder_cdev = NULL;
static char *message = NULL;
static dev_t dev;
static struct class *reminder_class = NULL;
volatile static int wait_for_keypress = 1;
#define CHAR_LIMIT 160
#define LINE_WIDTH 80
#define PLVL KERN_EMERG
	
int notf_btn_pressed(struct notifier_block *nblock, unsigned long code, void *_param)
{
	wait_for_keypress = 0;
	return 0;	
}

static struct notifier_block nb = {
	.notifier_call = notf_btn_pressed
};


int notf_shutdown(struct notifier_block *nblock, unsigned long code, void *_param)
{
	present_message();
	return 0;
}

static struct notifier_block rb_nb = {
	.notifier_call = notf_shutdown
};

void print_line()
{
	for(int it = 0; it < LINE_WIDTH; it++)
		printk(PLVL "-");
	printk(PLVL "\n");
}

void present_message()
{
	printk(PLVL "Message from reminder module:\n");
	print_line();
	if (!message)
		printk(PLVL "!!!Message has not been written into the driver!!!\n");
	else
		printk(PLVL "     %s\n", message);
	print_line();
	printk(PLVL "End of message. Press any key to continue.\n");
	register_keyboard_notifier(&nb);
	while(wait_for_keypress){}
	unregister_keyboard_notifier(&nb);
}

ssize_t reminder_write(struct file *f, const char __user *buf, size_t nbytes, loff_t *ppos)
{
	ssize_t notwr = 0;
	int temp = 0;
	if (nbytes >= BUF_SIZE) {
		nbytes = BUF_SIZE - 1;
		temp = 1;
	}
	message = kmalloc(nbytes, GFP_KERNEL);
	if (!message)
		return -ENOMEM;
	memset(message, 0, msg_size);
	notwr = copy_from_user(message, buf, nbytes);
	if (temp)
		message[BUF_SIZE - 1] = '\0';
	if (notwr)
		return -ENOSPC;
	return nbytes;
}

int reminder_open(struct inode *node, struct file *f)
{
	if (f->f_mode & FMODE_READ)
		return -EPERM;
	return 0;
}

int reminder_release(struct inode *node, struct file *f)
{
	return 0;
}

const struct file_operations reminder_fops = {
	.write = reminder_write,
	.open = reminder_open,
	.release = reminder_release,
};

static int __init reminder_init(void)
{
	int rt;
	struct device *device = NULL;
	if (message)
		return -EEXIST;
	rt = alloc_chrdev_region(&dev, 0, 1, "reminder");
	if (rt)
		return rt;

	reminder_class = class_create(THIS_MODULE, "reminder");
	if (IS_ERR(reminder_class)) {
		rt = PTR_ERR(reminder_class);
		goto err;
	}
	reminder_cdev = cdev_alloc();
	if (!reminder_cdev) {
		rt = -ENOMEM;
		goto err;
	}
	cdev_init(reminder_cdev, &reminder_fops);
	rt = cdev_add(reminder_cdev, dev, 1);
	if (rt) {
		kfree(reminder_cdev);
		reminder_cdev = NULL;
		goto err;
	}
	device = device_create(reminder_class, NULL, dev, NULL, "reminder");
	if (IS_ERR(device)) {
		rt = PTR_ERR(device);
		goto err;
	}
	rt = register_reboot_notifier(&rb_nb);
	if (rt)
		goto err;
	return 0;
err:
	if (reminder_cdev)
		cdev_del(reminder_cdev);		
	if (reminder_class)
		class_destroy(reminder_class);
	if (message)
		kfree(message);
	unregister_chrdev_region(dev, 1);
	return rt;
}

static void __exit reminder_exit(void)
{
	present_message();
	unregister_reboot_notifier(&rb_nb);
	device_destroy(reminder_class, dev);
	cdev_del(reminder_cdev);
	class_destroy(reminder_class);
	kfree(message);
	unregister_chrdev_region(dev, 1);
}

module_init(reminder_init);
module_exit(reminder_exit);
