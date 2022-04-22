FILES=../../build/programs/start.o ../../build/programs/peachos.o
INCLUDES=

all: ${FILES}
	i686-elf-ld -m elf_i386 -relocatable ${FILES} -o ../../build/programs/stdlib.elf

../../build/programs/start.o:
	nasm -f elf start.asm -o ../../build/programs/start.o

../../build/programs/peachos.o:
	nasm -f elf peachos.asm -o ../../build/programs/peachos.o

clean:
	rm -rf ${FILES}
	rm -rf ../../build/programs/stdlib.elf
