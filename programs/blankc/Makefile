FILES=../../build/programs/blankc.o
INCLUDES= -I../stdlib
FLAGS= -g

# link object files
all: $(FILES)
	i686-elf-gcc -g -T linker.ld -o ../../bin/blankc.elf -ffreestanding -O0 -nostdlib -fpic -g $(FILES) ../../build/programs/stdlib.elf

# compile blankc.c
../../build/programs/blankc.o: blankc.c
	i686-elf-gcc $(INCLUDES) $(FLAGS) -std=gnu99 -c blankc.c -o ../../build/programs/blankc.o

clean:
	rm -rf $(FILES)
	rm -rf ../../bin/blankc.elf
