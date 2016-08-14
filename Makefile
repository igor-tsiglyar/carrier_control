obj-m += carrier_control.o
carrier_control-objs := sysfs_cc.o cc_main.o

KDIR=/usr/src/kernels/3.10.0-327.28.2.el7.x86_64/

cc:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) clean
