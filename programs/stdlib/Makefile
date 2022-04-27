FILES=../../build/programs/start.asm.o ../../build/programs/start.o ../../build/programs/peachos.asm.o ../../build/programs/peachos.o ../../build/programs/stdlib.o ../../build/programs/stdio.o ../../build/programs/memory.o ../../build/programs/string.o
INCLUDES=
FLAGS = -g -ffreestanding -falign-jumps -falign-functions -falign-labels -falign-loops -fstrength-reduce -fomit-frame-pointer -finline-functions -Wno-unused-function -fno-builtin -Werror -Wno-unused-label -Wno-cpp -Wno-unused-parameter -nostdlib -nostartfiles -nodefaultlibs -Wall -O0 -Iinc

# link files
all: ${FILES}
	i686-elf-ld -m elf_i386 -relocatable ${FILES} -o ../../build/programs/stdlib.elf

# assemble start.asm
../../build/programs/start.asm.o:
	nasm -f elf start.asm -o ../../build/programs/start.asm.o

# compile start.c
../../build/programs/start.o: start.c
	i686-elf-gcc $(INCLUDES) $(FLAGS) -std=gnu99 -c start.c -o ../../build/programs/start.o

# assemble peachos.asm
../../build/programs/peachos.asm.o:
	nasm -f elf peachos.asm -o ../../build/programs/peachos.asm.o

# compile peachos.c
../../build/programs/peachos.o: peachos.c
	i686-elf-gcc $(INCLUDES) $(FLAGS) -std=gnu99 -c peachos.c -o ../../build/programs/peachos.o

# compile stdlib.c
../../build/programs/stdlib.o: stdlib.c
	i686-elf-gcc $(INCLUDES) $(FLAGS) -std=gnu99 -c stdlib.c -o ../../build/programs/stdlib.o

# compile stdio.c
../../build/programs/stdio.o: stdio.c
	i686-elf-gcc $(INCLUDES) $(FLAGS) -std=gnu99 -c stdio.c -o ../../build/programs/stdio.o

# compile memory.c
../../build/programs/memory.o: memory.c
	i686-elf-gcc $(INCLUDES) $(FLAGS) -std=gnu99 -c memory.c -o ../../build/programs/memory.o

# compile string.c
../../build/programs/string.o: string.c
	i686-elf-gcc $(INCLUDES) $(FLAGS) -std=gnu99 -c string.c -o ../../build/programs/string.o

clean:
	rm -rf ${FILES}
	rm -rf ../../build/programs/stdlib.elf
