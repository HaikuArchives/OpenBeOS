/* arp.c
 * simple arp implentation
 */
 
#include <stdio.h>

#include "mbuf.h"
#include "net_misc.h"
#include "arp/arp.h"
#include "protocols.h"
#include "net_module.h"
#include "nhash.h"
#include "pools.h"
#include "net_misc.h"

loaded_net_module *global_modules;
int *prot_table;
static net_hash *arphash;
static arp_cache_entry *arpcache;
static pool_ctl *arpp;

/* these are in seconds... */
static int arp_prune = 300; 	/* time interval we prune the arp cache? 5 minutes */
static int arp_keep  = 1200;	/* length of time we keep entries... (20 mins) */
static int arp_noflood = 1200;	/* seconds between arp flooding */

/* stats */
static int arp_inuse = 0;	/* how many entries do we have? */
static int arp_allocated = 0;	/* how many arp entries have we created? */
static int arp_maxtries = 5;	/* max tries before a pause */

/* unused presently, but useful to have around...*/
/*
static void dump_buffer(char *buffer, int len) {
	uint8 *b = (uint8 *)buffer;
	int i;
	
	printf ("  ");
	for (i=0;i<len;i+=2) {
		if (i%16 == 0)
			printf("\n  ");
		printf(" %02x", b[i]);
		printf("%02x ", b[i+1]);
	}
	printf("\n\n");
}
*/

int arp_input(struct mbuf *buf)
{
	ether_arp *arp = mtod(buf, ether_arp*);

	printf("arp request :\n");
	printf("            : hardware type : %s\n",
			ntohs(arp->arp_ht) == 1 ? "ethernet" : "unknown");
	printf("            : protocol type : %s\n",
			ntohs(arp->arp_pro) == ETHER_IPV4 ? "IPv4" : "unknown");
	printf("            : hardware size : %d\n", arp->arp_hsz);
	printf("            : protocol size : %d\n", arp->arp_psz);
	printf("            : op code       : ");
	
	switch (ntohs(arp->arp_op)) {
		case ARP_RQST:
			printf("ARP request\n            : ");
			print_ether_addr(&arp->sender);
			printf(" [");
			print_ipv4_addr(&arp->sender_ip);
			printf("] wants MAC address for ");
			print_ipv4_addr(&arp->target_ip);
			printf("\n");
			break;
		case ARP_RPLY:
			printf("ARP reply\n");
			break;
		default:
			printf("unknown (%d)\n", ntohs(arp->arp_op));
	}

	m_freem(buf);
	return 0;
}

int arp_init(loaded_net_module *ln, int *pt)
{
	global_modules = ln;
	prot_table = pt;

	if (!arphash)
		arphash = nhash_make();

	if (!arpp) {
		pool_init(&arpp, sizeof(arp_cache_entry));
		if (!arpp) {
			printf("failed to create a pool!\n");
			return -1;
		}
	}
	arpcache = NULL;

	return 0;
}

net_module net_module_data = {
	"ARP module",
	NS_ARP,
	NET_LAYER1,

	&arp_init,
	NULL,
	&arp_input, 
	NULL
};

