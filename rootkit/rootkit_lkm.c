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


// This is a pointer to the syscall table in memory, See 3.
static struct sysent *syscall_table;




// All
// New
// Handlers
// Go
// Here

static struct sysent new_read_handler = {

}




// this all depends on if we can change the memroy address in the sysent table on the fly without recompilatation
// this all depends on if we can change the memroy address in the sysent table on the fly without recompilatation
// this all depends on if we can change the memroy address in the sysent table on the fly without recompilatation
// this all depends on if we can change the memroy address in the sysent table on the fly without recompilatation
// this all depends on if we can change the memroy address in the sysent table on the fly without recompilatation




// Find the memory address of the sysent table and point to our handlers if found.
static int find_sysent_table (void) {

    // Store memory address of sysent table, See 4.
    old_syscall_table = sysent[SYS_read];

    if (old_syscall_table) {
        uprintf("Table found at address: %p\n", old_syscall_table);
        change_sysent_addresses(old_syscall_table); //add args
        return 0;
    }
    else {
        uprintf("Table not found.\n");
        return 1;
    }
}




static int change_sysent_addresses () {
    // Replace SYS_read with a custom handler
    // sysent[SYS_read] is a pointer to SYS_read in the sysent array

    //no args needed, just point to the start address of the new handler
    //sysent[SYS_read] = custom_syscall; 
    // sysent[SYS_read].sy_call = (sy_call_t *)custom_read; // Replace
}














// Create a handler for the LKM, See 5.
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