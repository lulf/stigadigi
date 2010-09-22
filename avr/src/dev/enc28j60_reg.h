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

#ifndef _ENC28J60_REG_H_
#define _ENC28J60_REG_H_
/* ENC28j60 control registers. */

/* All banks. */
#define EIE		0x1B
#define EIE_INTIE	0x80
#define EIE_PKTIE	0x40
#define EIE_TXIE	0x08
#define EIE_TXERIE	0x02
#define EIE_RXERIE	0x01

#define EIR		0x1C
#define EIR_TXERIF	0x02
#define EIE_RXERIF	0x01
#define ESTAT		0x1D
#define ECON2		0x1E
#define ECON2_PKTDEC	0x40
#define ECON1		0x1F
#define ECON1_BSEL0	0x01
#define ECON1_BSEL1	0x02
#define ECON1_RXEN	0x04
#define ECON1_TXRTS	0x08
#define ECON1_TXRST	0x80

#define REG_MASK	0x1F
#define BANK_MASK	0x60
#define BANK0		0x00
#define BANK1		0x20
#define BANK2		0x40
#define BANK3		0x60
#define DUMMY		0x80

/* Bank 0. */
#define ERDPTL		(0x00 | BANK0)
#define ERDPTH		(0x01 | BANK0)
#define EWRPTL		(0x02 | BANK0)
#define EWRPTH		(0x03 | BANK0)
#define ETXSTL		(0x04 | BANK0)
#define ETXSTH		(0x05 | BANK0)
#define ETXNDL		(0x06 | BANK0)
#define ETXNDH		(0x07 | BANK0)
#define ERXSTL		(0x08 | BANK0)
#define ERXSTH		(0x09 | BANK0)
#define ERXNDL		(0x0A | BANK0)
#define ERXNDH		(0x0B | BANK0)
#define ERXRDPTL	(0x0C | BANK0)
#define ERXRDPTH	(0x0D | BANK0)

/* Bank 1. */
#define ERXFCON		(0x18 | BANK1)
#define EPKTCNT		(0x19 | BANK1)

/* Bank 2. */
#define MACON1		(0x00 | BANK2 | DUMMY)
#define MACON1_LOOBPK	0x10
#define	MACON1_TXPAUS	0x08
#define MACON1_RXPAUS	0x04
#define MACON1_PASSALL	0x02
#define MACON1_MARXEN	0x01

#define MACON2		(0x01 | BANK2 | DUMMY)
#define MACON2_MARST	0x80
#define MACON2_RNDRST	0x40
#define MACON2_MARXRST	0x08
#define MACON2_RFUNRST	0x04
#define MACON2_MATXRST	0x02
#define MACON2_TFUNRST	0x01

#define MACON3		(0x02 | BANK2 | DUMMY)
#define	MACON3_PADCFG0	0x20
#define MACON3_TXCRCEN	0x10
#define MACON3_FRMLNEN	0x02

#define MACON4		(0x03 | BANK2 | DUMMY)
#define MABBIPG		(0x04 | BANK2 | DUMMY)
#define MAIPGL		(0x06 | BANK2 | DUMMY)
#define	MAIPGH		(0x07 | BANK2 | DUMMY)
#define MACLCON1	(0x08 | BANK2 | DUMMY)
#define MACLCON2	(0x09 | BANK2 | DUMMY)
#define MAMXFLL		(0x0A | BANK2 | DUMMY)
#define MAMXFLH		(0x0B | BANK2 | DUMMY)

#define MIREGADR	(0x14 | BANK2 | DUMMY)
#define MIWRL		(0x16 | BANK2 | DUMMY)
#define MIWRH		(0x17 | BANK2 | DUMMY)

/* Bank 3. */
#define MAADR1		(0x00 | BANK3 | DUMMY)
#define MAADR0		(0x01 | BANK3 | DUMMY)
#define MAADR3		(0x02 | BANK3 | DUMMY)
#define MAADR2		(0x03 | BANK3 | DUMMY)
#define MAADR5		(0x04 | BANK3 | DUMMY)
#define MAADR4		(0x05 | BANK3 | DUMMY)
#define MISTAT		(0x0A | BANK3 | DUMMY)
#define MISTAT_BUSY	0x01
#define EREVID		(0x12 | BANK3)
#define ECOCON		(0x15 | BANK3)

#define EPMCSL           (0x10|0x20)
#define EPMCSH           (0x11|0x20)
#define EPMM0            (0x08|0x20)
#define EPMM1            (0x09|0x20)

/* PHY registers. */
#define PHCON2		0x10
#define PHCON2_HDLDIS	0x0100
#define PHLCON		0x14

/* Receive Status Vector. */
#define RSV_CRC_ERROR	0x0010
#define RSV_RECV_OK	0x0080
#endif /* !_ENC28J60_REG_H_ */
