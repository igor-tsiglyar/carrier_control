obj-m += carrier_control.o
carrier_control-objs := sysfs_cc.o e1000_cc.o cc_main.o

KDIR=/usr/src/linux-headers-4.4.0-34-generic

cc:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) clean
