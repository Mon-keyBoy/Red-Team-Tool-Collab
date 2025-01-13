// These files will need to be within the environment that you compile this code 
// but not within the environment that you execute that compiled file in. 



// Necessary headers 

// Contains basic system parameters and macros used throughout the kernel.
#include <sys/param.h>

// Provides the DECLARE_MODULE macro for registering the module.
#include <sys/kernel.h>

// Contains struct module and functions to manage loadable kernel modules (LKMs).
// Defines constants like MOD_LOAD and MOD_UNLOAD for module lifecycle management.
#include <sys/module.h>

// We can't use <stdio> in kernel level application but this allows for uprintf
#include <sys/systm.h>

// Declares the sysent structure (syscall table), which holds syscall entries.
#include <sys/sysent.h>

// ???
#include <sys/proc.h>

// ???
#include <sys/sysproto.h>


// sysent is the syscall table in freebsd/pfsense, is it an imported variable and a structure
// This is a pointer to the syscall table in memory
static struct sysent *syscall_table;

// See if we can easily find the sysent table and point to it
static int find_sysent_table (void) {

    // The sysent table should be global in freeBSD/pfSense
    // Changing syscall_table will not change the memory addresses within the global sysent array
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






// default should implement some kind of persistance method.  
//Ideally it will somehow recreate the rootkit binary and re-load it upon the machine starting back up.

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
// This struct registers the module with the kernel, linking the name (rootkit) with the handler.
static moduledata_t rootkit_mod = {
    "rootkit",              // Module name
    rootkit_handler,        // Event handler
    NULL                  // Extra data (optional)
};


// Register the module
// What does it do???
// Module name, metadata struct, subsustem level the module will be loaded at, order in which the module is loaded 
// "rootkit", <rootkit_mod>, initialized at the driver subsystem level, will be loaded after critical drivers but before less important ones 
DECLARE_MODULE(rootkit, rootkit_mod, SI_SUB_DRIVERS, SI_ORDER_MIDDLE);