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

#define F_CPU 12500000UL
#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include <misc.h>
#include <spi.h>
//#include "enc28j60_orig.h"
#include <enc28j60.h>
#include <ethernet.h>
#include <arp.h>
#include <ip.h>

#define BUFFERSIZE	500
extern uint8_t myip[4];
uint8_t buffer[BUFFERSIZE];

int
main(void)
{
	CLKPR = (1 << CLKPCE);
	CLKPR = 0;
	_delay_loop_1(50);
	DDRD &= ~(1 << DDD2);

	spi_init();
	enc28j60_init();
	_delay_loop_1(50);
	ip_init();

	DDRB |= (1 << DDB1);
	//PORTB &= ~(1 << PORTB1);
	PORTB |= (1 << PORTB1);

	/* Shut down transistor on relay. */
	DDRD |= (1 << DDD7);
        PORTD &= ~(1 << PORTD7);

#define PHLCON		0x14
	write_phy(PHLCON, 0x476);
        _delay_loop_1(50); // 12ms
        

	uint8_t mc1 = 0xFF; //test();
	uint8_t dip[4];
	int ret;
	dip[0] = 10;
	dip[1] = 0;
	dip[2] = 0;
	dip[3] = 5;
	myip[0] = 10;
	myip[1] = 0;
	myip[2] = 0;
	myip[3] = 15;
	uint16_t destip[2];
	PACK_IP(destip, dip[0], dip[1], dip[2], dip[3]);
	//uint8_t dest[6] = {0x00,0x16,0xd3,0x38,0xb7,0xc8};
	uint8_t dest[6] = {0xc8,0xb7,0x38,0xd3,0x16,0x00};
//	ethernet_down(dest, ETHTYPE_IPV4, 90, ETH_NORMAL);//ip_down(dip, PROTO_UDP, 4);
	uint8_t i;
	i = 0;
	uint8_t d = 0;
	PORTB &= ~(1 << PORTB1);
	_delay_ms(1000);
	PORTB |= (1 << PORTB1);
	uint16_t len;
	uint16_t type;
	uint16_t proto;
	uint8_t *hdr;
	uint8_t hwbuf[6];
	//_delay_ms((delay));
	arp_resolv(destip, hwbuf);
	while (1) {

		while (enc28j60_has_frame() == 0);
		len = enc28j60_frame_receive(buffer, BUFFERSIZE);
		type = ethernet_up(buffer, len);
		switch (type) {
		case ETHTYPE_ARP:
			arp_up(buffer + ETHERNET_HDR_LEN, len -
			    ETHERNET_HDR_LEN);
			break;
		case ETHTYPE_IPV4:
			proto = ip_up(buffer + ETHERNET_HDR_LEN, len -
			    ETHERNET_HDR_LEN);
			switch (proto) {
			case PROTO_ICMP:
				ret = icmp_up(buffer + ETHERNET_HDR_LEN +
				    IPV4_HDR_LEN, len - ETHERNET_HDR_LEN -
				    IPV4_HDR_LEN);
				// Send reply
				/*
				if (ret == 0) {
					ip_down(reply_buffer,
					    sizeof(struct icmp_hdr),
					    lolip, PROTO_ICMP);
				}*/
			}
			break;
		}
	}
#if 0

/*	for (k = 0; k < 8; k++) {*/

		uint8_t v = (mc1 >> i) & 0x01;
		i++;
		if (v == 1) {
			uint8_t hwbuf[6];
			if (d == 0 && arp_resolv(destip, hwbuf) != 0) {
				uint8_t ptr;
				while (enc28j60_has_frame() == 0) {
//				while (enc28j60hasRxPkt() == 0) {

				_delay_ms(100);
				PORTB &= ~0x02;
				_delay_ms(100);
				PORTB |= 0x02;
				}
				len = enc28j60_frame_receive(buffer,
				    BUFFERSIZE);
				if (len == 0) {
					while(1);
				}
				ptr = ethernet_up(buffer, len);
				mc1 = ptr;
				d = 1;
				i = 0;
			} else {
				_delay_ms(100);
				PORTB &= ~0x02;
				_delay_ms(100);
				PORTB |= 0x02;
			}
//			ip_down(dip, PROTO_UDP, 4);
	//		ethernet_down(dest, ETHTYPE_IPV4, 90, ETH_NORMAL);//ip_down(dip, PROTO_UDP, 4);
		} else {
			_delay_ms(200);
//			PORTB |= 0x02;
		}
		/*
		_delay_ms(1000);
		if ((i % 2) == 0)
			PORTB |= (1 << PORTB1);
		else
			PORTB &= ~(1 << PORTB1);*/
		_delay_ms(500);
#endif

	return (0);
}
