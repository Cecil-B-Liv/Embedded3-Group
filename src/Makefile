# Collect all C source files from kernel and drivers
CFILES = $(wildcard ./kernel/*.c) $(wildcard ./drivers/*.c)

# Convert source paths to object paths (flat in ./build/)

# Step 1: Strip directory path (e.g., ./kernel/foo.c → foo.c)
OFILES = $(notdir $(CFILES))

# Step 2: Replace .c extension with .o (e.g., foo.c → foo.o)
OFILES := $(OFILES:.c=.o)

# Step 3: Add build/ prefix to object files (e.g., foo.o → ./build/foo.o)
OFILES := $(addprefix ./build/, $(OFILES))


# Compiler flags
GCCFLAGS = -Wall -O2 -ffreestanding -nostdinc -nostdlib 

# Default target
all: clean kernel8.img run

# Rule for assembling boot.S
./build/boot.o: ./kernel/boot.S
	aarch64-none-elf-gcc $(GCCFLAGS) -c $< -o $@

./build/%.o: ./kernel/%.c
	aarch64-none-elf-gcc $(GCCFLAGS) -c $< -o $@

./build/%.o: ./drivers/%.c
	aarch64-none-elf-gcc $(GCCFLAGS) -c $< -o $@

# Link all object files into kernel8.img
kernel8.img: ./build/boot.o $(OFILES)
	aarch64-none-elf-ld -nostdlib $^ -T ./kernel/link.ld -o ./build/kernel8.elf
	aarch64-none-elf-objcopy -O binary ./build/kernel8.elf kernel8.img

# Clean target
clean:
	del .\build\kernel8.elf .\build\*.o *.img

# Run with QEMU
run:
	qemu-system-aarch64 -M raspi3 -kernel kernel8.img -serial null -serial stdio