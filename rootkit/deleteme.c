// this was given by chatgpt when asked to make a module that hooks syscalls

#include <sys/param.h>
#include <sys/module.h>
#include <sys/kernel.h>
#include <sys/sysent.h>
#include <sys/systm.h>
#include <sys/proc.h>
#include <sys/sysproto.h>

// Pointer to the original syscall
static sy_call_t *original_open;

// Custom syscall handler (replaces sys_open)
static int hooked_open(struct thread *td, void *syscall_args) {
    struct open_args *uap = (struct open_args *)syscall_args;
    
    // Print intercepted file path
    uprintf("Intercepted open(): %s\n", uap->path);
    
    // Call the original sys_open after logging
    return original_open(td, syscall_args);
}

// Module load function
static int load_hook(struct module *module, int event, void *arg) {
    switch (event) {
    case MOD_LOAD:
        uprintf("Syscall Hook Module Loaded\n");
        
        // Replace sys_open with hooked_open
        original_open = sysent[SYS_open].sy_call;
        sysent[SYS_open].sy_call = (sy_call_t *)hooked_open;
        break;

    case MOD_UNLOAD:
        // Restore original sys_open on unload
        sysent[SYS_open].sy_call = original_open;
        uprintf("Syscall Hook Module Unloaded\n");
        break;

    default:
        return EOPNOTSUPP;
    }
    return 0;
}

// Module metadata
static moduledata_t hook_mod = {
    "syscall_hook",
    load_hook,
    NULL
};

DECLARE_MODULE(syscall_hook, hook_mod, SI_SUB_DRIVERS, SI_ORDER_MIDDLE);





/*
How It Works:
sysent[SYS_open].sy_call points to the original sys_open syscall handler.
The LKM replaces this pointer with the custom hooked_open function.
Every time a file is opened, hooked_open is executed, logging the path.
After logging, the original syscall (original_open) is invoked to proceed as usual.



Security Considerations:
System Instability:
Hooking syscalls incorrectly can cause kernel panics. Always restore the original syscall when unloading.
Version Compatibility:
Syscall table layouts (sysent[]) can change between FreeBSD versions, which may break hardcoded hooks.
Integrity Protection (W^X):
FreeBSD employs W^X (Write XOR Execute) protections. Disabling it may be necessary if modifying syscall tables directly.



Advanced Topics:
Hooking Multiple Syscalls:

sysent[SYS_read].sy_call = (sy_call_t *)hooked_read;
sysent[SYS_write].sy_call = (sy_call_t *)hooked_write;
Hooking Custom Syscalls (Ioctl/Exec):
Hook SYS_execve to intercept command execution.

Conditional Hooking (Per-Process Hooks):
Modify only if the calling process matches a condition:

if (strcmp(td->td_proc->p_comm, "sshd") == 0) {
    uprintf("SSH process intercepted\n");
}



Summary:
Syscall hooking allows kernel-level interception of critical system calls.
This module demonstrates how to hook the open syscall, monitor file access, and log results.
By carefully managing load/unload events and restoring the original syscall, you ensure system stability.
*/

