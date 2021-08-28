# ============================================================================
#  Edit these lines to match the microcontroller type and programmer
# ============================================================================

MCU := atxmega64a3u
PROGRAMMER := avrispv2
AVRDUDE_FLAGS := -p $(MCU) -c $(PROGRAMMER) -P usb

# ============================================================================
#  Use caution changing the following values.
# ============================================================================

CC := avr-gcc
F_CPU := 2000000
WARNINGS := -Wall
CORE_OPTS := -Os -std=c99 -flto -mrelax
INC := -I. -Ilib/pff
VPATH := .:lib/pff
CFLAGS ?= $(WARNINGS) $(CORE_OPTS) -mmcu=$(MCU) -DF_CPU=$(F_CPU) $(INC)
ASFLAGS ?= -mmcu=$(MCU) -DF_CPU=$(F_CPU)

MAIN := program
ASRCS := sp_driver.S
CSRCS := disk.c pff.c main.c
OBJS := $(ASRCS:.S=.o) $(CSRCS:.c=.o)

.PHONY: all
all: $(MAIN).bin

.PHONY: clean
clean:
	rm -f $(MAIN).elf $(MAIN).hex $(MAIN).bin $(MAIN).lst $(OBJS)

.PHONY: flash
flash: $(MAIN).bin
	avrdude $(AVRDUDE_FLAGS) -U boot:w:$<

.PHONY: reset
reset:
	avrdude $(AVRDUDE_FLAGS)

.PHONY: fuses
fuses:
	avrdude $(AVRDUDE_FLAGS) -U fuse2:w:0xBF:m

.PHONY: dump
dump: $(MAIN).elf
	avr-objdump -d -S -m avr $< > $(MAIN).lst

$(MAIN).elf: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ -g $(OBJS)
	avr-size -C --mcu=$(MCU) $@

$(MAIN).hex: $(MAIN).elf
	avr-objcopy -j .text -j .data -O ihex $< $@

$(MAIN).bin: $(MAIN).hex
	avr-objcopy -I ihex -O binary $< $@
