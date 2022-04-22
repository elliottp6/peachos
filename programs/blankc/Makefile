FILES=../../build/programs/blankc.o
INCLUDES= -I../stdlib
FLAGS = -g -ffreestanding -falign-jumps -falign-functions -falign-labels -falign-loops -fstrength-reduce -fomit-frame-pointer -finline-functions -Wno-unused-function -fno-builtin -Werror -Wno-unused-label -Wno-cpp -Wno-unused-parameter -nostdlib -nostartfiles -nodefaultlibs -Wall -O0 -Iinc

# link object files
all: $(FILES)
	i686-elf-gcc -g -T linker.ld -o ../../bin/blankc.elf -ffreestanding -O0 -nostdlib -fpic -g $(FILES) ../../build/programs/stdlib.elf

# compile blankc.c
../../build/programs/blankc.o: blankc.c
	i686-elf-gcc $(INCLUDES) $(FLAGS) -std=gnu99 -c blankc.c -o ../../build/programs/blankc.o

clean:
	rm -rf $(FILES)
	rm -rf ../../bin/blankc.elf
