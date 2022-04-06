all:
	nasm -f elf blank.asm -o ../../build/programs/blank.o
	i686-elf-gcc -g -T linker.ld -o blank.bin -ffreestanding -O0 -nostdlib -fpic -g ../../build/programs/blank.o

clean:
	rm -rf ../../build/programs/blank.o
