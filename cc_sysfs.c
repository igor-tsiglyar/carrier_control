#include "cc_sysfs.h"
#include "cc.h"
#include <linux/netdevice.h>
#include <linux/sysfs.h>


static ssize_t carrier_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    struct ethernet_port *port = container_of(kobj, struct ethernet_port, kobj);

    return sprintf(buf, "%d\n", netif_carrier_ok(port->netdev));
}


static ssize_t carrier_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    struct ethernet_port *port = container_of(kobj, struct ethernet_port, kobj);
    int carrier;

    sscanf(buf, "%d", &carrier);

    if (carrier == 1) {
        carrier_on(port);
    } else if (carrier == 0) {
        carrier_off(port);
    } else {
        pr_notice("Unknown carrier value: %d", carrier);
    }

    return count;
}


static ssize_t packet_drop_rate_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    struct ethernet_port *port = container_of(kobj, struct ethernet_port, kobj);

    return sprintf(buf, "%d\n", port->packet_drop_rate);
}


static ssize_t packet_drop_rate_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    struct ethernet_port *port = container_of(kobj, struct ethernet_port, kobj);
    int packet_drop_rate;

    sscanf(buf, "%d", &packet_drop_rate);

    if (packet_drop_rate >= 0 && packet_drop_rate <= 100) {
        port->packet_drop_rate = packet_drop_rate;
    } else {
        pr_notice("Illegal packet_drop_rate value: %d", packet_drop_rate);
    }

    return count;
}


static ssize_t bit_corrupt_rate_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    struct ethernet_port *port = container_of(kobj, struct ethernet_port, kobj);

    return sprintf(buf, "%d\n", port->bit_corrupt_rate);
}


static ssize_t bit_corrupt_rate_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    struct ethernet_port *port = container_of(kobj, struct ethernet_port, kobj);
    int bit_corrupt_rate;

    sscanf(buf, "%d", &bit_corrupt_rate);

    if (bit_corrupt_rate >= 0 && bit_corrupt_rate <= 100) {
        port->bit_corrupt_rate = bit_corrupt_rate;
    } else {
        pr_notice("Illegal bit_corrupt_rate value: %d", bit_corrupt_rate);
    }

    return count;
}


static struct kobj_attribute carrier_attribute = __ATTR_RW(carrier);


static struct kobj_attribute packet_drop_rate_attribute = __ATTR_RW(packet_drop_rate);


static struct kobj_attribute bit_corrupt_rate_attribute = __ATTR_RW(bit_corrupt_rate);


static struct attribute *ethernet_port_default_attrs[] = {
    &carrier_attribute.attr,
    &packet_drop_rate_attribute.attr,
    &bit_corrupt_rate_attribute.attr,
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


static ssize_t update_interfaces_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    destroy_ethernet_ports();
    create_ethernet_ports();

    return sprintf(buf, "%s", "");
}


static struct kobj_attribute update_interfaces_attribute = __ATTR_RO(update_interfaces);


static struct kset *ethernet_ports;


int create_module_dir(void)
{
    ethernet_ports = kset_create_and_add("carrier_control", NULL, kernel_kobj);

    if (!ethernet_ports) {
        pr_err("Cannot create /sys/carrier_control directory");
        return -ENOMEM;
    }

    if (sysfs_create_file(&ethernet_ports->kobj, &update_interfaces_attribute.attr)) {
        pr_err("Cannot create /sys/carrier_control/update_attributes file");
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

