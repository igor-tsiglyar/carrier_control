#include "fw_sysfs.h"
#include "fw.h"
#include <linux/netdevice.h>
#include <linux/sysfs.h>


static ssize_t udp_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    struct ethernet_port *port = container_of(kobj, struct ethernet_port, kobj);

    return sprintf(buf, "%d\n", port->udp);
}


static ssize_t udp_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    struct ethernet_port *port = container_of(kobj, struct ethernet_port, kobj);
    int udp;

    sscanf(buf, "%d", &udp);

    if (udp >= 0 && udp <= 65535) {
        port->udp = udp;
    } else {
        pr_notice("Illegal UDP port: %d", udp);
    }

    return count;
}


static ssize_t tcp_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    struct ethernet_port *port = container_of(kobj, struct ethernet_port, kobj);

    return sprintf(buf, "%d\n", port->tcp);
}


static ssize_t tcp_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    struct ethernet_port *port = container_of(kobj, struct ethernet_port, kobj);
    int tcp;

    sscanf(buf, "%d", &tcp);

    if (tcp >= 0 && tcp <= 65535) {
        port->tcp = tcp;
    } else {
        pr_notice("Illegal TCP port: %d", tcp);
    }

    return count;
}


static struct kobj_attribute udp_attribute = __ATTR_RW(udp);


static struct kobj_attribute tcp_attribute = __ATTR_RW(tcp);


static struct attribute *ethernet_port_default_attrs[] = {
    &tcp_attribute.attr,
    &udp_attribute.attr,
    NULL
};


static ssize_t ethernet_port_attr_show(struct kobject *kobj, struct attribute *attr, char *buf)
{
    struct kobj_attribute *attribute = container_of(attr, struct kobj_attribute, attr);

    if (!attribute->show) {
        return -EIO;
    }

    return attribute->show(kobj, attribute, buf);
}


static ssize_t ethernet_port_attr_store(struct kobject *kobj, struct attribute *attr, const char *buf, size_t count)
{
    struct kobj_attribute *attribute = container_of(attr, struct kobj_attribute, attr);

    if (!attribute->store) {
        return -EIO;
    }

    return attribute->store(kobj, attribute, buf, count);
}


static struct sysfs_ops ethernet_port_attr_ops = {
    .show = ethernet_port_attr_show,
    .store = ethernet_port_attr_store,
};


static void ethernet_port_release(struct kobject *kobj)
{
    kfree(container_of(kobj, struct ethernet_port, kobj));
}


static struct kobj_type ethernet_port_ktype = {
    .sysfs_ops = &ethernet_port_attr_ops,
    .release = ethernet_port_release,
    .default_attrs = ethernet_port_default_attrs,
};


static struct kset *ethernet_ports;


int create_module_dir(void)
{
    ethernet_ports = kset_create_and_add("firewall", NULL, kernel_kobj);

    if (!ethernet_ports) {
        pr_err("Cannot create /sys/firewall directory");
        return -ENOMEM;
    }

    return 0;
}

void destroy_module_dir(void)
{
    kset_unregister(ethernet_ports);
}

int create_ethernet_port_dir(struct ethernet_port *port)
{
    if (!port || !port->netdev) {
        return -EINVAL;
    }

    port->kobj.kset = ethernet_ports;

    if (kobject_init_and_add(&port->kobj, &ethernet_port_ktype, NULL, "%s", port->netdev->name)) {
        pr_err("Cannot create port dir for %s", port->netdev->name);
        return -EINVAL;
    }

    kobject_uevent(&port->kobj, KOBJ_ADD);

    return 0;
}


void destroy_ethernet_port_dir(struct ethernet_port *port)
{
    if (port) {
        kobject_put(&port->kobj);
    }
}

