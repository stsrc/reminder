#include <linux/errno.h>
#include <linux/types.h>
#include <linux/string.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/printk.h>

MODULE_LICENSE("DUAL BSD/GPL");

struct cdev *reminder_cdev; //IT SHOULDN'T BE HERE??
char *message;

ssize_t reminder_read(struct file *f, char __user *buf, size_t nbytes, loff_t *ppos)
{
	ssize_t notwr = copy_from_user(display_data, buf, nbytes);
	if(notwr)
		return -ENOSPC;
	return nbytes;
}

int reminder_open(struct inode *node, struct file *f)
{

}

int reminder_release(struct inode *node, struct file *f)
{

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
	size_t msg_size = 100;
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
		cdev_del(reminder_cdev); //SHOULD IT BE HERE?
		unregister_chrdev_region(dev, 1);
		return -ENOMEM;
	}
	memset(message, 0, msg_size);
	cdev_init(reminder_cdev, &reminder_fops);
	rt = cdev_add(reminder_cdev, dev, 1);
	if (rt) {
		cdev_del(reminder_cdev); //SHOULD IT BE HERE?
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
}

module_init(reminder_init);
module_exit(reminder_exit);
