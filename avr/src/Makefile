AS=avr-as
CC=avr-gcc
LD=avr-ld
OBJCOPY=avr-objcopy
SIZE=avr-size
CFLAGS=-g -Wall -Wstrict-prototypes -mcall-prologues -mmcu=atmega328p -Os -I/usr/lib/avr/include -I../include
LDFLAGS=-mavr5
OBJCOPYFLAGS=-O ihex -R .eeprom
LIBS=
DEVS=dev/spi.c dev/enc28j60.c
NET=net/ethernet.c net/ip.c net/arp.c
SRCS=main.c $(DEVS) $(NET) misc.c
OBJS=$(SRCS:.c=.o)
TARGET=stigadigital
PROGTARGET=$(TARGET).hex
PART=atmega328p
DEVBOARD=stk500v2
SERIAL=/dev/ttyUSB0
AVRDUDE=/usr/bin/avrdude

all: $(PROGTARGET)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $@ $(SRCS) $(LIBS)
	$(SIZE) $(TARGET)

$(PROGTARGET): $(TARGET)
	$(OBJCOPY) $(OBJCOPYFLAGS) $(TARGET) $(PROGTARGET)

upload: $(PROGTARGET)
	$(AVRDUDE) -p $(PART) -c $(DEVBOARD) -P $(SERIAL) -U flash:w:$(PROGTARGET)

clean:
	rm -f $(OBJS) $(TARGET) $(PROGTARGET)
