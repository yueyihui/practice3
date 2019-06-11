#include <linux/init.h>           // Macros used to mark up functions e.g. __init __exit
#include <linux/module.h>         // Core header for loading LKMs into the kernel
#include <linux/device.h>         // Header to support the kernel Driver Model
#include <linux/kernel.h>         // Contains types, macros, functions for the kernel
#include <linux/fs.h>             // Header for the Linux file system support
#include <linux/uaccess.h>        // Required for the copy to user function
#include <linux/thread_info.h>    // current macro
#include <linux/semaphore.h>
#include <linux/wait.h>
#include <linux/slab.h>

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
static pid_t  pid[3] = {-1, -1, -1};             //pid[0] = process A, pid[1] = process B, pid[2] = process C

DEFINE_SEMAPHORE(mutex);
wait_queue_head_t wait_queue;

int maxA;
int maxB;
int maxC;

int allocateA;
int allocateB;
int allocateC;

int needA;
int needB;
int needC;

int work;

bool flags = false;

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

    init_waitqueue_head(&wait_queue);

    // Try to dynamically allocate a major number for the device -- more difficult but worth it
    devId = register_chrdev(0, DEVICE_NAME, &fops);
    if (devId<0){
        printk(KERN_ALERT "EBBChar failed to register a major number\n");
        return devId;
    }

    // Register the device class
    ebbcharClass = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(ebbcharClass)){                // Check for error and clean up if there is
        unregister_chrdev(devId, DEVICE_NAME);
        printk(KERN_ALERT "Failed to register device class\n");
        return PTR_ERR(ebbcharClass);          // Correct way to return an error on a pointer
    }

    // Register the device driver
    ebbcharDevice = device_create(ebbcharClass, NULL, MKDEV(devId, 0), NULL, DEVICE_NAME);
    if (IS_ERR(ebbcharDevice)){               // Clean up if there is an error
        class_destroy(ebbcharClass);           // Repeated code but the alternative is goto statements
        unregister_chrdev(devId, DEVICE_NAME);
        printk(KERN_ALERT "Failed to create the device\n");
        return PTR_ERR(ebbcharDevice);
    }
    return 0;
}

static long practice_ioctl(struct file *filp, unsigned int cmd, unsigned long arg) {
    switch (cmd) {
        case STATE_A:

            if (!flags) {
                flags = true;
                needA = 3;
                needB = 0;
                needC = 3;

                allocateA = 1;
                allocateB = 4;
                allocateC = 5;

                work = 12 - allocateA - allocateB - allocateC;
            }

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

            break;
        case 2:

            break;
    }
    return 0;
}

static void __exit ebbchar_exit(void) {
    device_destroy(ebbcharClass, MKDEV(devId, 0));     // remove the device
    class_unregister(ebbcharClass);                    // unregister the device class
    class_destroy(ebbcharClass);                       // remove the device class
    unregister_chrdev(devId, DEVICE_NAME);             // unregister the major number
    printk(KERN_INFO "EBBChar: Goodbye from the Kernel!\n");
}

static int dev_open(struct inode *inodep, struct file *filep) {
    return 0;
}

static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset) {//get resource
    char* data = kmalloc(len, GFP_KERNEL);
    copy_from_user(data, buffer, len);
    char request = *data;
    kfree(data);
    if (current->pid == pid[PROCESS_A]) {

        if (needA == 0 || request > needA) {
            return 0;
        }  

checkA:
        down(&mutex);
        if (request > work) {
            up(&mutex);

            DECLARE_WAITQUEUE(process_a, current);
            add_wait_queue(&wait_queue, &process_a);
            set_current_state(TASK_INTERRUPTIBLE);
            schedule();
            remove_wait_queue(&wait_queue, &process_a);
            set_current_state(TASK_RUNNING);
            goto checkA;
        } else {
            up(&mutex);
        }

        allocateA += request;
        needA = needA - request;
        down(&mutex);
        work = work - request;
        up(&mutex);
        printk("%s\n","PROCESS_A obtain resource");
    } else if (current->pid == pid[PROCESS_B]) {

        if (needB == 0 || request > needB) {
            return 0;
        }  

checkB:
        down(&mutex);
        if (request > work) {
            up(&mutex);

            DECLARE_WAITQUEUE(process_b, current);
            add_wait_queue(&wait_queue, &process_b);
            set_current_state(TASK_INTERRUPTIBLE);
            schedule();
            remove_wait_queue(&wait_queue, &process_b);
            set_current_state(TASK_RUNNING);
            goto checkB;
        } else {
            up(&mutex);
        }

        allocateB += request;
        needB = needB - request;
        down(&mutex);
        work = work - request;
        up(&mutex);
        printk("%s\n","PROCESS_B obtain resource");
    } else {//PROCESS_C

        if (needC == 0 || request > needC) {
            return 0;
        }  

checkC:
        down(&mutex);
        if (request > work) {
            up(&mutex);

            DECLARE_WAITQUEUE(process_c, current);
            add_wait_queue(&wait_queue, &process_c);
            set_current_state(TASK_INTERRUPTIBLE);
            schedule();
            remove_wait_queue(&wait_queue, &process_c);
            set_current_state(TASK_RUNNING);
            goto checkC;
        } else {
            up(&mutex);
        }

        allocateC += request;
        needC = needC - request;
        down(&mutex);
        work = work - request;
        up(&mutex);
        printk("%s\n","PROCESS_C obtain resource");
    }

    return 0;
}

static ssize_t dev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset) {//release resource
    if (current->pid == pid[PROCESS_A]) {
        down(&mutex);
        work = work + allocateA;
        up(&mutex);
        wake_up(&wait_queue);
        printk("PROCESS_A release resource\n");
    } else if (current->pid == pid[PROCESS_B]) {
        down(&mutex);
        work = work + allocateB;
        up(&mutex);
        wake_up(&wait_queue);
        printk("PROCESS_B release resource\n");
    } else {//PROCESS_C
        down(&mutex);
        work = work + allocateC;
        up(&mutex);
        wake_up(&wait_queue);
        printk("PROCESS_C release resource\n");
    }
    return 0;
}

static int dev_release(struct inode *inodep, struct file *filep) {
    if (current->pid == pid[PROCESS_A]) {
        pid[PROCESS_A] = -2;
        if (pid[PROCESS_A] == -2 && pid[PROCESS_B] == -2 && pid[PROCESS_C] == -2) {//for reset all elements
            pid[PROCESS_A] = -1;
            pid[PROCESS_B] = -1;
            pid[PROCESS_C] = -1;
            flags = false;
        }
        printk(KERN_INFO "EBBChar: PROCESS_A Device successfully closed\n");
    } else if (current->pid == pid[PROCESS_B]) {
        pid[PROCESS_B] = -2;
        if (pid[PROCESS_A] == -2 && pid[PROCESS_B] == -2 && pid[PROCESS_C] == -2) {//for reset all elements
            pid[PROCESS_A] = -1;
            pid[PROCESS_B] = -1;
            pid[PROCESS_C] = -1;
            flags = false;
        }
        printk(KERN_INFO "EBBChar: PROCESS_B Device successfully closed\n");
    } else if (current->pid == pid[PROCESS_C]) {
        pid[PROCESS_C] = -2;
        if (pid[PROCESS_A] == -2 && pid[PROCESS_B] == -2 && pid[PROCESS_C] == -2) {//for reset all elements
            pid[PROCESS_A] = -1;
            pid[PROCESS_B] = -1;
            pid[PROCESS_C] = -1;
            flags = false;
        }
        printk(KERN_INFO "EBBChar: PROCESS_C Device successfully closed\n");
    } else {
        printk(KERN_INFO "EBBChar: Device successfully closed\n");
    }
    return 0;
}

module_init(ebbchar_init);
module_exit(ebbchar_exit);
