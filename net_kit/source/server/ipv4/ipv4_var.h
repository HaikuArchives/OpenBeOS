/* ipv4_var.h
 */

/* Parts of this file are covered under the following copyright */
/*
 * Copyright (c) 1982, 1986, 1993
 *      The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by the University of
 *      California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *      @(#)ip_var.h    8.1 (Berkeley) 6/10/93
 */

#ifndef OBOS_IPV4_VAR_H
#define OBOS_IPV4_VAR_H

struct  ipstat {
        int32  ips_total;              /* total packets received */
        int32  ips_badsum;             /* checksum bad */
        int32  ips_tooshort;           /* packet too short */
        int32  ips_toosmall;           /* not enough data */
        int32  ips_badhlen;            /* ip header length < data size */
        int32  ips_badlen;             /* ip length < ip header length */
        int32  ips_fragments;          /* fragments received */
        int32  ips_fragdropped;        /* frags dropped (dups, out of space) */
        int32  ips_fragtimeout;        /* fragments timed out */
        int32  ips_forward;            /* packets forwarded */
        int32  ips_cantforward;        /* packets rcvd for unreachable dest */
        int32  ips_redirectsent;       /* packets forwarded on same net */
        int32  ips_noproto;            /* unknown or unsupported protocol */
        int32  ips_delivered;          /* datagrams delivered to upper level*/
        int32  ips_localout;           /* total ip packets generated here */
        int32  ips_odropped;           /* lost packets due to nobufs, etc. */
        int32  ips_reassembled;        /* total packets reassembled ok */
        int32  ips_fragmented;         /* datagrams sucessfully fragmented */
        int32  ips_ofragments;         /* output fragments created */
        int32  ips_cantfrag;           /* don't fragment flag was set, etc. */
        int32  ips_badoptions;         /* error in option processing */
        int32  ips_noroute;            /* packets discarded due to no route */
        int32  ips_badvers;            /* ip version != 4 */
        int32  ips_rawout;             /* total raw ip packets generated */
        int32  ips_badfrags;           /* malformed fragments (bad length) */
        int32  ips_rcvmemdrop;         /* frags dropped for lack of memory */
        int32  ips_toolong;            /* ip length > max ip packet size */
        int32  ips_nogif;              /* no match gif found */
        int32  ips_badaddr;            /* invalid address on header */
        int32  ips_inhwcsum;           /* hardware checksummed on input */
        int32  ips_outhwcsum;          /* hardware checksummed on output */
};

#endif /* OBOS_IPV4_VAR_H */
