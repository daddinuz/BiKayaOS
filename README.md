# BiKaya OS - OS Course project @ University of Bologna 2019

## Requirements

Install Git
`bash
sudo apt install git
`

Install compilers for our architectures
`bash
sudo apt install build-essential gcc-arm-none-eabi gcc-mipsel-linux-gnu
`

Install Qt
`bash
sudo apt install qt5-default
`

Install elf utilities
`bash
sudo apt install libelf1 libelf-dev
`

Install boost libraries
`bash
sudo apt install libboost-all-dev
`

Install uARM emulator
`bash
cd ./emulators
git clone https://github.com/mellotanica/uARM.git
cd uArm
./compile
./install.sh
`

