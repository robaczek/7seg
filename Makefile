# Główny plik Makefile projektu

# Przedrostek nazwy kompilatora, identyfikujący platformę:
CROSS_COMPILE = arm-none-eabi-

# Końcowa nazwa pliku wyjściowego:
EXEC_FILE = test

# Narzędzia budowania:
CC = $(CROSS_COMPILE)gcc
CP = $(CROSS_COMPILE)objcopy
AS = $(CROSS_COMPILE)gcc -x assembler-with-cpp
ELF2BIN = $(CP) -O binary -S

# Identyfikator procesora:
UC = cortex-m0

# Adres flash na urządzeniu:
FLASH_START = 0x8000000

# Plik definiujący stałe platformy, umożliwiający start.
# Dostarczony przez ST.
STARTUP_FILE  = ./vendor/startup_stm32f0xx.s

# Plik definiujący proces łączenia plików w końcowy program,
# z zachowaniem poprawnego adresowania. Dostarczony przez ST.
LINKER_FILE = ./vendor/stm32f0_linker.ld

# Lokalizacja plików dostarczonych przez STM:
STM_ROOTDIR = ../STM32F0-Discovery_FW_V1.0.0

DEBUG = -g -gdwarf-2 # ustawienia załączenia symboli debuggera

MCFLAGS = -mcpu=$(UC) -mthumb # parametr ustawiający konkretny procesor

# Ustawienia procesu asemblacji:
ASFLAGS = $(MCFLAGS) $(DEBUG) -mthumb -Wa,-amhls=$(<:.s=.lst)

# Ustawienia procesu kompilacji:
CFLAGS = $(MCFLAGS) $(DEBUG) -fomit-frame-pointer -Wall -Wstrict-prototypes -fverbose-asm -Wa,-ahlms=$(<:.c=.lst) -DRUN_FROM_FLASH=1

# Ustawienia procesu łączenia:
LDFLAGS = $(MCFLAGS) $(DEBUG) -nostartfiles -T$(LINKER_FILE) -Wl,-Map=$(EXEC_FILE).map,--cref,--no-warn-mismatch

# Pliki źródłowe do kompilacji:
SRC = ./src/main.c \
./vendor/system_stm32f0xx.c \
./vendor/stm32f0xx_it.c \
./src/pt6961.c \
./src/delay.c \
./src/mini-printf.c \
./src/gpiopin.c

# Ścieżki dołączanych plików nagłówkowych:
INCLUDE_DIRS = ./include \
	$(STM_ROOTDIR)/Libraries/CMSIS/Include \
	$(STM_ROOTDIR)/Libraries/CMSIS/ST/STM32F0xx/Include \
	$(STM_ROOTDIR)/Project/Demonstration \
	$(STM_ROOTDIR)/Libraries/STM32F0xx_StdPeriph_Driver/inc

# Tworzenie parametrów dla kompilatora na podstawie listy katalogów:
INCLUDES = $(patsubst %,-I%, $(INCLUDE_DIRS))

# Pliki objektów (nazwy tworzone dynamicznie z listy źródeł):
OBJECTS = $(SRC:.c=.o) $(STARTUP_FILE:.s=.o)

# Cele makefile - część wykonywalna:

# Cel domyślny, wykonywany przy wywołaniu bez parametrów.
# Tworzy końcowy plik binarny, bez zapisu do pamięci flash.
all: $(EXEC_FILE).elf $(EXEC_FILE).bin
	$(CROSS_COMPILE)size $(EXEC_FILE).elf

%.o: %.c
	$(CC) -c $(CFLAGS) $(INCLUDES) $< -o $@

%.o: %.s
	$(AS) -c $(ASFLAGS) $< -o $@

%.elf: $(OBJECTS)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@

%.bin: %.elf
	$(ELF2BIN) $< $@

# Usunięcie efektów budowania - przywrócenie czystego drzewa projektu.
clean:
	-rm -rf $(OBJECTS)
	-rm -rf $(EXEC_FILE).elf
	-rm -rf $(EXEC_FILE).map
	-rm -rf $(EXEC_FILE).bin
	-rm -rf $(SRC:.c=.lst)
	-rm -rf $(STARTUP_FILE:.s=.lst)

flash: $(EXEC_FILE).bin
	st-info --flash
	st-flash write $(EXEC_FILE).bin $(FLASH_START)

.PHONY: clean all
