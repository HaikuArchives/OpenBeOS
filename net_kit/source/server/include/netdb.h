/* netdb.h */

#ifndef NETDB_H
#define NETDB_H

#include <errno.h>

#define MAXHOSTNAMELEN 64

#define HOST_NOT_FOUND 1
#define TRY_AGAIN 2
#define NO_RECOVERY 3
#define NO_DATA 4

//#ifndef h_errno
extern int h_errno;
extern int *_h_errnop(void);
//#define h_errno (*_h_errnop())
//#endif /* h_errno */

struct hostent {
	char *h_name;
	char **h_aliases;
	int h_addrtype;
	int h_length;
	char **h_addr_list;
};
#define h_addr h_addr_list[0]

struct servent {
	char *s_name;
	char **s_aliases;
	int s_port;
	char *s_proto;
};	

/*
 * Assumption here is that a network number
 * fits in an in_addr_t -- probably a poor one.
 */
struct  netent {
        char            *n_name;        /* official name of net */
        char            **n_aliases;    /* alias list */
        int             n_addrtype;     /* net address type */
        in_addr_t       n_net;          /* network # */
};

struct  protoent {
        char    *p_name;        /* official protocol name */
        char    **p_aliases;    /* alias list */
        int     p_proto;        /* protocol # */
};


struct hostent *gethostbyname(const char *hostname);
struct hostent *gethostbyaddr(const char *hostname, int len, int type);
struct servent *getservbyname(const char *name, const char *proto);
void herror(const char *);
unsigned long inet_addr(const char *a_addr);
char *inet_ntoa(struct in_addr addr);

int gethostname(char *hostname, size_t hostlen);

/* BE specific, because of lack of UNIX passwd functions */
int getusername(char *username, size_t userlen);
int getpassword(char *password, size_t passlen);


/* These are new! */
struct netent   *getnetbyaddr (in_addr_t, int);
struct netent   *getnetbyname (const char *);
struct netent   *getnetent (void);
struct protoent *getprotobyname (const char *);
struct protoent *getprotobynumber (int);
struct hostent  *gethostbyname2 (const char *, int);

#define _PATH_HEQUIV    "/etc/hosts.equiv"
#define _PATH_HOSTS     "/etc/hosts"
#define _PATH_NETWORKS  "/etc/networks"
#define _PATH_PROTOCOLS "/etc/protocols"
#define _PATH_SERVICES  "/etc/services"

/*
 * Error return codes from gethostbyname() and gethostbyaddr()
 * (left in extern int h_errno).
 */

#define NETDB_INTERNAL  -1      /* see errno */
#define NETDB_SUCCESS   0       /* no problem */
#define HOST_NOT_FOUND  1 /* Authoritative Answer Host not found */
#define TRY_AGAIN       2 /* Non-Authoritive Host not found, or SERVERFAIL */
#define NO_RECOVERY     3 /* Non recoverable errors, FORMERR, REFUSED, NOTIMP */
#define NO_DATA         4 /* Valid name, no data record of requested type */
#define NO_ADDRESS      NO_DATA         /* no address, look for MX record */

#define NI_NUMERICHOST  1       /* return the host address, not the name */
#define NI_NUMERICSERV  2       /* return the service address, not the name */
#define NI_NOFQDN       4       /* return a short name if in the local domain */
#define NI_NAMEREQD     8       /* fail if either host or service name is unknow
n */
#define NI_DGRAM        16      /* look up datagram service instead of stream */
#define NI_WITHSCOPEID  32      /* KAME hack: attach scopeid to host portion */

#define NI_MAXHOST      MAXHOSTNAMELEN  /* max host name returned by getnameinfo
 */
#define NI_MAXSERV      32      /* max serv. name length returned by getnameinfo
 */

#endif /* NETDB_H */
