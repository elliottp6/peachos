FILES=../../build/programs/start.o ../../build/programs/peachos.o ../../build/programs/stdlib.o ../../build/programs/stdio.o
INCLUDES=
FLAGS = -g -falign-jumps -falign-functions -falign-labels -falign-loops -fstrength-reduce -fomit-frame-pointer -finline-functions -Wno-unused-function -fno-builtin -Werror -Wno-unused-label -Wno-cpp -Wno-unused-parameter -nostdlib -nostartfiles -nodefaultlibs -Wall -O0 -Iinc

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

# compile stdio.c
../../build/programs/stdio.o: stdio.c
	i686-elf-gcc $(INCLUDES) $(FLAGS) -std=gnu99 -c stdio.c -o ../../build/programs/stdio.o

clean:
	rm -rf ${FILES}
	rm -rf ../../build/programs/stdlib.elf
