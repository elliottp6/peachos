# files
FILES = build/kernel.asm.o build/kernel.o build/idt/idt.asm.o build/idt/idt.o build/memory/memory.o build/io/io.asm.o build/memory/heap/heap.o build/memory/heap/kheap.o build/memory/paging/paging.o build/memory/paging/paging.asm.o build/disk/disk.o build/fs/pparser.o build/string/string.o build/disk/streamer.o build/fs/file.o build/fs/fat/fat16.o build/gdt/gdt.asm.o build/gdt/gdt.o build/task/tss.asm.o build/task/task.asm.o build/task/task.o build/task/process.o build/isr80h/isr80h.o build/isr80h/misc.o build/isr80h/io.o build/keyboard/keyboard.o build/keyboard/classic.o build/loader/formats/elf.o build/loader/formats/elfloader.o build/isr80h/heap.o build/isr80h/process.o
INCLUDES = -I./src
FLAGS = -g -ffreestanding -falign-jumps -falign-functions -falign-labels -falign-loops -fstrength-reduce -fomit-frame-pointer -finline-functions -Wno-unused-function -fno-builtin -Werror -Wno-unused-label -Wno-cpp -Wno-unused-parameter -nostdlib -nostartfiles -nodefaultlibs -Wall -O0 -Iinc

# build binary: boot w/ FAT16 header, kernel in reserved sectors, then 16MB of null bytes, then write w/ our "hello.txt" written into it as a FAT16 drive
all: bin/boot.bin bin/kernel.bin user_programs
	rm -rf bin/os.bin
	dd if=bin/boot.bin >> bin/os.bin
	dd if=bin/kernel.bin >> bin/os.bin
	dd if=/dev/zero bs=1048576 count=16 >> bin/os.bin
	sudo mount -t vfat bin/os.bin /mnt/d
	sudo cp hello.txt /mnt/d
	sudo cp bin/blank.bin /mnt/d
	sudo cp bin/blank.elf /mnt/d
	sudo cp bin/blankc.elf /mnt/d
	sudo cp bin/shell.elf /mnt/d
	sudo umount /mnt/d

# link kernel (note that kernel.asm MUST be the first object file, so that we get the entry point in the right place)
bin/kernel.bin: $(FILES)
	i686-elf-ld -g -relocatable $(FILES) -o build/kernelfull.o
	i686-elf-gcc $(FLAGS) -T src/linker.ld -o bin/kernel.bin -ffreestanding -O0 -nostdlib build/kernelfull.o

# assemble bootloader
bin/boot.bin: src/boot/boot.asm
	nasm -f bin src/boot/boot.asm -o bin/boot.bin

# assemble kernel
build/kernel.asm.o: src/kernel.asm
	nasm -f elf -g src/kernel.asm -o build/kernel.asm.o

# compile kernel.c
build/kernel.o: src/kernel.c
	i686-elf-gcc $(INCLUDES) $(FLAGS) -std=gnu99 -c src/kernel.c -o build/kernel.o

# assemble IDT loader
build/idt/idt.asm.o: src/idt/idt.asm
	nasm -f elf -g src/idt/idt.asm -o build/idt/idt.asm.o

# compile IDT functions
build/idt/idt.o: src/idt/idt.c
	i686-elf-gcc $(INCLUDES) -I./src/idt $(FLAGS) -std=gnu99 -c src/idt/idt.c -o build/idt/idt.o

# compile memory functions
build/memory/memory.o: src/memory/memory.c
	i686-elf-gcc $(INCLUDES) -I./src/memory $(FLAGS) -std=gnu99 -c src/memory/memory.c -o build/memory/memory.o

# assemble IO
build/io/io.asm.o: src/io/io.asm
	nasm -f elf -g src/io/io.asm -o build/io/io.asm.o

# compile heap functions
build/memory/heap/heap.o: src/memory/heap/heap.c
	i686-elf-gcc $(INCLUDES) -I./src/memory/heap $(FLAGS) -std=gnu99 -c src/memory/heap/heap.c -o build/memory/heap/heap.o

# compile kheap functions
build/memory/heap/kheap.o: src/memory/heap/kheap.c
	i686-elf-gcc $(INCLUDES) -I./src/memory/heap $(FLAGS) -std=gnu99 -c src/memory/heap/kheap.c -o build/memory/heap/kheap.o

# compile paging functions
build/memory/paging/paging.o: src/memory/paging/paging.c
	i686-elf-gcc $(INCLUDES) -I./src/memory/paging $(FLAGS) -std=gnu99 -c src/memory/paging/paging.c -o build/memory/paging/paging.o

# assemble paging functions
build/memory/paging/paging.asm.o: src/memory/paging/paging.asm
	nasm -f elf -g src/memory/paging/paging.asm -o build/memory/paging/paging.asm.o

# compile disk functions
build/disk/disk.o: src/disk/disk.c
	i686-elf-gcc $(INCLUDES) -I./src/disk $(FLAGS) -std=gnu99 -c src/disk/disk.c -o build/disk/disk.o

# compile path-parser functions
build/fs/pparser.o: src/fs/pparser.c
	i686-elf-gcc $(INCLUDES) -I./src/fs $(FLAGS) -std=gnu99 -c src/fs/pparser.c -o build/fs/pparser.o

# compile string functions
build/string/string.o: src/string/string.c
	i686-elf-gcc $(INCLUDES) -I./src/string $(FLAGS) -std=gnu99 -c src/string/string.c -o build/string/string.o

# compile file streamer
build/disk/streamer.o: src/disk/streamer.c
	i686-elf-gcc $(INCLUDES) -I./src/disk $(FLAGS) -std=gnu99 -c src/disk/streamer.c -o build/disk/streamer.o

# compile virtual filesystem (VFS)
build/fs/file.o: src/fs/file.c
	i686-elf-gcc $(INCLUDES) -I./src/fs $(FLAGS) -std=gnu99 -c src/fs/file.c -o build/fs/file.o

# compile static FAT16 driver
build/fs/fat/fat16.o: src/fs/fat/fat16.c
	i686-elf-gcc $(INCLUDES) -I./src/fs -I./src/fs/fat $(FLAGS) -std=gnu99 -c src/fs/fat/fat16.c -o build/fs/fat/fat16.o

# assemble GDT.asm
build/gdt/gdt.asm.o: src/gdt/gdt.asm
	nasm -f elf -g src/gdt/gdt.asm -o build/gdt/gdt.asm.o

# compile GDT.c functions
build/gdt/gdt.o: src/gdt/gdt.c
	i686-elf-gcc $(INCLUDES) -I./src/gdt $(FLAGS) -std=gnu99 -c src/gdt/gdt.c -o build/gdt/gdt.o

# assemble tss.asm
build/task/tss.asm.o: src/task/tss.asm
	nasm -f elf -g src/task/tss.asm -o build/task/tss.asm.o

# assemble task.asm
build/task/task.asm.o: src/task/task.asm
	nasm -f elf -g src/task/task.asm -o build/task/task.asm.o

# compile task.c functions
build/task/task.o: src/task/task.c
	i686-elf-gcc $(INCLUDES) -I./src/task $(FLAGS) -std=gnu99 -c src/task/task.c -o build/task/task.o

# compile process.c functions
build/task/process.o: src/task/process.c
	i686-elf-gcc $(INCLUDES) -I./src/task $(FLAGS) -std=gnu99 -c src/task/process.c -o build/task/process.o

# compile isr80h.c functions
build/isr80h/isr80h.o: src/isr80h/isr80h.c
	i686-elf-gcc $(INCLUDES) -I./src/isr80h $(FLAGS) -std=gnu99 -c src/isr80h/isr80h.c -o build/isr80h/isr80h.o

# compile misc.c functions
build/isr80h/misc.o: src/isr80h/misc.c
	i686-elf-gcc $(INCLUDES) -I./src/isr80h $(FLAGS) -std=gnu99 -c src/isr80h/misc.c -o build/isr80h/misc.o

# compile io.c functions
build/isr80h/io.o: src/isr80h/io.c
	i686-elf-gcc $(INCLUDES) -I./src/isr80h $(FLAGS) -std=gnu99 -c src/isr80h/io.c -o build/isr80h/io.o

# compile isr80h/process.c functions
build/isr80h/process.o: src/isr80h/process.c
	i686-elf-gcc $(INCLUDES) -I./src/isr80h $(FLAGS) -std=gnu99 -c src/isr80h/process.c -o build/isr80h/process.o

# compile keyboard.c functions
build/keyboard/keyboard.o: src/keyboard/keyboard.c
	i686-elf-gcc $(INCLUDES) -I./src/keyboard $(FLAGS) -std=gnu99 -c src/keyboard/keyboard.c -o build/keyboard/keyboard.o

# compile classic.c functions (classic keyboard driver)
build/keyboard/classic.o: src/keyboard/classic.c
	i686-elf-gcc $(INCLUDES) -I./src/keyboard $(FLAGS) -std=gnu99 -c src/keyboard/classic.c -o build/keyboard/classic.o

# compile loader/formats/elf.c functions
build/loader/formats/elf.o: src/loader/formats/elf.c
	i686-elf-gcc $(INCLUDES) -I./src/loader/formats $(FLAGS) -std=gnu99 -c src/loader/formats/elf.c -o build/loader/formats/elf.o

# compile loader/formats/elfloader.c functions
build/loader/formats/elfloader.o: src/loader/formats/elfloader.c
	i686-elf-gcc $(INCLUDES) -I./src/loader/formats $(FLAGS) -std=gnu99 -c src/loader/formats/elfloader.c -o build/loader/formats/elfloader.o

# compile heap.c functions
build/isr80h/heap.o: src/isr80h/heap.c
	i686-elf-gcc $(INCLUDES) -I./src/isr80h $(FLAGS) -std=gnu99 -c src/isr80h/heap.c -o build/isr80h/heap.o

# build programs
user_programs:
	cd programs/stdlib && $(MAKE) all
	cd programs/shell && $(MAKE) all
	cd programs/blank && $(MAKE) all
	cd programs/blankc && $(MAKE) all

# clean programs
user_programs_clean:
	cd programs/stdlib && $(MAKE) clean
	cd programs/shell && $(MAKE) clean
	cd programs/blank && $(MAKE) clean
	cd programs/blankc && $(MAKE) clean

# clean project
clean: user_programs_clean
	rm -rf $(FILES)
	rm -rf bin/boot.bin
	rm -rf bin/kernel.bin
	rm -rf bin/os.bin
	rm -rf build/kernelfull.o
