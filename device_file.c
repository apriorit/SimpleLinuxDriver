#include "device_file.h"
#include <linux/fs.h>     /* file stuff */
#include <linux/printk.h> /* pr_* */
#include <linux/errno.h>  /* error codes */
#include <linux/module.h> /* THIS_MODULE */
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h> /* copy_to_user() */

static const char g_s_Hello_World_string[] = "Hello world from kernel mode!\n";
static const ssize_t g_s_Hello_World_size = sizeof(g_s_Hello_World_string);

static ssize_t device_file_read(
    struct file *file_ptr, char __user *user_buffer, size_t count, loff_t *position)
{
    pr_notice("Simple-driver: Read from device file offset = %i, read bytes count = %u\n", (int)*position, (unsigned int)count);

    if (*position >= g_s_Hello_World_size)
        return 0;

    if (*position + count > g_s_Hello_World_size)
        count = g_s_Hello_World_size - *position;

    if (copy_to_user(user_buffer, g_s_Hello_World_string + *position, count) != 0)
        return -EFAULT;

    *position += count;
    return count;
}

static const struct file_operations simple_driver_fops =
{
    .owner = THIS_MODULE,
    .read = device_file_read,
};

static const char device_name[] = "simple-driver";
static const char class_name[] = "simple-driver-class";
dev_t g_devno = 0;
struct cdev g_cdev = {};
static struct class *g_class = NULL;
static struct device *g_device = NULL;

int register_device(void)
{
    int result = 0;

    pr_notice("Simple-driver: register_device() is called.\n");

    unsigned baseminor = 0;
    unsigned minor_count_required = 1;
    result = alloc_chrdev_region(&g_devno, baseminor, minor_count_required, device_name);
    if (result)
    {
        pr_err("Simple-driver: alloc_chrdev_region failed: %d\n", result);
        goto err_out;
    }

    cdev_init(&g_cdev, &simple_driver_fops);
    g_cdev.owner = THIS_MODULE;
    result = cdev_add(&g_cdev, g_devno, minor_count_required);
    if (result)
    {
        pr_err("Simple-driver: cdev_add failed: %d\n", result);
        goto err_unregister_chrdev_region;
    }

    g_class = class_create(class_name);
    if (IS_ERR(g_class))
    {
        result = PTR_ERR(g_class);
        pr_err("Simple-driver: class_create failed: %d\n", result);
        goto err_cdev_del;
    }

    g_device = device_create(g_class, NULL, g_devno, NULL, device_name);
    if (IS_ERR(g_device))
    {
        result = PTR_ERR(g_device);
        pr_err("Simple-driver: device_create failed: %d\n", result);
        goto err_class_destroy;
    }

    pr_notice("Simple-driver: Registered character device with major number = %i, minor number = %i\n", MAJOR(g_devno), MINOR(g_devno));
    return 0;

err_class_destroy:
    if (!IS_ERR_OR_NULL(g_class))
    {
        class_destroy(g_class);
        g_class = NULL;
    }

err_cdev_del:
    cdev_del(&g_cdev);

err_unregister_chrdev_region:
    if (g_devno)
    {
        unsigned minor_count_allocated = 1;
        unregister_chrdev_region(g_devno, minor_count_allocated);
        g_devno = 0;
    }

err_out:
    return result;
}

void unregister_device(void)
{
    pr_notice("Simple-driver: unregister_device() is called\n");

    if (!IS_ERR_OR_NULL(g_device))
    {
        device_destroy(g_class, g_devno);
        g_device = NULL;
    }

    if (!IS_ERR_OR_NULL(g_class))
    {
        class_destroy(g_class);
        g_class = NULL;
    }

    cdev_del(&g_cdev);

    if (g_devno)
    {
        unsigned minor_count_allocated = 1;
        unregister_chrdev_region(g_devno, minor_count_allocated);
        g_devno = 0;
    }
    pr_info("Simple-driver: Unregistered\n");
}
