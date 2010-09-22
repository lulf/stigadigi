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

#include <arp.h>
#include <ethernet.h>
#include <string.h>

#include <enc28j60.h>

/* Network MAC address. */
uint8_t ethernet_addr[6] = { 0xDE, 0xAD, 0xC0, 0xDE, 0x13, 0x37 };

int
ethernet_down(uint8_t *buffer, uint16_t datalen, uint8_t *dest, uint16_t type)
{
	struct ethernet_hdr *ehdr;
	uint16_t len;

	/* Not possible to transport this much data. */
	if (datalen > MAXFRAMELEN)
		return (-1);
	
	ehdr = (struct ethernet_hdr *)buffer;

	memcpy(ehdr->src, ethernet_addr, 6);
	memcpy(ehdr->dest, dest, 6);
	ehdr->type[0] = (type & 0xFF);
	ehdr->type[1] = ((type >> 8) & 0xFF);
	datalen += ETHERNET_HDR_LEN;
	len = enc28j60_frame_transmit(buffer, datalen);
	return (len - ETHERNET_HDR_LEN);
}

uint16_t
ethernet_up(uint8_t *buffer, uint16_t len)
{
	struct ethernet_hdr *ehdr;
	uint16_t type;
	int i;

	if (len < sizeof(struct ethernet_hdr))
		return 0;
	ehdr = (struct ethernet_hdr *)buffer;
	/*
	for (i = 0; i < 6; i++) {
		if (ehdr->dest[i] != ethernet_addr[i])
			return (NULL);
	}*/
	type = (ehdr->type[1] & 0xFF);
	type |= (ehdr->type[0] << 8);
	return (type);
}
