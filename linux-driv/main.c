#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/device.h>

// #include <stdio.h> 
// #include <stdlib.h> 

//-------------------------------------------------------------
#define CHANGE_MESSAGE   100
#define REVERSE_MESSAGE  101

#define f_mess     "the cake is a lie"
//#define f_mess_rev "eil a si ekac eht"
#define s_mess     "Science isnt about WHY, its about WHY NOT!"
//#define s_mess_rev "!TON YHW tuoba sti ,YHW tuoba tnsi ecneicS"

int active_connection = 0;
int mess_mode         = 0;
int reverse_text      = 0;

static int major = 0;
static struct class* mpp_class = NULL;
static struct device* mpp_device = NULL;
//-------------------------------------------------------------
void applyOp(char *r, const size_t size)
{
    int i = 0;
    char s[size];

    for (i = 0; i<size; i++)
        s[size-i-1] = r[i];

    

    //char* ptr = (char*)kmalloc(size*(sizeof(char)), GFP_KERNEL);
    //ptr[size-i-1] = r[i];
    // int begin, end, count = 0;
    // while (s[count] != '\0')
    //     count++;
    // end = count - 1;
    // for (begin = 0; begin < count; begin++) {
    //     r[begin] = s[end];
    //     end--;
    // }
    // r[begin] = '\0';
    //return ptr;
}
//-------------------------------------------------------------
static int mpp_open(struct inode *inode, struct file *filp) {
    if (active_connection)
        return 0;
    printk("MPP_opened\n");
    active_connection = 1;
    return 0; 
}

static int mpp_release(struct inode *inode, struct file *f) {
    if (!active_connection)
        return 0;
    printk("MPP_closed\n");
    active_connection = 0;
    return 0;
}

/**
 * Operace read bude vracet vámi 
 * definovaný řetězec, případně 
 * jeho délku.
 */
char buf_cp[100];
int buf_size = 0;

static ssize_t mpp_read(struct file *f, char *buf, size_t size, loff_t *offset){
    printk("MPP_read\n");
    // const int mess_len = (mess_mode)?(17):(42);
    // char *out_mes= (mess_mode)?(f_mess):(s_mess);

    printk("Before: %s\n", buf_cp);
    printk("Before: %d\n", buf_size);

    if (reverse_text){
        int i = 0;
        char s[buf_size+1];
        for (i = 0; i<buf_size; i++)
            s[buf_size-i-1] = buf_cp[i];
        s[buf_size] = '\0';
        for (i = 0; i<buf_size; i++)
            buf_cp[i]=s[i];
    }

    if (mess_mode){
        int i = 0;
        for (i = 0; i<buf_size; i++)
            if (buf_cp[i]=='a')
                buf_cp[i]='4';
    }

    copy_to_user(buf, buf_cp, min(size, buf_size));

    printk("Data: %s\n", buf_cp);
    

    // int i = 0;
    // char s[mess_len+1];

    // if(reverse_text){
    //     for (i = 0; i<mess_len; i++)
    //         s[mess_len-i-1] = out_mes[i];
    //     s[mess_len] = '\0';

        
    // }
    // else{
    //     copy_to_user(buf, out_mes, min(size, mess_len));

    //     printk("Data: %s\n", out_mes);
    // }

    return min(size, buf_size);
}

/**
 * Store data given from user to some storage
 * 
 * Operace write bude vypisovat obsah bufferu 
 * v textové podobě prostřednictvím ladícíh 
 * zpráv jádra (printk) zobrazitelných příkazem
 * dmesg.
 */
static ssize_t mpp_write(struct file *f, const char *buf, size_t size, loff_t *offset) {
    printk("MPP_write\n");
    buf_size = size;
    //copy from user space
    copy_from_user(buf_cp, buf, buf_size);
    //print to k
    printk("Data of %d bytes received\n", size);
    printk("Data: %s\n", buf_cp);
    //return size
    return size;
}

/**
 * 
 */
static long mpp_ioctl (struct file * f, unsigned int request, unsigned long param){
    printk("MPP_ioctl\n");

    if (request == CHANGE_MESSAGE){
        mess_mode = !mess_mode;
    }
    else if (request == REVERSE_MESSAGE){
        reverse_text = !reverse_text;
    }

    printk("MPP_ioctl applied mode %d\n", request);
    return 0;
}   

//-------------------------------------------------------------
struct file_operations mpp_fops = {
	.owner = THIS_MODULE,
	.llseek = NULL,
	.read = mpp_read,
	.write = mpp_write,
    //.aio_read = NULL,
    //.aio_write = NULL,
    //.readdir = NULL,
    .poll = NULL, 
    .unlocked_ioctl = mpp_ioctl,
	.compat_ioctl = NULL,
    .mmap = NULL,
	.open = mpp_open,
    .flush = NULL, 
	.release = mpp_release,
    .fsync = NULL,
    //.aio_fsync = NULL,
    .fasync = NULL,
    .lock = NULL,
    .sendpage = NULL,
    .get_unmapped_area = NULL,
    .check_flags = NULL,
    .flock = NULL,
    .splice_write = NULL,
    .splice_read = NULL,
    .setlease = NULL,
    .fallocate = NULL   
};

// loading & unloading of modules --s---------------------------
static int __init mpp_module_init(void)
{
        printk ("MPP module is loaded\n");
        major = register_chrdev(0, "mpp", &mpp_fops);

        mpp_class = class_create(THIS_MODULE, "mpp");

        mpp_device = device_create(mpp_class, NULL, MKDEV(major, 0), NULL, "mpp");

        return 0;
}

static void __exit mpp_module_exit(void)
{
 	printk ("MPP module is unloaded\n");
    device_destroy(mpp_class, MKDEV(major, 0));
    class_unregister(mpp_class);
    class_destroy(mpp_class);
    unregister_chrdev(major, "mpp");
    return;
}

module_init(mpp_module_init);
module_exit(mpp_module_exit);

MODULE_LICENSE("GPL");