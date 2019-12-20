add_definitions(-DTARGET_UARM=1)

if (NOT DEFINED UARM_PATH)
	set(UARM_PATH "/usr/include/uarm")
endif ()

include_directories(${UARM_PATH})

# C compiler setup
set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mcpu=arm7tdmi -nostdlib -Xlinker -G -Xlinker 0 -Xlinker -T -Xlinker ${UARM_PATH}/ldscripts/elf32ltsarm.h.uarmcore.x")

# lib target
include(${PROJECT_PATH}/cmake/lib.cmake)

# bin target
set(BIN_NAME kernel)
add_executable(${BIN_NAME} ${PROJECT_PATH}/sources/main.c)
target_link_libraries(${BIN_NAME} PRIVATE ${ARCHIVE_NAME} ${UARM_PATH}/libuarm.o ${UARM_PATH}/crtso.o ${UARM_PATH}/libdiv.o)
