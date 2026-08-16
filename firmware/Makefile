MCU = atxmega128a1
F_CPU = 32000000UL
PROGRAMMER = avrispmkII
PORT = usb

TARGET = build/xmega_domoticz_panel
SRCS = \
	app/main.c \
	board/mikromedia_xmega_adapter.c \
	drivers/accel/adxl345.c \
	drivers/lcd/lcd_ili9341.c \
	protocol/esp_protocol.c \
	touch/touch_resistive.c \
	transport/i2c_twid.c \
	transport/uart_debug.c \
	transport/uart_esp.c \
	ui/xmega_menu.c \
	ui/xmega_ui.c

CC = avr-gcc
OBJCOPY = avr-objcopy
SIZE = avr-size
AVRDUDE = avrdude

CFLAGS = -std=c99 -Wall -Wextra -Os -mmcu=$(MCU) -DF_CPU=$(F_CPU)
CFLAGS += -Iapp -Iboard -Idrivers/accel -Idrivers/lcd -Iprotocol -Itouch -Itransport -Iui
LDFLAGS = -mmcu=$(MCU)

OBJS = $(SRCS:%.c=build/%.o)

.PHONY: all clean flash size

all: $(TARGET).hex size

build:
	mkdir -p build

build/%.o: %.c | build
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET).elf: $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $@

$(TARGET).hex: $(TARGET).elf
	$(OBJCOPY) -O ihex -R .eeprom $< $@

size: $(TARGET).elf
	$(SIZE) --mcu=$(MCU) -C $<

flash: $(TARGET).hex
	$(AVRDUDE) -c $(PROGRAMMER) -p x128a1 -P $(PORT) -U flash:w:$(TARGET).hex:i

clean:
	rm -rf build
