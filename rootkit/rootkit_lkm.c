// Necessary headers
#include <sys/kernel.h>
#include <sys/module.h>
#include <sys/systm.h>
#include <sys/sysent.h>
#include <sys/syscall.h>
#include <sys/proc.h>
#include <sys/sysproto.h>
#include <sys/types.h>


// This is a pointer to the syscall table in memory
static struct sysent *syscall_table;

// See if we can easily find the sysent table and point to it
static int find_sysent_table (void) {

    // The sysent table should be global in freeBSD/pfSense
    syscall_table = sysent;
    if (syscall_table) {
        uprintf("Table found at address: %p\n", syscall_table);
        return 0;
    }
    else {
        uprintf("Table not found.\n");
        return 1;
    }
}

// Module event handler <rootkit_handler>
// Handles load/unload requests from kldload and kldunload.
static int rootkit_handler(struct module *module, int event, void *arg) {

    // This is the result of looking for the memory address of the sysent table, it will be 0 if successfull and anything else for other cases
    int search_table_case = 0;
    switch (event) {
    case MOD_LOAD:
        search_table_case = find_sysent_table();
        if (search_table_case == 0) {
            uprintf("No error with finding sysent table address.\n");
        }
        else {
            uprintf("sysent table not found.\n");
        }
        break;
    case MOD_UNLOAD:
        uprintf("Goodbye, Kernel!\n");
        break;
    default:
        return EOPNOTSUPP;  // Unsupported operation
    }
    // Returning 0 indicates successful execution of the handler.
    // A non-zero value would indicate failure and stop the module from loading/unloading.
    return 0;
}

// Module metadata and registration
static moduledata_t rootkit_mod = {
    "rootkit",              // Module name
    rootkit_handler,        // Event handler
    NULL                  // Extra data (optional)
};


// Register the module
DECLARE_MODULE(rootkit, rootkit_mod, SI_SUB_DRIVERS, SI_ORDER_MIDDLE);
