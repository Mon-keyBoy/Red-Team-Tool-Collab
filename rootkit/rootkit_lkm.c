// These files will need to be within the environment that you compile this code 
// but not within the environment that you execute that compiled file in. 



// Necessary headers 

// ???
#include <sys/param.h>

// Provides the DECLARE_MODULE macro for registering the module.
#include <sys/kernel.h>

// Contains struct module and functions to manage loadable kernel modules (LKMs).
// Defines constants like MOD_LOAD and MOD_UNLOAD for module lifecycle management.
#include <sys/module.h>

// ???
#include <sys/systm.h>

// Standard input/output library for C, allows for printing to the terminal and accepting input from it
#include <stdio.h>

// ???
#include <sys/sysent.h>

// ???
#include <sys/proc.h>

// ???
#include <sys/sysproto.h>




// This is temporary, figure out what you actually want to do.
// Most likely these will call different files, MOD_LOAD should call a file that start hooking syscalls and does the intended
// functionality of the rootkit.
// MOD_UNLOAD should implement log clearing (for what?) and a call to a file with the chosen persintance method
// I do not know what the default with be but most likely it will call a file that attempts to reload the module and if failed, created the persistant method
// The above line regarding the default case should take in account whether or not it is possible to hook a syscall (if it is a syscall)
// to shut down/reboot the system to pause it, excecute the desired operations, then continue with the shut down/reboot  

// Module event handler <rootkit_handler>
// Handles load/unload requests from kldload and kldunload.
static int rootkit_handler(struct module *module, int event, void *arg) {
    switch (event) {
    case MOD_LOAD:
        uprintf("Hello, Kernel!\n");  // Print message to userland
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
// Module name, metadata struct, ?, ?
DECLARE_MODULE(//fill this in);