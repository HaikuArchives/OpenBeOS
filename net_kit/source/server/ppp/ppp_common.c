#include <kernel/OS.h>

const char *codename(int code)
{
	static const char * const codes[] = {
		"Invalid",
		"Configure-Request",
		"Configure-Ack",
		"Configure-Nak",
		"Configure-Reject",
		"Terminate-Request",
		"Terminate-Ack",
		"Code-Reject",
		"Protocol-Reject",
		"Echo-Request",
		"Echo-Reply",
		"Discard-request",
	};
	if (code < 0 || code > sizeof(codes) / sizeof(*codes))
		return "Invalid value";
	return codes[code];
}

const char *protoname(int proto)
{
	static const char * const cftypes[] = {
		/* Check out the latest ``Assigned numbers'' rfc (1700) */
		NULL,
		"MRU",			/* 1: Maximum-Receive-Unit */
		"ACCMAP",		/* 2: Async-Control-Character-Map */
		"AUTHPROTO",	/* 3: Authentication-Protocol */
		"QUALPROTO",	/* 4: Quality-Protocol */
		"MAGICNUM",		/* 5: Magic-Number */
		"RESERVED",		/* 6: RESERVED */
		"PROTOCOMP",	/* 7: Protocol-Field-Compression */
		"ACFCOMP",		/* 8: Address-and-Control-Field-Compression */
		"FCSALT",		/* 9: FCS-Alternatives */
		"SDP",			/* 10: Self-Describing-Pad */
		"NUMMODE",		/* 11: Numbered-Mode */
		"MULTIPROC",	/* 12: Multi-Link-Procedure */
		"CALLBACK",		/* 13: Callback */
		"CONTIME",		/* 14: Connect-Time */
		"COMPFRAME",	/* 15: Compound-Frames */
		"NDE",			/* 16: Nominal-Data-Encapsulation */
		"MRRU",			/* 17: Multilink-MRRU */
		"SHORTSEQ",		/* 18: Multilink-Short-Sequence-Number-Header */
		"ENDDISC",		/* 19: Multilink-Endpoint-Discriminator */
		"PROPRIETRY",	/* 20: Proprietary */
		"DCEID",		/* 21: DCE-Identifier */
		"MULTIPP",		/* 22: Multi-Link-Plus-Procedure */
		"LDBACP",		/* 23: Link Discriminator for BACP */
	};

	if (proto < 0 || proto > sizeof cftypes / sizeof *cftypes ||
	    cftypes[proto] == NULL)
		return "unknown";

	return cftypes[proto];
}

const char *state2nam(uint state)
{
	static const char * const statenames[] = {
    "Initial", 
    "Starting", 
    "Closed", 
    "Stopped", 
    "Closing", 
    "Stopping",
    "Req-Sent", 
    "Ack-Rcvd", 
    "Ack-Sent", 
    "Opened",
	};

	if (state >= sizeof statenames / sizeof statenames[0])
		return "unknown";
	return statenames[state];
}
