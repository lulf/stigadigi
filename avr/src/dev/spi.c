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

#include <avr/io.h>
#include <avr/interrupt.h>

#include <spi.h>

#define SPI_PORT	PORTB
#define SPI_CS		2 //PORTB2

#define DD_CS		2 //PORTB2
#define DD_MOSI		PORTB3
#define DD_MISO		PORTB4
#define DD_SCK		PORTB5
#define DDR_SPI		DDRB

/**
 * Initialize the SPI bus.
 */
void
spi_init()
{
	/* Configure our port with the appropriate pins. */
	DDR_SPI |= (1 << DD_CS);
	spi_deactivate();
	DDR_SPI |= (1 << DD_MOSI) | (1 << DD_SCK);
	/* Set initial values */

	DDR_SPI &= ~(1 << DD_MISO);
	SPI_PORT &= ~(1 << DD_MOSI);
	SPI_PORT &= ~(1 << DD_SCK);
	/* Set ourselves as master. */
	SPCR = (1 << SPE) | (1 << MSTR);
	SPSR |= (1 << SPI2X);
}

/*
 * Activate chip select on SPI.
 */
void
spi_activate(void)
{
	SPI_PORT &= ~(1 << SPI_CS);
}

/*
 * Deactivate chip select on SPI.
 */
void
spi_deactivate(void)
{
	SPI_PORT |= (1 << SPI_CS);
}

/**
 * Transfer one character over the SPI bus synchronously.
 */
void
spi_transmit(uint8_t c)
{
	SPDR = c;
	/* Wait until transfer complete. */
	while (!(SPSR & (1 << SPIF)));
}

/**
 * Receive one character over the SPI bus synchronously.
 */
uint8_t
spi_receive(void)
{
	SPDR = 0x00;
	/* Wait for data to arrive. */
	while (!(SPSR & (1 << SPIF)));
	return (SPDR);
}
