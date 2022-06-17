
obj-m := sysfs.o
KDIR := /lib/modules/$(shell uname -r)/build/
all:
	make -C $(KDIR) M=$(PWD) modules
clean:
	rm -rf *.o *.mod *.symvers *.order *.mod.c .*.cmd *.ko *.dwo