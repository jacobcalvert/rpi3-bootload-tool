###############################################################
#
#
#
#
#
###############################################################


# Tool Defs
CC = aarch64-elf-gcc
LD = $(CC)
AR = aarch64-elf-ar
AS = aarch64-elf-as
RM = rm -rf
LS = ls -alh
CP = cp -R
OBJCOPY = aarch64-elf-objcopy
OBJDUMP = aarch64-elf-objdump

Q=@


# Flags and such
CC_FLAGS = -Wall -O0 -ffreestanding -nostdlib -nostartfiles -std=c11 -g -march=armv8-a+simd -mtune=cortex-a53 -fno-tree-loop-vectorize -fno-tree-slp-vectorize -I ./ -fpic
LD_FLAGS = 
AS_FLAGS = 


# Inputs

SRC := $(shell find . -type f -name "*.c")
OBJ := $(patsubst %.c,build/%.o,$(SRC)) build/boot.o




all: $(OBJ)
	$(Q)echo "Linking bootloader"
	$(Q)$(CC) -T linker.ld -Xlinker -Map=build/bootloader.map -o build/bootloader.elf -ffreestanding -O2 -nostdlib $(OBJ)
	$(Q)$(OBJCOPY) build/bootloader.elf -O binary build/kernel7.img
	$(Q)$(OBJDUMP) -f build/bootloader.elf
	
clean:
	$(Q)$(RM) build/*



build/%.o:%.c
	$(Q)echo "$<"
	$(Q)$(CC) $(CC_FLAGS) -c -o $@ $<


build/boot.o: boot.S
	$(Q)echo "$<"
	$(Q)$(CC) $(CC_FLAGS) -c -o $@ $<


test_blobs:
	head -c 1k /dev/urandom > build/1k.bin
	head -c 256k /dev/urandom > build/256k.bin
	head -c 1M /dev/urandom > build/1M.bin
	head -c 10M /dev/urandom > build/10M.bin



 

