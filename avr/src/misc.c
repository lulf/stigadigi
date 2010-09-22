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
#include <misc.h>

void
blink(uint16_t delay)
{
	PORTB &= ~0x02;
	_delay_ms(delay);
	PORTB |= 0x02;
	_delay_ms(delay);
}

void blink16(uint16_t val)
{
	int i;

	for (i = 15; i >= 0; i--) {
		if (((val >> i) & 0x1) == 1) {
			blink(1000);
		} else {
			blink(100);
			blink(100);
		}
		_delay_ms(1000);
	}
}

void blink8(uint8_t val)
{
	int i;

	for (i = 7; i >= 0; i--) {
		if (((val >> i) & 0x1) == 1) {
			blink(1000);
		} else {
			blink(100);
			blink(100);
		}
		_delay_ms(1000);
	}
}


