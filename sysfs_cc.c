#include "sysfs_cc.h"
#include "cc.h"
#include <linux/netdevice.h>


static struct ethernet_port_attribute {
    struct attribute attr;
    ssize_t (*show)(struct ethernet_port*, struct ethernet_port_attribute*, char*);
    ssize_t (*store)(struct ethernet_port*, struct ethernet_port_attribute*, const char*, size_t);
};


static ssize_t carrier_show(struct ethernet_port *port, struct ethernet_port_attribute *attr, char *buf)
{
    return sprintf(buf, "%d\n", netif_carrier_ok(port->netdev));
}


static ssize_t carrier_store(struct ethernet_port *port, struct ethernet_port_attribute *attr, const char *buf, size_t count)
{
    sscanf(buf, "%d", &port->has_carrier);

    if (port->has_carrier == 1) {
        carrier_on(port);
    } else if (port->has_carrier == 0) {
        carrier_off(port);
    }

    return count;
}


static struct ethernet_port_attribute carrier_attribute =
    __ATTR(carrier, S_IWUSR | S_IRWXO, carrier_show, carrier_store);


static struct attribute *ethernet_port_default_attrs[] = {
    &carrier_attribute.attr,
    NULL,
};


static ssize_t ethernet_port_attr_show(struct kobject *kobj, struct attribute *attr, char *buf)
{
    struct ethernet_port_attribute *attribute = container_of(attr, struct ethernet_port_attribute, attr);
    struct ethernet_port *port = container_of(kobj, struct ethernet_port, kobj);

    if (!attribute->show) {
        return -EIO;
    }

    return attribute->show(port, attribute, buf);
}


static ssize_t ethernet_port_attr_store(struct kobject *kobj, struct attribute *attr, const char *buf, size_t count)
{
    struct ethernet_port_attribute *attribute = container_of(attr, struct ethernet_port_attribute, attr);
    struct ethernet_port *port = container_of(kobj, struct ethernet_port, kobj);

    if (!attribute->store) {
        return -EIO;
    }

    return attribute->store(port, attribute, buf, count);
}


static struct sysfs_ops ethernet_port_ops = {
    .show = ethernet_port_attr_show,
    .store = ethernet_port_attr_store,
};


static void ethernet_port_release(struct kobject *kobj)
{
    kfree(container_of(kobj, struct ethernet_port, kobj));
}


static struct kobj_type ethernet_port_ktype = {
    .sysfs_ops = &ethernet_port_ops,
    .release = ethernet_port_release,
    .default_attrs = ethernet_port_default_attrs,
};


static ssize_t update_interfaces_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    destroy_ethernet_ports();
    create_ethernet_ports();

    return sprintf(buf, "%s", "");
}


static struct kobj_attribute update_interfaces_attribute =
    __ATTR(update_interfaces, S_IRUSR | S_IROTH, update_interfaces_show, NULL);


static struct kset *ethernet_ports;

int create_module_dir(void)
{
    ethernet_ports = kset_create_and_add("carrier_control", NULL, kernel_kobj);

    if (!ethernet_ports) {
        return -ENOMEM;
    }

    if (sysfs_create_file(&ethernet_ports->kobj, &update_interfaces_attribute.attr)) {
        kset_unregister(ethernet_ports);
        return -EINVAL;
    }

    return 0;
}

void destroy_module_dir(void)
{
    sysfs_remove_file(&ethernet_ports->kobj, &update_interfaces_attribute.attr);
    kset_unregister(ethernet_ports);
}

int create_ethernet_port_dir(struct ethernet_port *port)
{
    if (!(port && port->netdev)) {
        return -EINVAL;
    }

    port->kobj.kset = ethernet_ports;

    if (kobject_init_and_add(&port->kobj, &ethernet_port_ktype, NULL, "%s", port->netdev->name)) {
        destroy_ethernet_port_dir(port);
        return -ENOMEM;
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

