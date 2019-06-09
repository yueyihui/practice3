#include <linux/init.h>           // Macros used to mark up functions e.g. __init __exit
#include <linux/module.h>         // Core header for loading LKMs into the kernel
#include <linux/device.h>         // Header to support the kernel Driver Model
#include <linux/kernel.h>         // Contains types, macros, functions for the kernel
#include <linux/fs.h>             // Header for the Linux file system support
#include <linux/uaccess.h>        // Required for the copy to user function
#include <linux/thread_info.h>    // current macro
#define  DEVICE_NAME "ebbchar"    ///< The device will appear at /dev/ebbchar using this value
#define  CLASS_NAME  "ebb"        ///< The device class -- this is a character device driver

//These two state for practice requirements 1.
#define  STATE_A    0 //allocation=(1,4,5) max=(4,4,8)
#define  STATE_B    1 //allocation=(1,4,6) max=(4,6,8)

#define  PROCESS_A  0
#define  PROCESS_B  1
#define  PROCESS_C  2

MODULE_LICENSE("GPL");            ///< The license type -- this affects available functionality
MODULE_AUTHOR("Derek Molloy");    ///< The author -- visible when you use modinfo
MODULE_DESCRIPTION("A simple Linux char driver for the BBB");  ///< The description -- see modinfo
MODULE_VERSION("0.1");            ///< A version number to inform users

static int    devId;                  ///< Stores the device number -- determined automatically
static struct class*  ebbcharClass  = NULL; ///< The device-driver class struct pointer
static struct device* ebbcharDevice = NULL; ///< The device-driver device struct pointer
static pid_t  pid[3];             //pid[0] = process A, pid[1] = process B, pid[2] = process C

// The prototype functions for the character driver -- must come before the struct definition
static int     dev_open(struct inode *, struct file *);
static int     dev_release(struct inode *, struct file *);
static long practice_ioctl(struct file *, unsigned int, unsigned long);
static ssize_t dev_read(struct file *, char *, size_t, loff_t *);
static ssize_t dev_write(struct file *, const char *, size_t, loff_t *);

static struct file_operations fops =
{
   .open = dev_open,
   .read = dev_read,
   .write = dev_write,
   .compat_ioctl = practice_ioctl,
   .unlocked_ioctl = practice_ioctl,
   .release = dev_release,
};

static int __init ebbchar_init(void) {
    printk(KERN_INFO "EBBChar: Initializing the EBBChar LKM\n");

    // Try to dynamically allocate a major number for the device -- more difficult but worth it
    devId = register_chrdev(0, DEVICE_NAME, &fops);
    if (devId<0){
        printk(KERN_ALERT "EBBChar failed to register a major number\n");
        return devId;
    }
    printk(KERN_INFO "EBBChar: registered correctly with major number %d\n", devId);

    // Register the device class
    ebbcharClass = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(ebbcharClass)){                // Check for error and clean up if there is
        unregister_chrdev(devId, DEVICE_NAME);
        printk(KERN_ALERT "Failed to register device class\n");
        return PTR_ERR(ebbcharClass);          // Correct way to return an error on a pointer
    }
    printk(KERN_INFO "EBBChar: device class registered correctly\n");

    // Register the device driver
    ebbcharDevice = device_create(ebbcharClass, NULL, MKDEV(devId, 0), NULL, DEVICE_NAME);
    if (IS_ERR(ebbcharDevice)){               // Clean up if there is an error
        class_destroy(ebbcharClass);           // Repeated code but the alternative is goto statements
        unregister_chrdev(devId, DEVICE_NAME);
        printk(KERN_ALERT "Failed to create the device\n");
        return PTR_ERR(ebbcharDevice);
    }
    printk(KERN_INFO "EBBChar: device class created correctly\n"); // Made it! device was initialized
    return 0;
}

static long practice_ioctl(struct file *filp, unsigned int cmd, unsigned long arg) {
    switch (cmd) {
        case STATE_A:
            printk(KERN_INFO "start STATE_A");
            if (arg == PROCESS_A) {// determined which process is A
                printk("process A involked\n");
                pid[PROCESS_A] = current->pid; 
            } else if (arg == PROCESS_B) {// determined which process is B
                pid[PROCESS_B] = current->pid; 
                printk("process B involked\n");
            } else {// determined which process is C
                pid[PROCESS_C] = current->pid; 
                printk("process C involked\n");
            }
            break;
        case STATE_B:
            pid[1] = current->pid; 
            break;
        case 2:
            pid[2] = current->pid; 
            break;
    }
    return 0;
}

static void __exit ebbchar_exit(void) {
    device_destroy(ebbcharClass, MKDEV(devId, 0));     // remove the device
    class_unregister(ebbcharClass);                    // unregister the device class
    class_destroy(ebbcharClass);                       // remove the device class
    unregister_chrdev(devId, DEVICE_NAME);             // unregister the major number
    printk(KERN_INFO "EBBChar: Goodbye from the LKM!\n");
}

static int dev_open(struct inode *inodep, struct file *filep) {
    printk(KERN_INFO "dev_open EBBChar pid %d\n", current->pid);
    return 0;
}

static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset){
    current->pid;
    return 0;
}

static ssize_t dev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset) {
    printk(KERN_INFO "EBBChar pid %d\n", current->pid);
    return len;
}

static int dev_release(struct inode *inodep, struct file *filep) {
    printk(KERN_INFO "EBBChar: Device successfully closed\n");
    return 0;
}

module_init(ebbchar_init);
module_exit(ebbchar_exit);
