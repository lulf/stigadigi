/*
 * Copyright (c) 2010, Ulf Lilleengen
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _IP_H_
#define _IP_H_
/* IPv4 Header. */
struct ipv4_hdr {
	uint8_t vhl;
	uint8_t tos;
	uint16_t len;
	uint8_t ipid[2];
	uint8_t ipoffset[2];
	uint8_t ttl;
	uint8_t proto;
#define PROTO_ICMP	1
#define PROTO_UDP	17
#define PROTO_TCP	6
	uint16_t ipchksum;
	uint16_t srcipaddr[2];
	uint16_t dstipaddr[2];
};

struct icmp_hdr {
	uint8_t type;
#define TYPE_ECHO_REPLY		0
#define TYPE_ECHO_REQUEST	8
	uint8_t code;
	uint16_t chksum;
	uint16_t id;
	uint16_t sequence;
};

#define IPV4_HDR_LEN	20

void	ip_init(void);
int	ip_down(uint8_t *, uint16_t, uint8_t *, uint8_t);
uint8_t	ip_up(uint8_t *, uint16_t);
int	icmp_down(uint8_t *, uint16_t, uint8_t);
int	icmp_up(uint8_t *, uint16_t);


#define PACK_IP(addr, addr0, addr1, addr2, addr3) do {	\
	(addr)[0] = HTONS(((addr0) << 8) | (addr1));	\
	(addr)[1] = HTONS(((addr2) << 8) | (addr3));	\
} while(0)
#endif /* !_IP_H_ */
