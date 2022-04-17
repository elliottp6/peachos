all:
	nasm -f elf blank.asm -o ../../build/programs/blank.o
	i686-elf-gcc -g -T linker.ld -o ../../bin/blank.bin -ffreestanding -O0 -nostdlib -fpic -g ../../build/programs/blank.o
	i686-elf-gcc -g -T linker_elf.ld -o ../../bin/blank.elf -ffreestanding -O0 -nostdlib -fpic -g ../../build/programs/blank.o

clean:
	rm -rf ../../build/programs/blank.o
