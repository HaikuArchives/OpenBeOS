#ifdef USER
#include <stdio.h>
#endif

#include <sys/param.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <sys/socket.h>

#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include "core_module.h"
#include "core_funcs.h"
#include "net/ppp_defs.h"
#include "net/if_ppp.h"
#include "lcp.h"
#include "ipcp.h"

extern struct core_module_info *core;

static const char *protoname(int proto)
{
	static const char * const cftypes[] = {
		NULL,
		"IP Addresses",
		"IP Compression",
		"IP Address",
	};

	if (proto < 0 || proto > sizeof cftypes / sizeof *cftypes ||
	    cftypes[proto] == NULL)
		return "unknown";

	return cftypes[proto];
}

static char *ip_ntoa(uint32 ipaddr)
{
    static char b[64];

    ipaddr = ntohl(ipaddr);

    sprintf(b, "%d.%d.%d.%d",
	    (u_char)(ipaddr >> 24),
	    (u_char)(ipaddr >> 16),
	    (u_char)(ipaddr >> 8),
	    (u_char)(ipaddr));
    return b;
}

/* Just to show what's going on... */
void ipcp_input(struct mbuf *m)
{
	/* we ignore ConfigReq messages when in the following states... */
	uint8 *cp = mtod(m, uint8 *), *wp;
	struct lcpheader *lcph = mtod(m, struct lcpheader*);
	int len = ntohs(lcph->length) - sizeof(struct lcpheader);

	if (lcph->code > IPCP_MAX_CODE) {
		printf("** Invalid code for IPCP!\n");
		/* should send a CodeReject */
		m_freem(m);
		return;
	}

	/* skip the header */
	cp += sizeof(struct lcpheader);
	wp = cp;
	while (len > 0) {
		struct ipcp_opt *io = (struct ipcp_opt*)cp;
		printf("IPCP_OPT: id     %d [%s]\n", io->type, protoname(io->type));
	
		switch (io->type) {
			case IPCP_ADDRESSES:
				printf("\tIP_ADDRESSES has been deprecated! ???\n");
			case IPCP_ADDRESS:
				printf("\tAddress: %s\n", ip_ntoa(*((uint32*)&io->ipcp_data[0])));
				break;
			case IPCP_COMPPROT: {
				uint16 comp = ntohs(io->comp_prot);
				printf("\tProtocol requested was %04x\n", comp);
				printf("\tMax  slot Id : %02x\n", io->comp_data[0]);
				printf("\tComp slot Id : %02x\n", io->comp_data[1]);
				break;
			}
			default:
				//printf("Unknown type for IPCP\n");
		}
		cp += io->len;
		len -= io->len;
	}
}
