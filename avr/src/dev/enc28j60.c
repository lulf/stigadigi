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
#include <stdint.h>
#include <spi.h>
#include <ethernet.h>
#include <enc28j60.h>
#include "enc28j60_reg.h"
#include "enc28j60_int.h"

/* SPI command set for ENC28j60 */
#define RCR	0x00
#define RBM	0x3A
#define WCR	0x40
#define WBM	0x7A
#define BFS	0x80
#define BFC	0xA0
#define SC	0xFF

#define OP_MASK		0xE0
#define OP_OFFSET	5
#define ARG_MASK	0x1F
static void bus_write(uint8_t, uint8_t, uint8_t);
static uint8_t bus_read(uint8_t, uint8_t);
static void write_reg(uint8_t, uint8_t);
static void set_bits(uint8_t, uint8_t);
static void clear_bits(uint8_t, uint8_t);
void write_phy(uint8_t, uint16_t);
static void write_buffer(uint8_t);
static uint8_t read_buffer(void);
static void switch_bank(uint8_t);

uint16_t nextp;
uint8_t bank_reg;

/**
 * Write data to the ethernet controller over SPI
 */
static void
bus_write(uint8_t op, uint8_t arg, uint8_t data)
{
	spi_activate();
	spi_transmit(op | (arg & ARG_MASK));
	spi_transmit(data);
	spi_deactivate();
}

/**
 * Read data from the ethernet controller over SPI
 */
static uint8_t
bus_read(uint8_t op, uint8_t arg)
{
	uint8_t data;

	spi_activate();
	spi_transmit(op | (arg & ARG_MASK));
	/* Special handling for PHY and MAC registers. */
	data = spi_receive();
	if (arg & 0x80) {
		data = spi_receive();
	}
	spi_deactivate();
	return (data);
}

/**
 * Switch bank register.
 */
static void
switch_bank(uint8_t reg)
{
	/* Check if we need to switch bank. */
	if ((reg & BANK_MASK) == bank_reg)
		return;

	bus_write(BFC, ECON1, ECON1_BSEL1 | ECON1_BSEL0);
	bus_write(BFS, ECON1, (reg & BANK_MASK) >> 5);
	bank_reg = (reg & BANK_MASK);
}

/**
 * Write a control register and handle bank switching if necessary.
 */
static void
write_reg(uint8_t reg, uint8_t data)
{
	switch_bank(reg);
	bus_write(WCR, reg, data);
}

/**
 * Set a bit in a control register.
 */
static void
set_bits(uint8_t reg, uint8_t bitmask)
{
	switch_bank(reg);
	bus_write(BFS, reg, bitmask);
}

/**
 * Clear a bit in a control register.
 */
static void
clear_bits(uint8_t reg, uint8_t bitmask)
{
	switch_bank(reg);
	bus_write(BFC, reg, bitmask);
}

/**
 * Read a control register and handle bank switching if necessary.
 */
static uint8_t
read_reg(uint8_t reg)
{
	switch_bank(reg);
	return (bus_read(RCR, reg));
}

/**
 * Write to the enc28j60 network buffer.
 */
static void
write_buffer(uint8_t data)
{
	bus_write(WBM, 0, data);
}

/**
 * Read from the enc28j60 network buffer.
 */
static uint8_t
read_buffer(void)
{
	return (bus_read(RBM, 0));
}

/**
 * Write to PHY registers, which require a bit more work.
 */
void
write_phy(uint8_t reg, uint16_t data)
{
	write_reg(MIREGADR, reg);
	write_reg(MIWRL, (data & 0xFF));
	write_reg(MIWRH, ((data >> 8 ) & 0xFF));
	while (read_reg(MISTAT) & MISTAT_BUSY) {
		_delay_us(15);
	}
}

/**
 * Write a frame to the enc28j60 buffer.
 */
uint16_t
enc28j60_frame_transmit(uint8_t *data, uint16_t len)
{
	uint16_t i;

	/* Wait until ready. */
	while ((read_reg(ECON1) & ECON1_TXRTS) != 0) {
		/* Reset logic problem. */
		if ((read_reg(EIR) & EIR_TXERIF) != 0) {
			set_bits(ECON1, ECON1_TXRST);
			clear_bits(ECON1, ECON1_TXRST);
		}
	}

	write_reg(EWRPTL, (TXBUF_START & 0xFF));
	write_reg(EWRPTH, ((TXBUF_START >> 8)));
	write_reg(ETXNDL, ((TXBUF_START + len) & 0xFF));
	write_reg(ETXNDH, (((TXBUF_START + len) >> 8)));
	bus_write(WBM, 0, 0x00);

	for (i = 0; i < len; i++) {
		write_buffer(*data);
		data++;
	}
	set_bits(ECON1, ECON1_TXRTS);
	return (len);
}

uint8_t
enc28j60_has_frame(void)
{
	return (read_reg(EPKTCNT));
/*	if (read_reg(EPKTCNT) == 0) {
		return (0);
	}
	return (1);*/
}
#if 1
/**
 * Read a frame from the enc28j60 buffer.
 */
uint16_t
enc28j60_frame_receive(uint8_t *buf, uint16_t buflen)
{
	uint16_t len = 0;
	uint16_t count = 0;
	uint16_t rsv = 0;

	/* No frames in buffer. */
	if (read_reg(EPKTCNT) == 0)
		return (0);

	/* Set read pointer to where we got previous next frame. */
	write_reg(ERDPTL, (nextp & 0xFF));
	write_reg(ERDPTH, ((nextp >> 8) & 0xFF));

	/* Find pointers to next frame. */
	nextp = read_buffer();
	nextp |= (read_buffer() << 8);

	/* Read frame information. */
	len = read_buffer();
	len |= (read_buffer() << 8);
	len -= 4;
	rsv = read_buffer();
	rsv |= (read_buffer() << 8);

	/* Not enough buffer space for retrieving frame. */
	if (len > buflen)
		len = buflen;

	/* Drop frames with receive error. */
/*	if ((rsv & RSV_RECV_OK) == 0)
		goto badframe;*/
	count = len;
	while (count > 0) {
		*buf = read_buffer();
		buf++;
		count--;
	}
	/* Free up data. */
	write_reg(ERXRDPTL, (nextp & 0xFF));
	write_reg(ERXRDPTH, ((nextp >> 8) & 0xFF));
	if ((nextp - 1 < RXBUF_START) || (nextp-1 > RXBUF_END)) {
		write_reg(ERXRDPTL, (RXBUF_END) & 0xFF);
		write_reg(ERXRDPTH, (RXBUF_END) >> 8);
	} else {
		write_reg(ERXRDPTL, (nextp - 1) & 0xFF);
		write_reg(ERXRDPTH, (nextp - 1) >> 8);
	}
	set_bits(ECON2, ECON2_PKTDEC);
	return (len);
badframe:
	return (0);
}
#else
#include <avr/io.h>
#define ADDR_MASK        0x1F
#define ENC28J60_CONTROL_PORT   PORTB
#define ENC28J60_CONTROL_DDR    DDRB
#define ENC28J60_CONTROL_CS     2
#define ENC28J60_CONTROL_SO PORTB4
#define ENC28J60_CONTROL_SI PORTB3
#define ENC28J60_CONTROL_SCK PORTB5
#define ENC28J60_READ_CTRL_REG       0x00
#define ENC28J60_READ_BUF_MEM        0x3A
#define ENC28J60_WRITE_CTRL_REG      0x40
#define ENC28J60_WRITE_BUF_MEM       0x7A
#define ENC28J60_BIT_FIELD_SET       0x80
#define ENC28J60_BIT_FIELD_CLR       0xA0
#define ENC28J60_SOFT_RESET          0xFF

// set CS to 0 = active
#define CSACTIVE ENC28J60_CONTROL_PORT&=~(1<<ENC28J60_CONTROL_CS)
// set CS to 1 = passive
#define CSPASSIVE ENC28J60_CONTROL_PORT|=(1<<ENC28J60_CONTROL_CS)
//
#define waitspi() while(!(SPSR&(1<<SPIF)))

void enc28j60ReadBuffer(uint16_t len, uint8_t* data)
{
        CSACTIVE;
        // issue read command
        SPDR = ENC28J60_READ_BUF_MEM;
        waitspi();
        while(len)
        {
                len--;
                // read data
                SPDR = 0x00;
                waitspi();
                *data = SPDR;
                data++;
        }
        *data='\0';
        CSPASSIVE;
}

uint16_t enc28j60_frame_receive(uint8_t* packet, uint16_t maxlen)
{
        uint16_t rxstat;
        uint16_t len;
        // check if a packet has been received and buffered
        //if( !(enc28j60Read(EIR) & EIR_PKTIF) ){
        // The above does not work. See Rev. B4 Silicon Errata point 6.
        if( read_reg(EPKTCNT) ==0 ){
                return(0);
        }

        // Set the read pointer to the start of the received packet
        write_reg(ERDPTL, (nextp &0xFF));
        write_reg(ERDPTH, (nextp)>>8);
        // read the next packet pointer
        nextp  = bus_read(ENC28J60_READ_BUF_MEM, 0);
        nextp |= bus_read(ENC28J60_READ_BUF_MEM, 0)<<8;
        // read the packet length (see datasheet page 43)
        len  = bus_read(ENC28J60_READ_BUF_MEM, 0);
        len |= bus_read(ENC28J60_READ_BUF_MEM, 0)<<8;
        len-=4; //remove the CRC count
        // read the receive status (see datasheet page 43)
        rxstat  = bus_read(ENC28J60_READ_BUF_MEM, 0);
        rxstat |= ((uint16_t)bus_read(ENC28J60_READ_BUF_MEM, 0))<<8;
        // limit retrieve length
        if (len>maxlen-1){
                len=maxlen-1;
        }
        // check CRC and symbol errors (see datasheet page 44, table 7-3):
        // The ERXFCON.CRCEN is set by default. Normally we should not
        // need to check this.
        if ((rxstat & 0x80)==0){
                // invalid
                len=0;
        }else{
                // copy the packet from the receive buffer
                enc28j60ReadBuffer(len, packet);
        }
        // Move the RX read pointer to the start of the next received packet
        // This frees the memory we just read out
        write_reg(ERXRDPTL, (nextp &0xFF));
        write_reg(ERXRDPTH, (nextp)>>8);
        // Move the RX read pointer to the start of the next received packet
        // This frees the memory we just read out.
        // However, compensate for the errata point 13, rev B4: enver write an even address!
        if ((nextp - 1 < RXBUF_START)
                || (nextp -1 > RXBUF_END)) {
                write_reg(ERXRDPTL, (RXBUF_END)&0xFF);
                write_reg(ERXRDPTH, (RXBUF_END)>>8);
        } else {
                write_reg(ERXRDPTL, (nextp-1)&0xFF);
                write_reg(ERXRDPTH, (nextp-1)>>8);
        }
        // decrement the packet counter indicate we are done with this packet
        set_bits(ECON2, ECON2_PKTDEC);
        return(len);
}
#endif

/**
 * Initialize the enc28j60 device.
 */
void
enc28j60_init()
{
	nextp = RXBUF_START;

	/* Reset device. */
	bus_write(SC, 0, 0xFF);
	_delay_loop_1(205);

	switch_bank(0);
	/* Initialize memory buffers. */
	write_reg(ERXSTL, (RXBUF_START & 0xFF));
	write_reg(ERXSTH, ((RXBUF_START >> 8) & 0xFF));
	write_reg(ERXNDL, (RXBUF_END & 0xFF));
	write_reg(ERXNDH, ((RXBUF_END >> 8) & 0xFF));
	write_reg(ERXRDPTL, (RXBUF_START & 0xFF));
	write_reg(ERXRDPTH, ((RXBUF_START >> 8) & 0xFF));

	write_reg(ETXSTL, (TXBUF_START & 0xFF));
	write_reg(ETXSTH, ((TXBUF_START >> 8) & 0xFF));
	write_reg(ETXNDL, (TXBUF_END & 0xFF));
	write_reg(ETXNDH, ((TXBUF_END >> 8) & 0xFF));

	write_reg(ERXFCON, 0); // Disable packet filter

	/* Initialize MAC. */
	write_reg(MACON1, MACON1_TXPAUS | MACON1_RXPAUS | MACON1_MARXEN);
	write_reg(MACON2, 0x00);
	set_bits(MACON3, MACON3_PADCFG0 | MACON3_TXCRCEN | MACON3_FRMLNEN);

	write_reg(MAIPGL, 0x12);
	write_reg(MAIPGH, 0x0C);
	write_reg(MABBIPG, 0x12);

	write_reg(MAMXFLL, (FRAMELENGTH & 0xFF));
	write_reg(MAMXFLH, ((FRAMELENGTH >> 8) & 0xFF));

	write_reg(MAADR0, ethernet_addr[5]);
	write_reg(MAADR1, ethernet_addr[4]);
	write_reg(MAADR2, ethernet_addr[3]);
	write_reg(MAADR3, ethernet_addr[2]);
	write_reg(MAADR4, ethernet_addr[1]);
	write_reg(MAADR5, ethernet_addr[0]);

	/* Initialize PHY. */
	write_phy(PHCON2, PHCON2_HDLDIS);

	/* Enable interrupts */
//	write_reg(EIE, (1 << INTIE) | (1 << PKTIE));
//	write_reg(ECON1, (1 << RXEN));
	switch_bank(ECON1);
	set_bits(EIE, EIE_INTIE | EIE_PKTIE);
	set_bits(ECON1, ECON1_RXEN);
	/* Set correct clock. */
	write_reg(ECOCON, 2 & 0x7);
	
}

uint8_t test() { return read_reg(ECON1); }
