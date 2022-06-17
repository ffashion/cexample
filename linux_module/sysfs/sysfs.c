/*general module */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>

/*network*/
#include <linux/skbuff.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <linux/tcp.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>

/*sysfs*/
#include <linux/sysfs.h>
#include <linux/kobject.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ffashion");
struct kobject *obj;
int test_parm = 1;
// /sys/module/sysfs/parameters/test_parm
module_param(test_parm, int, 0644);
// module_param_named()
// module_param_array()
static char *param_buf;
// function for many symbol data enter
static ssize_t __used store_value(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count){ 
    printk(KERN_ALERT "you entered %s\n", buf);
    strncpy(param_buf, buf, PAGE_SIZE - 1);
    return count;
}
static struct kobj_attribute store_val_attribute = __ATTR( put_parameters, 0220, NULL, store_value);
static struct attribute *register_attrs[] = {
    &store_val_attribute.attr,
    NULL,   /* NULL terminate the list*/
};
static struct attribute_group  reg_attr_group = {
    .attrs = register_attrs
};

static int __init module_sysfs_init(void) {
    param_buf = kzalloc(PAGE_SIZE, GFP_KERNEL);
    // /sys/kernel/test_1025_sym dir
    obj = kobject_create_and_add("test_1025_sym", kernel_kobj);
    if (!obj)
        return -ENOMEM;

    //create attributes (files)
    if (sysfs_create_group(obj, &reg_attr_group)) {
        kobject_put(obj);
        return -ENOMEM;
    }
    return 0;
}
static void __exit module_sysfs_exit(void) {
    if (obj) {
        printk("free %p kobject\n", obj);
        kobject_del(obj);
    }
}



module_init(module_sysfs_init);
module_exit(module_sysfs_exit);

