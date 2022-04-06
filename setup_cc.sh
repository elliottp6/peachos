# https://wiki.osdev.org/GCC_Cross-Compiler

# install dependencies (note: libcloog-isl-dev omitted since it's missing & optional)
sudo apt install build-essential bison flex libgmp3-dev libmpc-dev libmpfr-dev texinfo libisl-dev

# download binutils & gcc
mkdir -p ~/src
cd ~/src
wget --no-clobber https://ftp.gnu.org/gnu/binutils/binutils-2.35.tar.xz
wget --no-clobber https://ftp.gnu.org/gnu/gcc/gcc-10.2.0/gcc-10.2.0.tar.xz

# extract binutils & gcc
tar -xf binutils-2.35.tar.xz
tar -xf gcc-10.2.0.tar.xz

# setup environment variables
export PREFIX="$HOME/opt/cross"
export TARGET=i686-elf
export PATH="$PREFIX/bin:$PATH"

# make build-binutils
mkdir -p build-binutils
cd build-binutils
../binutils-2.35/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot --disable-nls --disable-werror
make
make install

# make gcc
cd ~/src
mkdir -p build-gcc
cd build-gcc
../gcc-10.2.0/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --enable-languages=c,c++ --without-headers
make all-gcc
make all-target-libgcc
make install-gcc
make install-target-libgcc

# test gcc
$HOME/opt/cross/bin/$TARGET-gcc --version
