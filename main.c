#include "device_file.h"
#include <linux/init.h>       /* module_init, module_exit */
#include <linux/module.h>     /* version info, MODULE_LICENSE, MODULE_AUTHOR, printk() */

MODULE_DESCRIPTION("Simple Linux driver");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Apriorit, Inc");

/*===============================================================================================*/
static int simple_driver_init(void)
{
    int result = 0;
    printk( KERN_NOTICE "Simple-driver: Initialization started\n" );

    result = register_device();
    return result;
}

/*===============================================================================================*/
static void simple_driver_exit(void)
{
    printk( KERN_NOTICE "Simple-driver: Exiting\n" );
    unregister_device();
}

/*===============================================================================================*/
module_init(simple_driver_init);
module_exit(simple_driver_exit);
