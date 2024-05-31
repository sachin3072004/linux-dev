#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/fs.h>
#include<linux/cdev.h>
#include<linux/kdev_t.h>
#include<linux/delay.h>
#include<linux/miscdevice.h>
#include "ioctl_cmd.h"
struct class *c = NULL;
dev_t devNum;
struct cdev cd;
char kernelBuffer[100];
#define MAX_SIZE 1000
atomic_t device_available = ATOMIC_INIT(1);
atomic_t usage_count = ATOMIC_INIT(0);
kuid_t device_owner;
#define DEVICE_NAME "testingmisc"
int device_open(struct inode *i, struct file* f ){
        printk(KERN_INFO "DEVICE OPEN %d %d\n", __kuid_val(current_uid()), atomic_read(&usage_count));
        if(atomic_read(&usage_count) != 0 && !uid_eq(device_owner, current_uid())){
                printk(KERN_INFO "Device is busy1 \n");
                return -EBUSY;
        }
        if(!atomic_dec_and_test(&usage_count)){
                device_owner = current_uid();
        }else{
                printk(KERN_INFO "Device is busy2 \n");
                atomic_inc(&usage_count);
                return -EBUSY;
        }
        if(!atomic_dec_and_test(&device_available)){
                printk(KERN_INFO "Device is busy3 \n");
                atomic_inc(&device_available);
                return -EBUSY;
        }
        return 0;
}

int device_release(struct inode *i, struct file* f ){
        atomic_inc(&usage_count);
        atomic_inc(&device_available);
        printk(KERN_INFO "TEST_EXIT %d %d \n",atomic_read(&usage_count),atomic_read(&device_available));
        device_owner.val = 0;
        printk(KERN_INFO "DEVICE OPEN\n");
        return 0;
}

ssize_t device_read(struct file* f, char __user* buffer, size_t sz, loff_t* loff){
	int retVal = access_ok(buffer, 100);
	printk(KERN_INFO "DEVICE_READi FALED RETVAL %d\n", retVal);
	if(!retVal){
		printk("DEVICE_READi FALED");
		return -EFAULT;
	}
	char kernelBuffer[] = "KAKU Sachin Gupta ka kernel";
	int num = copy_to_user(buffer, kernelBuffer+*loff, sz);
	printk("Copy to user Buffer:  %s Kernel buffer: %s RetVal - %d SZ: %lu %lu\n",buffer, kernelBuffer, num, sz, sizeof(kernelBuffer));
	*loff = *loff + sz;
	printk(KERN_INFO "DEVICE READ LOFF %llu\n",*loff);
	return 0;
}

ssize_t device_write(struct file* f, const char __user* buffer, size_t sz, loff_t* loff){
	int retVal = access_ok(buffer, 100);
	printk(KERN_INFO "DEVICE_WRITE FALED RETVAL %d\n", retVal);
	if(!retVal){
		printk("DEVICE_WRITE FALED");
		return -EFAULT;
	}
	int num = copy_from_user(kernelBuffer + *loff, buffer, sz);
	printk("Copy from user buffer  %s NUM = = %d Size = %lu \n", kernelBuffer ,num , sz);
	printk(KERN_INFO "DEVICE WRITE LOFF  %llu \n",*loff);
	*loff += sz;
	return sz;
}

loff_t device_seek(struct file* f, loff_t offset, int orig){
	loff_t new_pos = 0;
	switch(orig){
		case 0:
			new_pos = offset;
			break;
		case 1:
			new_pos = f->f_pos + offset;
			break;
		case 2:
			new_pos = 100 - offset;
			break;
	}
	printk(KERN_INFO "Lseek1 Current Ptr == %llu", f->f_pos);
	f->f_pos = new_pos;
	printk(KERN_INFO "Lseek2 Current Ptr == %llu", f->f_pos);
	return new_pos;
}

long device_ioctl(struct file* file, unsigned int cmd, unsigned long l){
	char kernel_buffer[MAX_SIZE];
	unsigned char ch;
	switch(cmd){
		case MSG_IOCTL_GET_LENGTH :
			pr_info("Get Buffer Length \n");
			put_user(MAX_SIZE, (unsigned int*)l);
			break;
		case MSG_IOCTL_CLEAR_BUFFER:
			pr_info("CLEAR BUFFER Length\n");
			memset(kernel_buffer, 0, sizeof(kernel_buffer));
			break;
		case MSG_IOCTL_FULL_BUFFER:
			get_user(ch, (unsigned char*)l);
			pr_info("Fill Character: %c\n",ch);
			memset(kernel_buffer, ch, sizeof(kernel_buffer));
			kernel_buffer[MAX_SIZE-1] = '\0';
			printk(KERN_INFO "Kernel Buffer == %s\n",kernel_buffer);
			break;
		case GET_ADDRESS:
			break;
		case CHECK_ADDRESS:
			break;
		default:
			return ENOTTY;
			break;
	}
	printk(KERN_INFO "DEVICE_IOCTL  %u %lu \n", cmd, l );
	return 0;
}

struct file_operations fp = {
	.open = device_open,
	.read = device_read,
	.write = device_write,
	.release = device_release,
	.llseek = device_seek,
	.unlocked_ioctl = device_ioctl,
	.compat_ioctl = device_ioctl,
};

static struct miscdevice my_misc_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = DEVICE_NAME,
	.fops = &fp
};
static int __init test_init(void){
	if(misc_register(&my_misc_device)){
		pr_err("Could not register");
		return -EBUSY;
	}
	pr_err("Succeeded in registering %s", DEVICE_NAME);
	return 0;
}

static void test_exit(void){
	misc_deregister(&my_misc_device);
}

MODULE_LICENSE("GPL");
module_init(test_init);
module_exit(test_exit);
