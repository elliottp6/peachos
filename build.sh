# setup environment variables
export PREFIX="$HOME/opt/cross"
export TARGET=i686-elf
export PATH="$PREFIX/bin:$PATH"

# build folder for OS
mkdir -p bin
mkdir -p build
mkdir -p build/idt
mkdir -p build/memory
mkdir -p build/memory/heap
mkdir -p build/memory/paging
mkdir -p build/io
mkdir -p build/disk
mkdir -p build/string
mkdir -p build/fs
mkdir -p build/fs/fat
mkdir -p build/gdt
mkdir -p build/task
mkdir -p build/isr80h

# build folder for programs
mkdir -p build/programs

# make project
make all && qemu-system-i386 -hda bin/os.bin
#make all && qemu-system-i386 -hda bin/os.bin
#make all && qemu-system-x86_64 -hda bin/os.bin

# --mount os.bin as FAT16 drive--
# sudo mkdir /mnt/d
# sudo mount -t vfat os.bin /mnt/d

# cd bin
# gdb
# add-symbol-file ../build/kernelfull.o 0x100000
# break _start
# target remote | qemu-system-x86_64 -S -gdb stdio -hda os.bin
# OR: target remote | qemu-system-i386 -S -gdb stdio -hda os.bin
# layout asm
# layout prev (when in layout asm, this will take you back to the C code layout)
# stepi (next assembly instruction) OR next (next line of C code)
# bt = backtrace
# break kernel.c:55
# break *0x400000 // break when ip points to this address, which is where user programs start from
# print variable_name
# c = continue
# print $eax
# print $edx
# print buffer
# print (unsigned char)buffer[0]
# print *root_path

#debug mode: gdb, target remote | qemu-system-x86_64 -hda bin/boot.bin -S -gdb stdio
# gdb commands: c (continue), layout asm, info registers

#ndisasm bin/os.bin
#bless bin/os.bin

# list block devices
#lsblk

# should we write to /dev/sda?
#read -p "Write to /dev/sda? (y/n)" yn

# bail if we didn't get a 'y'
#if [ $yn != 'y' ]; then
#    echo "bailing out!"
#    exit 1
#fi

# write boot sector to USB stick
#sudo dd if=bin/os.bin of=/dev/sda
