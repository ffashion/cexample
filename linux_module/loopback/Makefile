
BEAR = $(command -v bear)
obj-m := loopback.o
KDIR := /lib/modules/$(shell uname -r)/build/
all:
ifdef BEAR
		bear -a make -C $(KDIR) M=$(PWD) modules
else
		make -C $(KDIR) M=$(PWD) modules
endif

clean:
	rm -rf *.o *.mod *.symvers *.order *.mod.c .*.cmd *.ko *.dwo