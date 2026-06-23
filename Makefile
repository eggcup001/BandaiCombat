MCU      = attiny85
F_CPU    = 8000000UL
TARGET   = combat

USERPROFILE_POSIX := $(subst \,/,$(USERPROFILE))
PIO_AVR_BIN       := $(USERPROFILE_POSIX)/.platformio/packages/toolchain-atmelavr/bin
PIO_AVRDUDE       := $(USERPROFILE_POSIX)/.platformio/packages/tool-avrdude/avrdude.exe

ifneq ($(wildcard $(PIO_AVR_BIN)/avr-gcc.exe),)
CC       = $(PIO_AVR_BIN)/avr-gcc
OBJCOPY  = $(PIO_AVR_BIN)/avr-objcopy
SIZE     = $(PIO_AVR_BIN)/avr-size
else
CC       = avr-gcc
OBJCOPY  = avr-objcopy
SIZE     = avr-size
endif

ifneq ($(wildcard $(PIO_AVRDUDE)),)
AVRDUDE  = $(PIO_AVRDUDE)
else
AVRDUDE  = avrdude
endif

ifeq ($(OS),Windows_NT)
MKDIR_BUILD = if not exist build mkdir build
RMDIR_BUILD = if exist build rmdir /s /q build
else
MKDIR_BUILD = mkdir -p build
RMDIR_BUILD = rm -rf build
endif

CFLAGS = -mmcu=$(MCU) -DF_CPU=$(F_CPU) -Os -std=c99 -Wall -Wextra
CFLAGS += -Iinclude
LDFLAGS = -mmcu=$(MCU)

SRC = \
src/main.c \
src/util/cell.c \
src/util/random.c \
src/util/timer.c \
src/layer1/shift595.c \
src/layer1/adc_ladder.c \
src/layer1/buzzer.c \
src/layer2/display_mux.c \
src/layer2/matrix.c \
src/layer2/buttons.c \
src/layer2/display.c \
src/layer2/sound.c \
src/layer3/stage_patterns.c \
src/layer3/stage.c \
src/layer3/combat_game.c

OBJ = $(SRC:.c=.o)
HDR = $(wildcard include/*.h include/*/*.h)

all: build/$(TARGET).hex

build:
	$(MKDIR_BUILD)

build/$(TARGET).elf: $(SRC) $(HDR) | build
	$(CC) $(CFLAGS) $(SRC) -o $@
	$(SIZE) $@

build/$(TARGET).hex: build/$(TARGET).elf
	$(OBJCOPY) -O ihex -R .eeprom $< $@

flash: build/$(TARGET).hex
	$(AVRDUDE) -p t85 -c usbasp -U flash:w:build/$(TARGET).hex

fuse:
	$(AVRDUDE) -p t85 -c usbasp \
	-U lfuse:w:0xE2:m \
	-U hfuse:w:0xDF:m \
	-U efuse:w:0xFF:m

clean:
	$(RMDIR_BUILD)

.PHONY: all flash fuse clean
