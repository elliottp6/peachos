FILES=../../build/programs/start.o ../../build/programs/peachos.o ../../build/programs/stdlib.o
INCLUDES=
FLAGS= -g

# link files
all: ${FILES}
	i686-elf-ld -m elf_i386 -relocatable ${FILES} -o ../../build/programs/stdlib.elf

# assemble start.asm
../../build/programs/start.o:
	nasm -f elf start.asm -o ../../build/programs/start.o

# assemble peachos.asm
../../build/programs/peachos.o:
	nasm -f elf peachos.asm -o ../../build/programs/peachos.o

# compile stdlib.c
../../build/programs/stdlib.o: stdlib.c
	i686-elf-gcc $(INCLUDES) $(FLAGS) -std=gnu99 -c stdlib.c -o ../../build/programs/stdlib.o

clean:
	rm -rf ${FILES}
	rm -rf ../../build/programs/stdlib.elf
