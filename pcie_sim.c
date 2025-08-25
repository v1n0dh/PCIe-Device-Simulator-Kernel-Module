#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>
#include <linux/mutex.h>
#include <linux/timer.h>

#define DEVICE_NAME "pcie_sim"
#define REG_SIZE 256

static dev_t dev_number;
static struct cdev pcie_cdev;
static uint8_t device_regs[REG_SIZE];
static struct mutex device_lock;
static struct timer_list irq_timer;

// Simulated PCIe interrupt function
static void irq_simulator(struct timer_list *t) {
    printk(KERN_INFO "pcie_sim: Simulated interrupt triggered!\n");
    mod_timer(&irq_timer, jiffies + msecs_to_jiffies(2000)); // repeat every 2s
}

// IOCTL commands for read/write
#define PCIE_WRITE_REG 0x01
#define PCIE_READ_REG  0x02

static long pcie_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
    uint32_t val;

    if (cmd == PCIE_WRITE_REG) {
        if (copy_from_user(&val, (uint32_t __user *)arg, sizeof(val)))
            return -EFAULT;
        mutex_lock(&device_lock);
        device_regs[0] = val & 0xFF;
        mutex_unlock(&device_lock);
        printk(KERN_INFO "pcie_sim: Wrote %u to register 0\n", val);
    } else if (cmd == PCIE_READ_REG) {
        mutex_lock(&device_lock);
        val = device_regs[0];
        mutex_unlock(&device_lock);
        if (copy_to_user((uint32_t __user *)arg, &val, sizeof(val)))
            return -EFAULT;
        printk(KERN_INFO "pcie_sim: Read %u from register 0\n", val);
    } else {
        return -EINVAL;
    }
    return 0;
}

// Open device
static int pcie_open(struct inode *inode, struct file *file) {
    printk(KERN_INFO "pcie_sim: Device opened\n");
    return 0;
}

// Release device
static int pcie_release(struct inode *inode, struct file *file) {
    printk(KERN_INFO "pcie_sim: Device closed\n");
    return 0;
}

// File operations structure
static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = pcie_open,
    .release = pcie_release,
    .unlocked_ioctl = pcie_ioctl,
};

// Module initialization
static int __init pcie_init(void) {
    int ret;
    mutex_init(&device_lock);

    // Allocate device number
    ret = alloc_chrdev_region(&dev_number, 0, 1, DEVICE_NAME);
    if (ret < 0) return ret;

    // Initialize and add cdev
    cdev_init(&pcie_cdev, &fops);
    ret = cdev_add(&pcie_cdev, dev_number, 1);
    if (ret < 0) return ret;

    // Setup simulated interrupt timer
    timer_setup(&irq_timer, irq_simulator, 0);
    mod_timer(&irq_timer, jiffies + msecs_to_jiffies(2000));

    printk(KERN_INFO "pcie_sim: Module loaded\n");
    return 0;
}

// Module cleanup
static void __exit pcie_exit(void) {
    del_timer_sync(&irq_timer);
    cdev_del(&pcie_cdev);
    unregister_chrdev_region(dev_number, 1);
    printk(KERN_INFO "pcie_sim: Module unloaded\n");
}

module_init(pcie_init);
module_exit(pcie_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Vinodh Balakrishna");
MODULE_DESCRIPTION("Simulated PCIe device kernel module for system-level driver testing");
