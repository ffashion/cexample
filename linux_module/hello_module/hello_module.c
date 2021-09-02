#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
MODULE_LICENSE("GPL");
MODULE_AUTHOR("ffashion");


static int hello_init(void){
    printk("hello init\n");
    return 0;
}
static void hello_exit(void){
    printk("hello exit\n");
    return;
}

module_init(hello_init);


module_exit(hello_exit);
