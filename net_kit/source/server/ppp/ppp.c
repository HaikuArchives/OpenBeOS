#include <kernel/OS.h>
#include <sys/mbuf.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>

#include <net/if.h>
#include <sys/sockio.h>

#include "net/ppp_stats.h"
#include "net/if_ppp.h"
#include "ppp_device.h"
#include "fsm.h"
#include "core_module.h"
#include "ppp_module.h"

#include "net/ppp_ctrl.h"
#include "ppp_defs.h"
#include "ppp_device_module.h"


#include "core_funcs.h"
#include "net_timer.h"

#ifdef _KERNEL_
#include <KernelExport.h>
#define spawn_thread spawn_kernel_thread
static int32 ppp_ops(int32 op, ...);
#else
#define ppp_ops NULL
#endif


struct core_module_info *core = NULL;
static int ppp_devs = 0;

static int	pppsioctl (struct ifnet *, int, caddr_t);
static void	pppdumpm (struct mbuf *m0);
static thread_id pppctrl = -1;
static struct ppp_dev_match *patterns = NULL;
static struct ppp_proto_match *proto_matches = NULL;

struct ppp_device {
	struct ppp_device *next;
	struct ppp_device_module_info *minfo;
	image_id mid;
} *ppp_devices = NULL;

struct ppp_modules {
	struct ppp_modules *next;
	struct ppp_module_info *minfo;
	image_id mid;
} *ppp_modules = NULL;

#define MAX_DUMP_BYTES	128

struct ifnet *ppp_create(struct ifnet *io, struct ppp_device_module_info *mod);
static int ppp_get_protocols(void);
static int ppp_get_devices(void);

static int32 ppp_conn_control(void *data)
{
	struct ppp_softc *ppp = (struct ppp_softc*)data;
	char *pdata;
	int32 code;
	size_t bufsize;
	int rv;
	thread_id me = find_thread(NULL), sender;

	pdata = malloc(PPPCTRL_MAXSIZE);
	if (!pdata) {
		printf("Failed to allocate data for recv buffer for ppp control thread!\n");
		return -1;
	}
	while (1) {
		/* setup buffer */
		memset(pdata, 0, PPPCTRL_MAXSIZE);
		bufsize = PPPCTRL_MAXSIZE;
		code = receive_data(&sender, pdata, bufsize);

		if (code < PPPCTRL_DELETE)
			continue;

		if (me == sender) {
			printf("ppp_control: looped msg received!??? How???\n");
			continue;
		}
		
		switch (code) {
			case PPPCTRL_UP:
				/* Do any config and change state to Up */
				/* ??? Hmmm, do we want to do this??? */
				break;
			case PPPCTRL_OPEN:
				/* Start any threads reqd and send an Open message. */
				rv = ppp->child->start(ppp->if_child);
				break;
			case PPPCTRL_DELETE:
				rv = ppp->child->destroy_device(ppp->if_child);
				send_data(sender, rv, NULL, 0);
				break;
		}
	}
	return 0;
}
	
static int32 ppp_control(void *data)
{
	char *pdata;
	int32 code;
	size_t bufsize;
	int rv;
	thread_id me = find_thread(NULL), sender;
	struct ifnet *child = NULL, *parent = NULL;
	
	pdata = malloc(PPPCTRL_MAXSIZE);
	if (!pdata) {
		printf("Failed to allocate data for recv buffer for ppp control thread!\n");
		return -1;
	}
	while (1) {
		/* setup buffer */
		memset(pdata, 0, PPPCTRL_MAXSIZE);
		bufsize = PPPCTRL_MAXSIZE;
		code = receive_data(&sender, pdata, bufsize);
		if (code < PPPCTRL_CREATE)
			continue;
		if (me == sender) {
			printf("ppp_control: looped msg received!??? How???\n");
			continue;
		}
		
		switch (code) {
			case PPPCTRL_CREATE: {
				struct ppp_dev_match *p = patterns;
				printf("PPPCTRL_CREATE: %s\n", pdata);
				for(; p ; p = p->next) {
					if (strlen(pdata) >= p->mlen &&
					    strncmp(pdata, p->match, p->mlen) == 0) {
						thread_id newpppctl;
						char tname[B_OS_NAME_LENGTH];
						printf("Asking %s to create sub device...\n", p->mod->name);
						child = p->mod->create_device(pdata, strlen(pdata));
						if (child)
							parent = ppp_create(child, p->mod);
						if (parent) {
							char unit[6];
							p->mod->attach(child, (struct ppp_softc*)parent);
							sprintf(tname, "%s_control_thread", parent->if_name);
							newpppctl = spawn_thread(ppp_conn_control, tname, B_NORMAL_PRIORITY, parent);
							if (newpppctl > 0) {
								resume_thread(newpppctl);
								/* XXX - not sure about this */
								p->mod->start(child);			
								sprintf(unit, "%s", parent->if_name);
								send_data(sender, PPPCTRL_OK, &unit, strlen(unit));
							} else {
								send_data(sender, PPPCTRL_FAIL, NULL, 0);
							}
						}
						break;
					}
				}
				/* XXX - lame, find a better way */
				send_data(sender, PPPCTRL_FAIL, "Unable to find a matching device to use", 38);
				break;
			}			
			default:
				printf("Dunno. Just say OK :)\n");
				rv = send_data(sender, PPPCTRL_OK, NULL, 0);
				if (rv < 0)
					printf("write_port failed... %d [%s]\n", rv, strerror(rv));
		}
		
		printf("I have read and written data from/to the port - code was %ld\n", code);
	}
	
	return 0;
}
			
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

static int32 ppprx(void *data)
{
	struct ppp_softc *sc = (struct ppp_softc *)data;
	struct mbuf *m;
	uint16 proto;
	struct fsm *fsm;
	
	while (1) {
		acquire_sem_etc(sc->sc_if.rxq->pop, 1, B_CAN_INTERRUPT | B_DO_NOT_RESCHEDULE, 0);
		IFQ_DEQUEUE(sc->sc_if.rxq, m);

	    sc->sc_stats.ppp_ipackets++;
	    
	    /* determine the protocol, then move the buffer pointer
	     * past it
	     */
		if (mtod(m, void*) && m->m_len > 8) {
			proto = PPP_PROTOCOL(mtod(m, uchar *));
			m_adj(m, 2);
			for (fsm = sc->fsm_list; fsm ; fsm = fsm->fsm_next) {
				if (fsm->proto == proto) {
					fsm_Input(fsm, m);
					break;
				}
			}
			if (!fsm) {
				sc->sc_stats.ppp_ierrors++;
				printf("Unknown protocol encountered! %04x\n", proto);
			}
		} else {
			m_freem(m);
		}
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
#endif
	    default:
			error = EINVAL;
	}
	return (error);
}


int pppoutput(struct ifnet *ifp, struct mbuf *m0, struct sockaddr *dst, 
              struct rtentry *rtp)
{
	struct ppp_softc *sc = (struct ppp_softc *)ifp;
	int protocol;
	u_char *cp;
	int error;
	struct ip *ip;
	int len;
	struct mbuf *m;

    if ((ifp->if_flags & IFF_RUNNING) == 0 ||
	    ((ifp->if_flags & IFF_UP) == 0 && dst->sa_family != AF_UNSPEC)) {
		error = ENETDOWN;
		goto bad;
    }

    switch (dst->sa_family) {
	    case AF_INET:
			protocol = PPP_IP;
			ip = mtod(m0, struct ip *);
			break;
	    case AF_UNSPEC:
			protocol = PPP_PROTOCOL(dst->sa_data);
			break;
    	default:
			printf("%s: af%d not supported\n", ifp->if_name, dst->sa_family);
			error = EAFNOSUPPORT;
			goto bad;
    }

	m0 = m_prepend(m0, 2);
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

	if (sc->sc_flags & PPP_LOG_OUTPKT) {
		printf("%s output: ", ifp->if_name);
		pppdumpm(m0);
	}
   	IFQ_ENQUEUE(sc->if_child->txq, m0);
   	error = 0;

    ifp->if_opackets++;
    ifp->if_obytes += len;

    return (0);

bad:
    m_freem(m0);
    return (error);
}

static void ppp_add_proto(char *ptag, struct ppp_softc *sc)
{
	struct ppp_proto_match *p = proto_matches;
	int taglen = strlen(ptag);

	for (; p ; p = p->next) {
		if (strncmp(p->match, ptag, taglen) == 0) {
			/* we have a valid module to add! */
			struct fsm *fsm = (struct fsm*)malloc(sizeof(struct fsm));
			memset(fsm, 0, sizeof(*fsm));
			
			/* do initial conf */
			printf("setting fsm %p to have sc %p\n", fsm, sc);
			fsm->sc = sc;
			fsm_Init(fsm);
			
			if (sc->fsm_list) {
				/* find the end of the list and add ourselves there */
				struct fsm *f = sc->fsm_list;
				for (; f->fsm_next ; f = f->fsm_next)
					continue;
				f->fsm_next = fsm;
			} else
				sc->fsm_list = fsm;

			/* we now have an empty fsm structure, time to fill it in with the
			 * protocol specifics we need :)
			 */
			p->mod->init_fsm(fsm);
			
			sc->layers |= fsm->layer;
			
			break;
		}
	}
}

struct ifnet *ppp_create(struct ifnet *io, struct ppp_device_module_info *mod)
{
	char tname[32];
	struct ppp_softc *sc = malloc(sizeof(struct ppp_softc));
	if (!sc)
		return NULL;
	memset(sc, 0, sizeof(*sc));
	
	sc->sc_if.if_unit = ppp_devs++;
	sc->sc_if.name = PPP_DEVICENAME;
	sc->sc_if.if_mtu = PPP_MTU;
	sc->sc_if.if_flags = (IFF_POINTOPOINT | IFF_MULTICAST | IFF_UP);
	sc->sc_if.if_type = IFT_PPP;
	sc->sc_if.if_hdrlen = PPP_HDRLEN;
	sc->sc_if.ioctl = pppsioctl;
	sc->sc_if.output = pppoutput;
	sc->sc_if.txq = start_ifq();
	
	sc->child = mod;
	sc->if_child = io;
	
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
	
	/* Add the LCP FSM */ 
	ppp_add_proto("lcp", sc);
	/* the LCP FSM deals with the physical layer, so attach the new
	 * FSM to the physical layer...
	 */
	mod->attach_fsm(io, sc->fsm_list);
	
	ppp_add_proto("ipcp", sc);
		
	return &sc->sc_if;
}

static int ppp_module_init(void *cpp)
{
	if (cpp)
		core = cpp;

	pppctrl = spawn_thread(ppp_control, PPPCTRL_THREADNAME, 
	                       B_NORMAL_PRIORITY, NULL);
	if (pppctrl > 0)
		resume_thread(pppctrl);

	printf("PPP: loaded %d protocols that can be used\n", ppp_get_protocols());
	/* now load a list of our (PPP) devices */
	printf("PPP: loaded %d devices that can be used\n", ppp_get_devices());
	
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
	NULL,
#endif
	NULL,
	NULL,
	fsm_Up,
	fsm_Open,
	fsm_Down,
	fsm_Close,
	fsm_Output
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

static int ppp_get_devices(void)
{
	int ndevs = 0;
	struct ppp_device *pppd = NULL;
	char dirpath[PATH_MAX], path[PATH_MAX];
	DIR *dir;
	struct dirent *fe;
	status_t status;
		
	getcwd(path, PATH_MAX);
	sprintf(dirpath, "%s/modules/ppp/devices", path);
	dir = opendir(dirpath);
	
	while ((fe = readdir(dir)) != NULL) {
		/* last 2 entries are only valid for development... */
		if (strcmp(fe->d_name, ".") == 0 || strcmp(fe->d_name, "..") == 0)
                        continue;
		sprintf(path, "%s/%s", dirpath, fe->d_name);

		pppd = (struct ppp_device*)malloc(sizeof(struct ppp_device));
		if (!pppd)
			return ndevs;
		pppd->mid = load_add_on(path);
		if (pppd->mid > 0) {		
			status = get_image_symbol(pppd->mid, "ppp_device",
						B_SYMBOL_TYPE_DATA, (void**)&pppd->minfo);
			if (status == B_OK) {
				pppd->minfo->set_core(core);
				pppd->minfo->set_ppp(&protocol_info);
				pppd->minfo->add_matches(&patterns, pppd->minfo);
				pppd->next = ppp_devices;
				ppp_devices = pppd;
				
				ndevs++;
			} else {
				free(pppd);
				pppd = NULL;
			}
		}
	}
	return ndevs;
}

static int ppp_get_protocols(void)
{
	int nmods = 0;
	struct ppp_modules *pppd = NULL;
	char dirpath[PATH_MAX], path[PATH_MAX];
	DIR *dir;
	struct dirent *fe;
	status_t status;
		
	getcwd(path, PATH_MAX);
	sprintf(dirpath, "%s/modules/ppp/protocols", path);
	dir = opendir(dirpath);
	
	while ((fe = readdir(dir)) != NULL) {
		/* last 2 entries are only valid for development... */
		if (strcmp(fe->d_name, ".") == 0 || strcmp(fe->d_name, "..") == 0)
                        continue;
		sprintf(path, "%s/%s", dirpath, fe->d_name);

		pppd = (struct ppp_modules*)malloc(sizeof(struct ppp_modules));
		if (!pppd)
			return nmods;
		pppd->mid = load_add_on(path);
		if (pppd->mid > 0) {		
			status = get_image_symbol(pppd->mid, "protocol_info",
						B_SYMBOL_TYPE_DATA, (void**)&pppd->minfo);
			if (status == B_OK) {
				pppd->minfo->set_core(core);
				pppd->minfo->set_ppp(&protocol_info);
				pppd->minfo->add_matches(&proto_matches, pppd->minfo);
				pppd->next = ppp_modules;
				ppp_modules = pppd;
				
				nmods++;
			} else {
				free(pppd);
				pppd = NULL;
			}
		}
	}
	return nmods;
}
