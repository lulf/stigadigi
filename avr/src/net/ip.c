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

#include <global.h>
#include <ethernet.h>
#include <arp.h>
#include <ip.h>
#include <misc.h>

uint16_t chksum(uint16_t *, uint16_t);

uint8_t myip[4] = {10, 0, 0, 15};
uint8_t lolip[4] = {10, 0, 0, 5};
uint16_t myip_packed[2];

void
ip_init(void)
{
	PACK_IP(myip_packed, myip[0], myip[1], myip[2], myip[3]);
}

/*
 * Fill all necessary data into the IPv4 Data structures.
 */
int
ip_down(uint8_t *buffer, uint16_t datalen, uint8_t *destip, uint8_t proto)
{
	uint16_t len;
	struct ipv4_hdr *iphdr;

	iphdr = (struct ipv4_hdr *)(buffer + ETHERNET_HDR_LEN);
	iphdr->srcipaddr[0] = myip_packed[0];
	iphdr->srcipaddr[1] = myip_packed[1];
	PACK_IP(iphdr->dstipaddr, destip[0], destip[1], destip[2], destip[3]);
	iphdr->vhl = 0x45;
	iphdr->tos = 0;
	iphdr->ttl = 64;
	iphdr->ipid[0] = 0;
	iphdr->ipid[1] = 4;
	iphdr->proto = proto;
	iphdr->ipoffset[0] = iphdr->ipoffset[1] = 0;
	iphdr->len = htons((datalen + IPV4_HDR_LEN));
	iphdr->ipchksum = 0;
	iphdr->ipchksum = ~(chksum((uint16_t *)iphdr, IPV4_HDR_LEN));
	len = arp_down(buffer, datalen + sizeof(struct ipv4_hdr),
	    iphdr->dstipaddr, htons(ETHTYPE_IPV4));
	return (len - IPV4_HDR_LEN);
}

uint8_t
ip_up(uint8_t *buffer, uint16_t len)
{
	struct ipv4_hdr *iphdr;

	iphdr = (struct ipv4_hdr *)buffer;
	if ((iphdr->dstipaddr[0] != myip_packed[0]) ||
	    (iphdr->dstipaddr[1] != myip_packed[1])) {
		blink(100);
		blink(100);
		blink(100);
		blink(100);
		blink(100);
		blink(100);
		return (0);
	}
	return (iphdr->proto);
}

int
icmp_up(uint8_t *buffer, uint16_t len)
{
	struct icmp_hdr *ihdr, *reply;
#define ICMP_OFFSET (ETHERNET_HDR_LEN + IPV4_HDR_LEN)
#define ICMP_REPLY_SIZE (ICMP_OFFSET + sizeof(struct icmp_hdr) + 50)
	uint8_t reply_buffer[ICMP_REPLY_SIZE];

	ihdr = (struct icmp_hdr *)buffer;
	reply = (struct icmp_hdr *)(reply_buffer + ICMP_OFFSET);

	/* We only support echo. */
	if (ihdr->type != TYPE_ECHO_REQUEST)
		return (-1);

	reply->type = TYPE_ECHO_REPLY;
	reply->code = 0;
	reply->chksum = 0;
	reply->id = ihdr->id;
	reply->sequence = ihdr->sequence;
	reply->chksum = ~(chksum((uint16_t *)reply, sizeof(struct icmp_hdr)));
	ip_down(reply_buffer, sizeof(struct icmp_hdr), lolip, PROTO_ICMP);
	return (0);
}

/* Checksum calculation function from DM9000 uIP */
uint16_t
chksum(uint16_t *sdata, uint16_t len)
{
	uint16_t acc;

	for (acc = 0; len > 1; len -= 2) {
		uint16_t u = ((unsigned char *)sdata)[0] + (((unsigned char *)sdata)[1] << 8);
		if ((acc += u) < u) {
			/* Overflow, so we add the carry to acc (i.e., increase by one). */
			++acc;
		}
		++sdata;
	}

	/* add up any odd byte */
	if(len == 1) {
		acc += htons(((uint16_t)(*(uint8_t *)sdata)) << 8);
		if(acc < htons(((uint16_t)(*(uint8_t *)sdata)) << 8)) {
			++acc;
		}
	}
	return (acc);
}

