// SPDX-License-Identifier: GPL-2.0-or-later
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/fs.h>

#define DEVICE_NAME "foo_dsp"
static int major;
module_param(major, int, 0);
MODULE_PARM_DESC(major, "Major device number");

static struct platform_device *pdev;  /* use in dev_*() */
static struct cdev foo_driver_cdev;

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
	.owner = THIS_MODULE,
	.open = foo_open,
	.read = foo_read,
	.write = foo_write,
	.release = foo_release,
};

static int __init foo_device_init(void)
{
	int ret = 0;
	dev_t devid;

	pdev = platform_device_alloc(DEVICE_NAME, 0);
	if (!pdev)
		return -ENOMEM;

	ret = platform_device_add(pdev);
	if (ret) {
		ret = -ENODEV;
		goto undo_platform_dev_alloc;
	}

	if (major) {
		devid = MKDEV(major, 0);
		ret = register_chrdev_region(devid, 1, DEVICE_NAME);
	} else {
		ret = alloc_chrdev_region(&devid, 0, 1, DEVICE_NAME);
		major = MAJOR(devid);
	}

	if (ret < 0) {
		pr_err("register-chrdev failed: %d\n", ret);
		goto undo_platform_dev_add;
	}

	if (!major) {
		major = ret;
		pr_debug("got dynamic major %d\n", major);
	}

	/* ignore minor errs, and succeed */
	cdev_init(&foo_driver_cdev, &fops);
	cdev_add(&foo_driver_cdev, devid, 1);

	pr_info("foo_device module has been loaded: %d\n", major);

	return 0;

undo_platform_dev_add:
	platform_device_del(pdev);
undo_platform_dev_alloc:
	platform_device_put(pdev);

	return ret;
}

static void __exit foo_device_exit(void)
{
	cdev_del(&foo_driver_cdev);
	unregister_chrdev_region(MKDEV(major, 0), 1);
	platform_device_unregister(pdev);
	pr_info("foo_device module has been unloaded\n");
}

module_init(foo_device_init);
module_exit(foo_device_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Michael Trimarchi <michael@amarulasolutions.com>");
MODULE_DESCRIPTION("Foo Device Model for the linux kernel");
MODULE_VERSION("0.1");

