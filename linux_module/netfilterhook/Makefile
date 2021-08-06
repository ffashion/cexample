
obj-m := netfilterhook.o
KDIR := /lib/modules/$(shell uname -r)/build/
all:
	#modules 表示以模块方式将这个源码编译到内核
	make -C $(KDIR) M=$(PWD) modules
clean:
	rm -rf *.o *.mod *.symvers *.order *.mod.c .*.cmd *.ko