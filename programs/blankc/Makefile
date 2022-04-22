FILES=../../build/programs/blankc.o ../../build/programs/start.o
INCLUDES=

# link object files
all: $(FILES)
	i686-elf-gcc -g -T linker.ld -o ../../bin/blankc.elf -ffreestanding -O0 -nostdlib -fpic -g $(FILES)

# compile blankc.c
../../build/programs/blankc.o: blankc.c
	i686-elf-gcc $(INCLUDES) -I./$(FLAGS) -std=gnu99 -c blankc.c -o ../../build/programs/blankc.o

# assemble start.asm
../../build/programs/start.o: start.asm
	nasm -f elf start.asm -o ../../build/programs/start.o

clean:
	rm -rf $(FILES)
	rm ../../bin/blankc.elf
