#include <stdio.h>
#include <sys/param.h>
#include <sys/mbuf.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <malloc.h>

#include <net/if.h>
#include <net/if_types.h>
#include <net/route.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/in_var.h>
#include <netinet/ip.h>
#include <sys/sockio.h>

#include <net/ppp_defs.h>
#include <net/if_ppp.h>
#include "lcp.h"
#include "ipcp.h"
#include <net/if_pppvar.h>

#include "core_module.h"
#include "core_funcs.h"
#include "net_timer.h"
#include "ppp_module.h"

#ifdef _KERNEL_
#include <KernelExport.h>
#define spawn_thread spawn_kernel_thread
static int32 ppp_ops(int32 op, ...);
#else
#define ppp_ops NULL
#endif


#ifdef _KERNEL_
# ifdef VJC
# error VJC not yet implemented
# endif
#endif

#ifdef VJC
#include <net/slcompress.h>
#endif

#ifdef PPP_COMPRESS
# define PACKETPTR	struct mbuf *
# include <net/ppp-comp.h>
#endif

struct core_module_info *core = NULL;
static int ppp_devs = 0;

#define PPP_DEVNAME "ppp"

static int	pppsioctl (struct ifnet *, int, caddr_t);
//static void	ppp_requeue (struct ppp_softc *);
//static void	ppp_ccp (struct ppp_softc *, struct mbuf *m, int rcvd);
//static void	ppp_ccp_closed (struct ppp_softc *);
//static void	ppp_inproc (struct ppp_softc *, struct mbuf *);
static void	pppdumpm (struct mbuf *m0);

/*
 * We steal two bits in the mbuf m_flags, to mark high-priority packets
 * for output, and received packets following lost/corrupted packets.
 */
#define M_HIGHPRI	0x2000	/* output packet for sc_fastq */
#define M_ERRMARK	0x4000	/* steal a bit in mbuf m_flags */
#define MAX_DUMP_BYTES	128

static void pppdumpm(struct mbuf *m0)
{
    char buf[3*MAX_DUMP_BYTES+4];
    char *bp = buf;
    struct mbuf *m;
    static char digits[] = "0123456789abcdef";

    for (m = m0; m; m = m->m_next) {
		int l = m->m_len;
		u_char *rptr = (u_char *)m->m_data;

		while (l--) {
		    if (bp > buf + sizeof(buf) - 4)
				goto done;
			*bp++ = digits[*rptr >> 4]; /* convert byte to ascii hex */
			*bp++ = digits[*rptr++ & 0xf];
		}

		if (m->m_next) {
		    if (bp > buf + sizeof(buf) - 3)
			goto done;
	    	*bp++ = '|';
		} else
	    	*bp++ = ' ';
    }
done:
    if (m)
		*bp++ = '>';
    *bp = 0;
    printf("%s\n", buf);
}

static void ppp_inproc(struct ppp_softc *sc, struct mbuf *m)
{
	struct ifnet *ifp = &sc->sc_if;
	uint16 proto;
	u_char *ptr;
	int ilen;
	struct mbuf *mp = NULL;

    sc->sc_stats.ppp_ipackets++;
	
	ptr = mtod(m, u_char *);
	proto = PPP_PROTOCOL(ptr);

	if (m->m_flags & M_ERRMARK) {
		m->m_flags &= ~M_ERRMARK;
		sc->sc_flags |= SC_VJ_RESET;
	}

	/* deal with compression here!!! */
		
    ilen = 0;
    for (mp = m; mp != NULL; mp = mp->m_next)
		ilen += mp->m_len;

	printf("PPP: protocol    : %04x\n", proto);
	
	m_adj(m, 2); /* remove protocol */
	switch (proto) {
		case PPP_LCP:
			if (!sc->lcp) {
				sc->lcp = (struct lcp *)malloc(sizeof(struct lcp));
				lcp_init(sc->lcp, ifp);
			}
			lcp_input(m, sc->lcp);
			break;
		case PPP_IPCP:
			/* just shows information... */
			ipcp_input(m);
			break;
		default:
			printf("We don't handle this yet!\n");
			break;
	}
}

static int32 ppprx(void *data)
{
	struct ppp_softc *sc = (struct ppp_softc *)data;
	struct mbuf *m;
	
	while (1) {
		acquire_sem_etc(sc->sc_if.rxq->pop, 1, B_CAN_INTERRUPT | B_DO_NOT_RESCHEDULE, 0);
		IFQ_DEQUEUE(sc->sc_if.rxq, m);
		ppp_inproc(sc, m);
	}
	return 0;
}



static int pppsioctl(struct ifnet *ifp, int cmd, caddr_t data)
{
	struct ppp_softc *sc = (struct ppp_softc *)ifp;
	struct ifaddr *ifa = (struct ifaddr *)data;
	struct ifreq *ifr = (struct ifreq *)data;
	struct ppp_stats *psp;
#ifdef	PPP_COMPRESS
    struct ppp_comp_stats *pcp;
#endif
	int error = 0;

	switch (cmd) {
		case SIOCSIFFLAGS:
			if ((ifp->if_flags & IFF_RUNNING) == 0)
	    		ifp->if_flags &= ~IFF_UP;
			break;
	    case SIOCSIFADDR:
			if (ifa->ifa_addr->sa_family != AF_INET)
	    		error = EAFNOSUPPORT;
			break;
	    case SIOCSIFDSTADDR:
			if (ifa->ifa_addr->sa_family != AF_INET)
	    		error = EAFNOSUPPORT;
			break;
	    case SIOCSIFMTU:
			sc->sc_if.if_mtu = ifr->ifr_mtu;
			break;
	    case SIOCADDMULTI:
    	case SIOCDELMULTI:
			if (ifr == NULL) {
	    		error = EAFNOSUPPORT;
			    break;
			}
			switch(ifr->ifr_addr.sa_family) {
				case AF_INET:
			    	break;
				default:
			    	error = EAFNOSUPPORT;
					break;
			}
			break;
	    case SIOCGPPPSTATS:
			psp = &((struct ifpppstatsreq *) data)->stats;
			memset((void*)psp, 0, sizeof(*psp));
			psp->p = sc->sc_stats;
			break;
#ifdef PPP_COMPRESS
	    case SIOCGPPPCSTATS:
			pcp = &((struct ifpppcstatsreq *) data)->stats;
			memset((void*)pcp, 0, sizeof(*pcp));
			if (sc->sc_xc_state != NULL)
	    		(*sc->sc_xcomp->comp_stat)(sc->sc_xc_state, &pcp->c);
			if (sc->sc_rc_state != NULL)
	    		(*sc->sc_rcomp->decomp_stat)(sc->sc_rc_state, &pcp->d);
			break;
#endif /* PPP_COMPRESS */
	    default:
			error = EINVAL;
	}
	return (error);
}

int pppoutput(struct ifnet *ifp, struct mbuf *m0, struct sockaddr *dst, 
              struct rtentry *rtp)
{
	struct ppp_softc *sc = (struct ppp_softc *)ifp;
	int protocol, address, control;
	u_char *cp;
	int error;
	struct ip *ip;
	struct ifq *ifq;
	enum NPmode mode;
	int len;
	struct mbuf *m;

    if ((ifp->if_flags & IFF_RUNNING) == 0 ||
	    ((ifp->if_flags & IFF_UP) == 0 && dst->sa_family != AF_UNSPEC)) {
		error = ENETDOWN;	/* sort of */
		goto bad;
    }

    /*
     * Compute PPP header.
     */
    m0->m_flags &= ~M_HIGHPRI;
    switch (dst->sa_family) {
	    case AF_INET:
			address = PPP_ALLSTATIONS;
			control = PPP_UI;
			protocol = PPP_IP;
			mode = sc->sc_npmode[NP_IP];

			/*
			 * If this packet has the "low delay" bit set in the IP header,
			 * put it on the fastq instead.
			 */
			ip = mtod(m0, struct ip *);
			if (ip->ip_tos & IPTOS_LOWDELAY)
	    		m0->m_flags |= M_HIGHPRI;
			break;
	    case AF_UNSPEC:
			protocol = PPP_PROTOCOL(dst->sa_data);
			mode = NPMODE_PASS;
			break;
    	default:
			printf("%s: af%d not supported\n", ifp->if_name, dst->sa_family);
			error = EAFNOSUPPORT;
			goto bad;
    }

    /*
     * Drop this packet, or return an error, if necessary.
     */
    if (mode == NPMODE_ERROR) {
		error = ENETDOWN;
		goto bad;
    }
    if (mode == NPMODE_DROP) {
		error = 0;
		goto bad;
    }

    /*
     * Add PPP header.  If no space in first mbuf, allocate another.
     * (This assumes M_LEADINGSPACE is always 0 for a cluster mbuf.)
     */
	m0 = m_prepend(m0, PPP_PROTHDRLEN);
	if (m0 == NULL) {
    	error = ENOBUFS;
		printf("ppp_output: ENOBUFS\n");
    	goto bad;
	}

    cp = mtod(m0, u_char *);
    *cp++ = protocol >> 8;
    *cp++ = protocol & 0xff;

    len = 0;
    for (m = m0; m != 0; m = m->m_next)
		len += m->m_len;

    if (sc->sc_flags & SC_LOG_OUTPKT) {
		printf("%s output: ", ifp->if_name);
		pppdumpm(m0);
    }

    /*
     * Put the packet on the appropriate queue.
     */
    if (mode == NPMODE_QUEUE) {
		/* XXX we should limit the number of packets on this queue */
		*sc->sc_npqtail = m0;
		m0->m_nextpkt = NULL;
		sc->sc_npqtail = &m0->m_nextpkt;
    } else {
		if ((m0->m_flags & M_HIGHPRI)) {
			ifq = sc->sc_fastq;
	    	if (IFQ_FULL(ifq) && dst->sa_family != AF_UNSPEC) {
				m_freem(m0);
				error = ENOBUFS;
	    	}
	    	else {
				IFQ_ENQUEUE(ifq, m0);
				error = 0;
	    	}
		} else {
	    	IFQ_ENQUEUE(sc->sc_if.txq, m0);
	    	error = 0;
	    }

		if (error) {
		    sc->sc_if.if_oerrors++;
		    sc->sc_stats.ppp_oerrors++;
	    	return (error);
		}
    }
    ifp->if_opackets++;
    ifp->if_obytes += len;

    return (0);

bad:
    m_freem(m0);
    return (error);
}

struct ifnet *pppconnection(void)
{
	char tname[32];
	struct ppp_softc *sc = malloc(sizeof(struct ppp_softc));
	if (!sc)
		return NULL;
	memset(sc, 0, sizeof(*sc));
	
	sc->sc_unit = sc->sc_if.if_unit = ppp_devs++;
	sc->sc_if.name = PPP_DEVNAME;
	sc->sc_if.if_mtu = PPP_MTU;
	sc->sc_if.if_flags = (IFF_POINTOPOINT | IFF_MULTICAST | IFF_UP);
	sc->sc_if.if_type = IFT_PPP;
	sc->sc_if.if_hdrlen = PPP_HDRLEN;
	sc->sc_if.ioctl = pppsioctl;
	sc->sc_if.output = pppoutput;

	sc->sc_inq = start_ifq();
	sc->sc_fastq = start_ifq();
	sc->sc_rawq = start_ifq();
	sc->sc_if.txq = start_ifq();
	
	if_attach(&sc->sc_if);
	
	sc->sc_if.rxq = start_ifq();
	if (!sc->sc_if.rxq) {
		printf("Failed to start an IFQ for the ppp device!\n");
		return NULL;
	}
	sprintf(tname, "%s%d_rx_thread", PPP_DEVNAME, sc->sc_if.if_unit);
	sc->sc_if.rx_thread = spawn_thread(ppprx, tname,
	                                   B_NORMAL_PRIORITY, sc);
	if (sc->sc_if.rx_thread > 0) {
		resume_thread(sc->sc_if.rx_thread);
		sc->sc_if.if_flags |= IFF_RUNNING;
	}	
	return &sc->sc_if;
}


static int ppp_module_init(void *cpp)
{
	if (cpp)
		core = cpp;

	return 0;
}

#ifndef _KERNEL_
void ppp_set_core(struct core_module_info *cp)
{
	core = cp;
}
#endif

_EXPORT struct ppp_module_info protocol_info = {
	{
		{
			PPP_MODULE_PATH,
			0,
			ppp_ops
		},
		ppp_module_init,
		NULL /*ppp_module_stop*/
	},
#ifndef _KERNEL_
	ppp_set_core,
#endif
	pppconnection
};

#ifdef _KERNEL_
static status_t ppp_ops(int32 op, ...)
{
	switch(op) {
		case B_MODULE_INIT:
			get_module(CORE_MODULE_PATH, (module_info**)&core);
			if (!core)
				return B_ERROR;
			return B_OK;
		case B_MODULE_UNINIT:
			break;
		default:
			return B_ERROR;
	}
	return B_OK;
}

_EXPORT module_info *modules[] = {
	(module_info *)&protocol_info,
	NULL
};
#endif