add_definitions(-DTARGET_UARM=1 -DUARM_MACHINE_COMPILING=1)

set(UARM_INCLUDE "${PROJECT_PATH}/include/uarm")
set(UARM_SOURCES "${PROJECT_PATH}/sources/uarm")
set(UARM_SCRIPTS "${PROJECT_PATH}/scripts/uarm")

# C compiler setup
set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mcpu=arm7tdmi -nostdlib -Xlinker -G -Xlinker 0 -Xlinker -T -Xlinker ${UARM_SCRIPTS}/elf32ltsarm.h.uarmcore.x")

# lib target
include(${PROJECT_PATH}/cmake/lib.cmake)

# bin targets
set(BIKAYA_LIBS ${ARCHIVE_NAME} ${UARM_SOURCES}/libuarm.o ${UARM_SOURCES}/crtso.o ${UARM_SOURCES}/libdiv.o)
include(${PROJECT_PATH}/cmake/bin.cmake)
