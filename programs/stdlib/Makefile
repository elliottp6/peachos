FILES=../../build/programs/start.o
INCLUDES=

all: ${FILES}
	i686-elf-ld -m elf_i386 -relocatable ${FILES} -o ../../build/programs/stdlib.elf

../../build/programs/start.o:
	nasm -f elf start.asm -o ../../build/programs/start.o

clean:
	rm -rf ${FILES}
	rm -rf ../../build/programs/stdlib.elf
