CFILES := $(wildcard */*.c)
OFILES := $(addprefix build/, $(notdir $(CFILES:.c=.o)))

$(info CFILES: $(CFILES))
$(info OFILES: $(OFILES))

# Compiler flags
GCCFLAGS = -Wall -O2 -ffreestanding -nostdinc -nostdlib 

# Default target
all: clean kernel8.img run

# Rule for assembling boot.S
./build/boot.o: ./kernel/boot.S
	aarch64-none-elf-gcc $(GCCFLAGS) -c $< -o $@

./build/%.o: ./*/%.c
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