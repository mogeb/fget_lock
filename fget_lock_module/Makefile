obj-m += fget_lock_module.o
fget_lock_module-objs := fget_lock_module_src.o
ccflags-y += -I$(src)

all:
	make -C /lib/modules/`uname -r`/build M=$(PWD) SUBDIRS=$(PWD) modules

clean:
	make -C /lib/modules/`uname -r`/build M=$(PWD) clean
