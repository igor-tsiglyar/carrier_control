obj-m += firewall.o
firewall-objs := fw_sysfs.o fw_hook.o fw_main.o

fw:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) clean
