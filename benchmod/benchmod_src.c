#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>

#define CREATE_TRACE_POINTS
#include "empty_tp.h"

#define PROC_ENTRY_NAME "benchmod"

long benchmod_ioctl(
        struct file *file,
        unsigned int ioctl_num,
        unsigned long ioctl_param)
{
    int ret = 0;

    trace_empty_ioctl_1b(0);

    return ret;
}

static const struct file_operations benchmod_operations = {
    .unlocked_ioctl = benchmod_ioctl
};


/* Register the character device in procfs */
static int __init benchmod_init(void)
{
    int ret = 0;
    printk(KERN_INFO "Init benchmod\n");

    proc_create_data(PROC_ENTRY_NAME, S_IRUGO | S_IWUGO, NULL,
            &benchmod_operations, NULL);

    return ret;
}

static void __exit benchmod_exit(void)
{
    printk(KERN_INFO "Exit benchmod\n");
    remove_proc_entry(PROC_ENTRY_NAME, NULL);
}

module_init(benchmod_init)
module_exit(benchmod_exit)

MODULE_LICENSE("GPL and additional rights");
MODULE_AUTHOR("Mohamad Gebai");
MODULE_DESCRIPTION("Testing benchmark module");

