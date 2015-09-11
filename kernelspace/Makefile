ccflags-y += -DDEBUG -std=gnu99 -Wno-declaration-after-statement
ifneq ($(KERNELRELEASE),)
	obj-m := reminder.o

else
	KERNELDIR ?= /lib/modules/$(shell uname -r)/build
	PWD := $(shell pwd)

default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

endif
