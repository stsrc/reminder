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

MODULE_LICENSE("GPL");

static struct cdev *reminder_cdev = NULL;
static char *message = NULL;
static dev_t dev;
static struct class *reminder_class = NULL;
#define BUF_SIZE 32

ssize_t reminder_write(struct file *f, const char __user *buf, size_t nbytes, loff_t *ppos)
{
	ssize_t notwr = 0;
	int temp = 0;
	if (nbytes > BUF_SIZE) {
		nbytes = BUF_SIZE;
		temp = 1;
	}
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
	size_t msg_size = BUF_SIZE;
	struct device *device = NULL;
	rt = alloc_chrdev_region(&dev, 0, 1, "reminder");
	if (rt)
		return rt;

	reminder_class = class_create(THIS_MODULE, "reminder");
	if (IS_ERR(reminder_class)) {
		rt = PTR_ERR(reminder_class);
		unregister_chrdev_region(dev, 1);
		return rt;
	}
	reminder_cdev = cdev_alloc();
	if (!reminder_cdev) {
		class_destroy(reminder_class);
		unregister_chrdev_region(dev, 1);
		return -ENOMEM;
	}
	
	message = kmalloc(msg_size, GFP_KERNEL);
	if (!message) {
		kfree(reminder_cdev);
		class_destroy(reminder_class);
		/*
		 * it is legal to free cdev by kdev, 
		 * look to drivers/media/v4l2-core/v4l2-dev.c
		 */
		unregister_chrdev_region(dev, 1);
		return -ENOMEM;
	}
	memset(message, 0, msg_size);
	cdev_init(reminder_cdev, &reminder_fops);
	rt = cdev_add(reminder_cdev, dev, 1);
	if (rt) {
		class_destroy(reminder_class);
		kfree(reminder_cdev);
		unregister_chrdev_region(dev, 1);
		kfree(message);
		return rt;
	}
	device = device_create(reminder_class, NULL, dev, NULL, "reminder");
	if (IS_ERR(device)) {
		rt = PTR_ERR(device);
		cdev_del(reminder_cdev);
		class_destroy(reminder_class);
		kfree(message);
		unregister_chrdev_region(dev, 1);
		return rt;
	}
	return 0;
}

static void __exit reminder_exit(void)
{
	printk(KERN_EMERG "%s\n", message);
	device_destroy(reminder_class, dev);
	cdev_del(reminder_cdev);
	class_destroy(reminder_class);
	kfree(message);
	unregister_chrdev_region(dev, 1);
}

module_init(reminder_init);
module_exit(reminder_exit);
