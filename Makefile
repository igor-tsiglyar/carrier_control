obj-m += carrier_control.o
carrier_control-objs := sysfs_cc.o e1000_cc.o cc_main.o

cc:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) clean
