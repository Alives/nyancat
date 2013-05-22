ifneq ($(KERNELRELEASE),)
	obj-m := nyancat_cdev_module.o
	nyancat_cdev_module-y := nyancat_cdev.o renderer.o
else
KERNELDIR?=/lib/modules/$(shell uname -r)/build
PWD:=$(shell pwd)

default: module

module:
	make -C $(KERNELDIR) M=$(PWD) modules

clean:
	make -C $(KERNELDIR) M=$(PWD) clean

endif
