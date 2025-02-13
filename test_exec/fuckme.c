#include <sys/param.h>
#include <sys/module.h>
#include <sys/kernel.h>
#include <sys/socketvar.h>
#include <sys/protosw.h>
#include <sys/systm.h>
#include <sys/types.h>
#include <sys/mbuf.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <sys/proc.h>
#include <sys/sysent.h>
#include <sys/sysproto.h>
#include <sys/exec.h>
#include <net/pfil.h>
#include <netinet/ip_var.h>
#include <net/if.h>
#include <sys/syslog.h>
#include <sys/eventhandler.h>
#include <sys/libkern.h>  // Kernel-space string functions
#include <sys/sx.h> // for finding proc
#include <sys/queue.h> // for finding proc
#include <sys/lock.h>     // Locking mechanisms
#include <sys/sched.h>    // Needed for FIRST_THREAD_IN_PROC()
#include <sys/unistd.h> // for RFPROC
#include <sys/imgact.h> // For image_args

#include <amd64/include/pcb.h> // for pcb struct
// all the retarded includes for pcb
#include <amd64/include/fpu.h>
#include <amd64/include/segments.h>
#include <amd64/include/tss.h>


// since BSD is gay as hell and doesn't provide headers for these we declare them as external
extern int kern_execve(struct thread *td, struct image_args *args, struct mac *mac_p,
    struct vmspace *oldvmspace);





static int custom_copin_args_for_exec(struct image_args *args, const char *fname, enum uio_seg segflg, char **argv, char **envv) {
    u_long arg, env;
    int error;

    bzero(args, sizeof(*args));
    if (argv == NULL) {
        return (EFAULT);
    }

    /*
        * Allocate demand-paged memory for the file name, argument, and
        * environment strings.
        */
    error = exec_alloc_args(args);
    if (error != 0) {
        return (error);
    }

    /*
        * Copy the file name.
        */
    error = exec_args_add_fname(args, fname, segflg);
    if (error != 0) {
        printf("error at line %d\n", __LINE__);
        return error;
    }
    /*
        * extract arguments first
        */
    for (;;) {
        arg = (u_long)*argv++;
        if (arg == 0) {
            break;
        }
        error = exec_args_add_arg(args, (char *)(uintptr_t)arg,
            UIO_SYSSPACE);
        if (error != 0) {
            printf("error at line %d\n", __LINE__);
            break;
        }
    }

    /*
     * extract environment strings
     */
    if (envv) {
        for (;;) {
            env = (u_long)*envv++;
            if (env == 0) {
                break;
            }
            error = exec_args_add_env(args,
                (char *)(uintptr_t)env, UIO_SYSSPACE);
            if (error != 0) {
            printf("error at line %d\n", __LINE__);
            break;
            }
        }
    }

    return (error);
}





static void do_fuck(void) {
    struct image_args args;
    int error;
    // Define command and arguments: /bin/sh -c "echo hello"
    char *argv[] = { "/bin/sh", "-c", "echo hello | wall", NULL };
    char *envp[] = { "PATH=/bin:/usr/bin", NULL };  // Basic environment
    struct thread *td = curthread;
    error = custom_copin_args_for_exec(&args, argv[0], UIO_SYSSPACE, (char**)&argv, (char**)&envp);
    if (error != 0) {
        printf("[LKM] exec_copyin_args failed: %d\n", error);
        return;
    }

    error = kern_execve(td, &args, NULL, NULL);
    printf("[LKM] kern_execve returned: %d\n", error);

}

static int event_handler(struct module *module, int event, void *arg) {
    switch (event) {
        case MOD_LOAD:
            do_fuck();
            return 0;
        case MOD_UNLOAD:
            printf("[LKM] Module unloaded.\n");
            return 0;
        default:
            return EOPNOTSUPP;
    }
}

static moduledata_t module_data = {
    "kms",
    event_handler,
    NULL
};


DECLARE_MODULE(kms_lkm, module_data, SI_SUB_DRIVERS, SI_ORDER_MIDDLE);



