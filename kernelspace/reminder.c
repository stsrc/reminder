#include <linux/errno.h>
#include <linux/types.h>
#include <linux/string.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/printk.h>

MODULE_LICENSE("DUAL BSD/GPL");

static struct cdev *reminder_cdev;
static char *message;

ssize_t reminder_read(struct file *f, char __user *buf, size_t nbytes, loff_t *ppos)
{
	ssize_t notwr = copy_from_user(message, buf, nbytes);
	if(notwr)
		return -ENOSPC;
	return nbytes;
}

int reminder_open(struct inode *node, struct file *f)
{
	if (f->fmode & FMODE_READ)
		return -EPERM;
	return 0;
}

int reminder_release(struct inode *node, struct file *f)
{
	return 0;
}



const struct file_operations reminder_fops = {
	.read = reminder_read,
	.open = reminder_open,
	.release = reminder_release,
}


static int __init reminder_init(void)
{
	int rt;
	dev_t dev;
	size_t msg_size = 32;
	rt = alloc_chrdev_region(&dev, 0, 1, "reminder");
	if (rt)
		return rt;
	reminder_cdev = cdev_alloc();
	if (!reminder_cdev) {
		unregister_chrdev_region(dev, 1);
		return -ENOMEM;
	}
	
	message = kmalloc(msg_size, GFP_KERNEL);
	if (!message) {
		kfree(reminder_cdev);
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
		kfree(reminder_cdev);
		unregister_chrdev_region(dev, 1);
		kfree(message);
		return rt;
	}
	return 0;
}

static int __exit reminder_exit(void)
{
	int rt;
	printk(KERN_EMERG "%s\n", message);
	cdev_del(reminder_cdev);
	unregister_chrdev_region(dev, 1);
	kfree(message);	
	return 0;
}

module_init(reminder_init);
module_exit(reminder_exit);
