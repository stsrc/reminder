#include <linux/errno.h>
#include <linux/types.h>
#include <linux/string.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/printk.h>

MODULE_LICENSE("DUAL BSD/GPL");

struct cdev reminder_cdev; //IT SHOULDN'T BE HERE
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
	rt = alloc_chrdev_region(&dev, 0, 1, "reminder");
	if (rt)
		return rt;
	size_t msg_size = 100;
	message = kmalloc(msg_size, GFP_KERNEL);
	if (!message)
		return -ENOMEM;
	memset(message, 0, msg_size);
	return 0;
}

static int __exit reminder_exit(void)
{
	printk(KERN_EMERG "%s\n", message);
}

module_init(reminder_init);
module_exit(reminder_exit);
