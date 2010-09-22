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

#ifndef _ARP_H_
#define _ARP_H_
#include <global.h>
struct arp_hdr {
	uint16_t hwtype;
#define ARP_HWTYPE_ETH	1
	uint16_t protocol;
	uint8_t hwlen;
	uint8_t protolen;
	uint16_t opcode;
#define ARP_REQUEST	1
#define ARP_REPLY	2
	uint8_t srchwaddr[6];
	uint16_t srcipaddr[2];
	uint8_t dsthwaddr[6];
	uint16_t dstipaddr[2];
};

#define ARP_HDR_LENGTH	28

int	arp_resolv(uint16_t *, uint8_t *);
void	arp_up(uint8_t *, uint16_t);
int	arp_down(uint8_t *, uint16_t, uint16_t *, uint16_t);
#endif /* !_ARP_H_ */
