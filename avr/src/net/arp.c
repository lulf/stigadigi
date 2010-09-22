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

#include <avr/interrupt.h>
#include <avr/sleep.h>

#include <ethernet.h>
#include <arp.h>
#include <ip.h>
#include <misc.h>

#define READY	0	/* Ready to process new request. */
#define WAIT	1	/* Waiting for an arp resolv.    */
int arp_state;
void
arp_init(void)
{
	arp_state = READY;
}

/* Maintain a local ARP cache of two entries. */
struct arp_cache_entry {
	uint16_t ipaddr[2];
	uint8_t hwaddr[6];
//	uint8_t timestamp;
};

#define ARP_CACHE_SIZE	4
struct arp_cache_entry arp_cache[ARP_CACHE_SIZE];
uint8_t oldest_entry = 0;

extern uint8_t ethernet_addr[6];
//extern uint8_t myip[4];

/**
 * Resolv ipaddr and put hardware address into hwbuf if found.
 */
int
arp_resolv(uint16_t *ipaddr, uint8_t *hwbuf)
{
	struct arp_cache_entry *arp_ce;
	int i, j;
	uint8_t mip[4];

	mip[0] = 10;
	mip[1] = 0;
	mip[2] = 0;
	mip[3] = 15;
	for (i = 0; i < ARP_CACHE_SIZE; i++) {
		arp_ce = &arp_cache[i];
		if (ipaddr[0] == arp_ce->ipaddr[0] &&
		    ipaddr[1] == arp_ce->ipaddr[1]) {
			for (j = 0; j < 6; j++)
				hwbuf[j] = arp_ce->hwaddr[j];
			return (0);
		}
	}
	/*
	 * If we came here, it could not be resolved. Send an ARP request and
	 * signal to caller that it should try again later.
	 */
#define ARP_REQ_SIZE (ETHERNET_HDR_LEN + sizeof(struct arp_hdr) + 50)
	uint8_t buffer[ARP_REQ_SIZE];
	struct arp_hdr *ahdr = (struct arp_hdr *)(buffer + ETHERNET_HDR_LEN);
	ahdr->hwtype = htons(ARP_HWTYPE_ETH);
	ahdr->protocol = htons(ETHTYPE_IPV4);
	ahdr->hwlen = 6;
	ahdr->protolen = 4;
	ahdr->opcode = htons(ARP_REQUEST);
	// XXX
	for (i = 0; i < 6; i++) {
		ahdr->srchwaddr[i] = ethernet_addr[i];
		ahdr->dsthwaddr[i] = 0xFF;
	}

	PACK_IP(ahdr->srcipaddr, mip[0], mip[1], mip[2], mip[3]);
	ahdr->dstipaddr[0] = ipaddr[0];
	ahdr->dstipaddr[1] = ipaddr[1];

	ethernet_down(buffer, sizeof(struct arp_hdr), ahdr->dsthwaddr,
	    htons(ETHTYPE_ARP));
	return (1);
}

int
arp_down(uint8_t *buffer, uint16_t datalen, uint16_t *ipaddr, uint16_t type)
{
	uint8_t destaddr[6];
	uint16_t len;

	if (arp_resolv(ipaddr, destaddr) != 0) {
		return 0;
	}
	arp_state = READY;
	len = ethernet_down(buffer, datalen, destaddr, type);
	return (len);
}

void
arp_up(uint8_t *buf, uint16_t len)
{
	struct arp_hdr *ahdr = (struct arp_hdr *)buf;
	struct arp_cache_entry *arp_ce;
	int i;
	#define ARP_REPLY_SIZE	(ETHERNET_HDR_LEN + sizeof(struct arp_hdr) + 50)
	uint8_t mip[4];

	mip[0] = 10;
	mip[1] = 0;
	mip[2] = 0;
	mip[3] = 15;

	uint8_t reply_buffer[ARP_REPLY_SIZE];
	struct arp_hdr *reply = (struct arp_hdr *)(reply_buffer +
	    ETHERNET_HDR_LEN);

	switch (htons(ahdr->opcode)) {
	/* Provide a reply for a request. */
	case ARP_REQUEST:
		reply->hwtype = htons(ARP_HWTYPE_ETH);
		reply->protocol = htons(ETHTYPE_IPV4);
		reply->hwlen = 6;
		reply->protolen = 4;
		reply->opcode = htons(ARP_REPLY);
		for (i = 0; i < 6; i++) {
			reply->srchwaddr[i] = ethernet_addr[i];
			reply->dsthwaddr[i] = ahdr->srchwaddr[i];
		}

		PACK_IP(reply->srcipaddr, mip[0], mip[1], mip[2], mip[3]);
		reply->dstipaddr[0] = ahdr->srcipaddr[0];
		reply->dstipaddr[1] = ahdr->srcipaddr[1];

		// XXX: Should we update cache here? Probably not in a strict
		// protocol
		ethernet_down(reply_buffer, sizeof(struct arp_hdr),
		    reply->dsthwaddr, htons(ETHTYPE_ARP));
		break;
	case ARP_REPLY:
		arp_ce = &arp_cache[oldest_entry];
		for (i = 0; i < 2; i++)
			arp_ce->ipaddr[i] = ahdr->srcipaddr[i];
		for (i = 0; i < 6; i++)
			arp_ce->hwaddr[i] = ahdr->srchwaddr[i];
		oldest_entry = ((oldest_entry + 1) % ARP_CACHE_SIZE);
		break;
	default:
		blink16(htons(ahdr->opcode));
		break;
	}
}
