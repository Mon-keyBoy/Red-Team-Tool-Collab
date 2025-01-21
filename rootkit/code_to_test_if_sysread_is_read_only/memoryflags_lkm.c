#include <sys/param.h>
#include <sys/kernel.h>
#include <sys/module.h>
#include <sys/systm.h>
#include <sys/sysent.h>
#include <sys/syscall.h>
#include <sys/proc.h>
#include <sys/sysproto.h>
#include <sys/types.h>
#include <vm/vm.h>
#include <vm/vm_map.h>


static struct sysent *original_sysread;

// function to check is sysread is read only
void check_memory_protection(vm_offset_t address) {
    vm_map_t map = kernel_map;  // Kernel's memory map
    vm_map_entry_t entry;

    vm_map_lock_read(map);  // Lock the map for reading
    if (vm_map_lookup_entry(map, address, &entry)) {
        uprintf("Address: %p, Protection: %d\n", (void *)address, entry->protection);
    } else {
        uprintf("Address: %p not found in kernel map.\n", (void *)address);
    }
    vm_map_unlock_read(map);  // Unlock the map
}

// Find the memory address of the sysent table and point to our handlers if found.
static int find_sysread_address (void) {

    // Store memory address of sysread table, See 4.

    original_sysread = &sysent[SYS_read];

    if (original_sysread->sy_call) {
        uprintf("Sysread found at address: %p\n", original_sysread->sy_call);
        check_memory_protection((vm_offset_t)original_sysread->sy_call)

        // sysent[SYS_read].sy_call = (sy_call_t *)custom_sysread; 
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
            uprintf("No error with finding sysread address.\n");
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