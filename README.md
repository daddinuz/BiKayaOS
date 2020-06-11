# BiKayaOS

BiKayaOS is an operating system with educational purposes, 
developed during the course of "Sistemi Operativi" at 
Alma Mater Studiorum University of Bologna, whose main 
target is portability among multiple architectures, in 
particular uARM and uMPS2.

##### Authors

Luca Borghi, Stefano Cremona, Davide Di Carlo, Andrea Segantini.

### Table of Contents

- [Project Structure](#project-structure)
- [Modules Overview](#modules-overview)
- [Design Choices and Encountered Problems](#design-choices-and-encountered-problems)
- [Build and Setup](#build-and-setup)

## Project Structure

```
BiKayaOS/
├── build/            # output of compilation per architecture.
│   ├── uarm/
│   └── umps/
├── cmake/
│   ├── bin.cmake     # build sources inside kernels/
│   ├── lib.cmake     # libbikaya build instructions
│   ├── uarm.cmake    # uARM specific build instructions
│   └── umps.cmake    # uMPS2 specific build instructions
├── CMakeLists.txt    # cmake entry point
├── emulators/
│   └── config/       # config files for each architecture
│       ├── uarm/
│       └── umps/
├── include/          # BiKaya header files
│   ├── uarm/         # uARM specific header files
│   └── umps/         # uMPS2 specific header files
├── kernels/          # source files of kernel executables
├── run.sh            # helper script for building and running
├── sources/          # BiKaya source files
│   ├── uarm/         # uARM specific .o files
│   └── umps/         # uMPS2 specific .o files
└── tests/            # tests' source files
```

## Modules Overview

- **core.h/.c**

Provides low level facilities which consist in a first layer 
of abstraction from the underlying archicture.
These facilities comprehend: CPU, devices, memory and time related stuff, 
and in general arch-dependant stuff.

- **pcb.h/.c & asl.h/.c**

Process Control Block and Active Semaphore List provide low level facilities 
on which the scheduler relies upon to manage processes and their synchronization.

- **scheduler.h/.c**

Actor that integrates the underlying levels, in particular 
schedules and synchronizes processes. Also offers various services to handlers.
It's the only module that uses pcb and asl.

- **handlers.h/.c**

Handlers provides system calls to the user and 
handles interrupts and various exceptions.

## Design Choices and Encountered Problems

#### architectures abstraction

The main target of BiKaya is portability between architectures.
We designed a layer (namely core module) that abstracts from the underlying architecture.
By using this layer which is a architecture-independent interface, the obtained code
can be easily ported on different architectures. In order to reach this goal,
we delegated specific architecture-dependant code inside the implementation of this module.

#### readyQueue optimization

Initially, we iterated the queue from the first element to the last.
Later we realized that we could do an optimization: iterating the
queue from the last element to the first, we get 2 benefits for the readyQueue.
1) Since we insert the new process at the end of processes with its same priority,
   the insertion takes place immediately without iterating all processes of equal priority.
2) Since a process usually takes more than one timeslice to terminate, if in the readyQueue
   there are enough processes waiting, after some timeslices the insertion tends to be
   at the end of the readyQueue, since the priority of the waiting processes will be much 
   more higher than that of the process to be inserted.

#### process stack pointer assignment

By specifications, in the scheduling of a process, its stack pointer should be assigned
in relation to its original priority; however, this causes two or more processes with
the same original priority to have the same stack frame. This can lead to problems.
To avoid this situation, we assign the stack pointer basing on an identifier that we
guarantee to be unique for each process. This mechanism is not exploited by the syscall
CREATEPROCESS (where the caller must specify the process state and so its stack pointer).

#### aging of processes blocked on semaphore queues

In order to manage processes aging in the readyqueue, each time we launch a process we increase
the priority of processes in the ready queue. However, this mechanism would be quite
expensive to implement for the processes stuck in the semaphore queues, therefore
we used a different mechanism: we defined a global age that increases every time a process
is launched. When a process gets stuck on the queue of a semaphore, we sample the global age value.
When the process needs to be unlocked (and then put back into the ready queue), we update its priority
according to how much the global age has increased.

#### terminating a process

Since the termination of a process causes the termination of its progeny and since it is
possible that a given process terminates an arbitrary process, then it is possible that
a process, in terminating another process, also terminates itself (i.e. if it is part of the
dynasty of the process killed). Therefore, since it is not known if the current process will be terminated
before calling the syscall TERMINATEPROCESS, we provisionally insert the current process at the top
of the readyQueue and, once the dynasty of the process to be removed is cut out and terminated
with also the process itself, if the current process does not stand in the readyQueue anymore, then
we launch the process at the top of the readyQueue, otherwise we resume the execution of the current process.

#### verhogen for a killed process while in a critical section

When a process is terminated, it may be in a critical section and therefore it does not perform a verhogen
for that critical section. In this circumstance, a couple of solutions have been explored:

1) adding a field in the process descriptor denoting the critical section where a given process is working;
   however a process can be in multiple critical sections, thus this solution would not be sufficient
   to manage the problem;
2) using a table for each process that denotes in which critical sections the process is working;
   however, in case that the number of critical sections of a process exceeds the number of critical
   sections managed by the table, the problem would be solved only partially and moreover this solution
   would be quite inefficient.

Therefore, we have delegated the responsibility to the user not to terminate processes that are in
a critical section.

#### time handling

Each process has information regarding its execution times; these times are:

- **user time**: time that the process takes in carrying out its own routines; 
- **kernel time**: time the process takes to execute system routines;
- **wallclock time**: total time since the process was launched for the first time.

These times get updated in three particular cases:
- when the GETCPUTIME syscall is called by the user;
- every time an interrupt is raised;
- when a process time slice ends;

The wallclock time for a process is calculated only when the GETCPUTIME syscall is called, by substracting
the TODLow sampled the first time the process is launched from the current value of the TODLow.
Whenever the execution of a process is interrupted by the execution of a handler, at the beginning of the
execution of the handler we sample the time remaining at the end of the process time slice and we reset the
timer to the highest value it can assume. At the end of the execution of the handler we calculate the time
that was taken to execute it and update the kernel time accordingly. We also reset the timer to the value
we sampled upon entering the handler.
To update the user time, we save the timer value at the exit of the last handler, then when we enter the
next handler, we calculate the elapsed time.

## Build and Setup

#### Requirements (based on Ubuntu 18.04)

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

#### Setup default terminals and printers

At the root of the project:

```bash
touch emulators/config/uarm/{term0,printer0}.uarm
touch emulators/config/umps/{term0,printer0}.umps
```

#### Building

You can use the `run.sh` script to easily build and run, otherwise 
you can build manually following the instructions below.

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
