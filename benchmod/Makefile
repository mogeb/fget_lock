obj-m += benchmod.o
benchmod-objs := benchmod_src.o
ccflags-y += -I$(src)

all:
	make -C /lib/modules/`uname -r`/build M=$(PWD) SUBDIRS=$(PWD) modules

clean:
	make -C /lib/modules/`uname -r`/build M=$(PWD) clean
