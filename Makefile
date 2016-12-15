obj-m += carrier_control.o
carrier_control-objs := cc_sysfs.o cc_hook.o cc_main.o

cc:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) clean
