# BiKaya OS - OS Course project @ University of Bologna 2019

## Requirements (based on Ubuntu 18.04)

Install Git
```bash
sudo apt install git
```

Install compilers and build tools for our architectures
```bash
sudo apt install build-essential cmake libtool m4 automake autotools-dev autoconf gcc-arm-none-eabi gcc-mipsel-linux-gnu
```

Install Qt
```bash
sudo apt install qt5-default qt4-qmake qt4-dev-tools
```

Install elf utilities
```bash
sudo apt install libelf1 libelf-dev
```

Install boost libraries
```bash
sudo apt install libboost-all-dev
```

Install libsigc++
```bash
sudo apt install libsigc++-2.0-dev
```

Install uARM emulator
```bash
cd emulators
git clone https://github.com/mellotanica/uARM.git
cd uARM
./compile
sudo ./install.sh
cd ..
```

Install uMPS2 emulator
```bash
git clone https://github.com/tjonjic/umps.git
cd umps
autoreconf -vfi
QT_SELECT=qt4 ./configure --enable-maintainer-mode --with-mips-tool-prefix=mipsel-linux-gnu-
make
sudo make install
```

## Building

#### uARM targets

At the root of the project:

```bash
cd build/uarm/debug
cmake -DTARGET_ARCH=uARM ../../..
make
```

#### uMPS targets

At the root of the project:

```bash
cd build/umps/debug
cmake -DTARGET_ARCH=uMPS ../../..
make
```

## Setup default terms and printers

At the root of the project:

```bash
touch emulators/config/uarm/{term0,printer0}.uarm
touch emulators/config/umps/{term0,printer0}.umps
```
