/* core_funcs.h
 * convenience macros to for calling core functions in the kernel
 */

#ifndef OBOS_CORE_FUNCS_H
#define OBOS_CORE_FUNCS_H

#include "net_server/core_module.h"

#ifdef _KERNEL_MODE
#	define add_protosw         core->add_protosw

#	define pool_init           core->pool_init
#	define pool_get            core->pool_get
#	define pool_put            core->pool_put

#	define m_get               core->m_get
#	define m_gethdr            core->m_gethdr
#	define m_free              core->m_free
#	define m_freem             core->m_freem
#	define m_adj               core->m_adj
#	define m_prepend           core->m_prepend
#	define m_pullup            core->m_pullup
#	define m_copydata          core->m_copydata
#	define m_copym             core->m_copym

#	define in_pcballoc         core->in_pcballoc
#	define in_pcbconnect       core->in_pcbconnect
#	define in_pcbdisconnect    core->in_pcbdisconnect
#	define in_pcbbind          core->in_pcbbind
#	define in_pcblookup        core->in_pcblookup
#	define in_pcbdetach        core->in_pcbdetach
#	define in_pcbrtentry       core->in_pcbrtentry
#	define in_localaddr        core->in_localaddr
#	define in_losing           core->in_losing
#	define in_broadcast        core->in_broadcast
#	define in_control          core->in_control
#	define in_setsockaddr      core->in_setsockaddr
#	define in_setpeeraddr      core->in_setpeeraddr

#	define ifa_ifwithdstaddr   core->ifa_ifwithdstaddr
#	define ifa_ifwithnet       core->ifa_ifwithnet

#	define sbappend            core->sbappend
#	define sbappendaddr        core->sbappendaddr
#	define sbdrop              core->sbdrop
#	define sbflush             core->sbflush
#	define sbreserve           core->sbreserve

#	define soreserve           core->soreserve
#	define sowakeup            core->sowakeup
#	define sonewconn           core->sonewconn
#	define soisconnected       core->soisconnected
#	define soisconnecting      core->soisconnecting
#	define soisdisconnected    core->soisdisconnected
#	define soisdisconnecting   core->soisdisconnecting
#	define sohasoutofband      core->sohasoutofband
#	define socantsendmore      core->socantsendmore
#	define socantrcvmore       core->socantrcvmore

#	define rtfree              core->rtfree
#	define rtalloc             core->rtalloc
#	define get_primary_addr    core->get_primary_addr
#endif	/* _KERNEL_MODE */

#endif	/* OBOS_CORE_FUNCS_H */
