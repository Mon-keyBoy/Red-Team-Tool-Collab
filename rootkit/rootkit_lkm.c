#include <sys/param.h>
#include <sys/kernel.h>
#include <sys/module.h>
#include <sys/systm.h>
#include <sys/sysent.h>
#include <sys/syscall.h>
#include <sys/proc.h>
#include <sys/sysproto.h>
#include <sys/types.h>

// for making memory addresses writable 
// #include <vm/vm.h>
// #include <vm/pmap.h>

// This is a pointer to sysfork in memory, See 3.
static sy_call_t *original_mkdir = NULL; // Store original sys_mkdir


/* Custom mkdir handler */
static int custom_mkdir(struct thread *td, void *syscall_args) {
    struct mkdir_args *uap = (struct mkdir_args *)syscall_args;
    char path[PATH_MAX];
    int error;

    /* Safely copy path from user space */
    error = copyinstr(uap->path, path, sizeof(path), NULL);
    if (error) {
        // printf("[LKM] Failed to copy user path (error: %d)\n", error);
        return error;
    }

    // printf("[LKM] mkdir called: Path = %s\n", path);

    /* Call original mkdir functionality */
    int ret = original_mkdir(td, syscall_args);

    // if (ret == 0)
    //     printf("[LKM] Directory %s created successfully!\n", path);
    // else
    //     printf("[LKM] mkdir failed with error code: %d\n", ret);

    return ret;
}

// Create a handler for the LKM, See 5.
static int rootkit_handler(struct module *module, int event, void *arg) {

    switch (event) {
    case MOD_LOAD:

        original_mkdir = sysent[SYS_mkdir].sy_call;  // Save original
        sysent[SYS_mkdir].sy_call = (sy_call_t *)custom_mkdir;  // Hook syscall
        //printf("[LKM] Hooked sys_mkdir!\n");
        break;
    case MOD_UNLOAD:
        //printf("Goodbye, Kernel!\n");
        break;
    default:
        return EOPNOTSUPP;  // Unsupported operation
    }
    return 0;
}

// Module metadata and registration, See 6.
static moduledata_t rootkit_mod = {
    "rootkit",              // Module name
    rootkit_handler,        // Event handler
    NULL                  // Extra data (optional)
};

// Register the module, See 7.
DECLARE_MODULE(rootkit, rootkit_mod, SI_SUB_DRIVERS, SI_ORDER_MIDDLE);