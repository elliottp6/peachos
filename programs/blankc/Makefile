FILES=../../build/blankc.o
INCLUDES=

# link files
all: $(FILES)
	i686-elf-gcc -g -T linker.ld -o ../../bin/blankc.elf -ffreestanding -O0 -nostdlib -fpic -g ../../build/programs/blankc.o

# compile blankc.c
../../build/blankc.o: blankc.c
	i686-elf-gcc ${INCLUDES} -I./$(FLAGS) -std=gnu99 -c blankc.c -o ../../build/programs/blankc.o

clean:
	rm -rf $(FILES)
	rm ../../bin/blankc.elf

