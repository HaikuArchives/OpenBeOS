/* udp_var.h */

#ifndef UDP_VAR_H
#define UDP_VAR_H

struct  udpstat {
                                /* input statistics: */
        uint32  udps_ipackets;          /* total input packets */
        uint32  udps_hdrops;            /* packet shorter than header */
        uint32  udps_badsum;            /* checksum error */
        uint32  udps_nosum;             /* no checksum */
        uint32  udps_badlen;            /* data length larger than packet */
        uint32  udps_noport;            /* no socket on port */
        uint32  udps_noportbcast;       /* of above, arrived as broadcast */
        uint32  udps_nosec;             /* dropped for lack of ipsec */
        uint32  udps_fullsock;          /* not delivered, input socket full */
        uint32  udps_pcbhashmiss;       /* input packets missing pcb hash */
        uint32  udps_inhwcsum;          /* input hardware-csummed packets */
                                /* output statistics: */
        uint32  udps_opackets;          /* total output packets */
        uint32  udps_outhwcsum;         /* output hardware-csummed packets */
};

#endif
