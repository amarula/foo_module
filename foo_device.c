// SPDX-License-Identifier: GPL-2.0-or-later
#include <linux/module.h>
#include <linux/fs.h>

#define DEVICE_NAME "foo_device"

static int major;

static int foo_open(struct inode *inodep, struct file *filep)
{
	pr_info("foo_device device opened\n");
	return 0;
}

static ssize_t foo_write(struct file *filep, const char *buffer,
			 size_t len, loff_t *offset)
{

	pr_info("Sorry, foo_device is read only\n");
	return -EFAULT;
}

static int foo_release(struct inode *inodep, struct file *filep)
{
	pr_info("foo_device device closed\n");
	return 0;
}

static ssize_t foo_read(struct file *filep, char *buffer, size_t len,
			loff_t *offset)
{
	int ret = 0;
	const char *msg = "foo_device read response\n";

	ret = copy_to_user(buffer, msg, strlen(msg));

	return ret == 0 ? len : -EFAULT;
}

static struct file_operations fops = {
	.open = foo_open,
	.read = foo_read,
	.write = foo_write,
	.release = foo_release,
};

static int __init foo_device_init(void)
{
	major = register_chrdev(0, DEVICE_NAME, &fops);

	if (major < 0) {
		pr_err("foo_device load failed\n");
		return major;
	}

	pr_info("foo_device module has been loaded: %d\n", major);
	return 0;
}

static void __exit foo_device_exit(void)
{
	unregister_chrdev(major, DEVICE_NAME);
	pr_info("foo_device module has been unloaded\n");
}

module_init(foo_device_init);
module_exit(foo_device_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Michael Trimarchi <michael@amarulasolutions.com>");
MODULE_DESCRIPTION("Foo Device Model for the linux kernel");
MODULE_VERSION("0.1");

