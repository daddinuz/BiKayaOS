add_definitions(-DTARGET_UMPS=1)

set(UMPS_INCLUDE "${PROJECT_PATH}/include/umps")
set(UMPS_SOURCES "${PROJECT_PATH}/sources/umps")
set(UMPS_SCRIPTS "${PROJECT_PATH}/scripts/umps")

# C compiler setup
set(CMAKE_C_COMPILER mipsel-linux-gnu-gcc)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-pic -mips1 -mabi=32 -mfp32 -mno-gpopt -G 0 -nostdlib")

# WARN: this hack is needed to change the underlying linker
set(CMAKE_LINKER mipsel-linux-gnu-ld)
set(CMAKE_C_LINK_FLAGS "${CMAKE_C_LINK_FLAGS} -nostdlib -G 0 -T ${UMPS_SCRIPTS}/umpscore.ldscript")
set(CMAKE_C_LINK_EXECUTABLE "<CMAKE_LINKER> <CMAKE_C_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")

# lib target
include(${PROJECT_PATH}/cmake/lib.cmake)

# bin target
set(BIKAYA_LIBS ${ARCHIVE_NAME} ${UMPS_SOURCES}/crtso.o ${UMPS_SOURCES}/libumps.o)
include(${PROJECT_PATH}/cmake/bin.cmake)

add_custom_command(TARGET ${BIN_PHASE0} POST_BUILD COMMAND umps2-elf2umps -k ${BIN_PHASE0})
add_custom_command(TARGET ${BIN_PHASE1} POST_BUILD COMMAND umps2-elf2umps -k ${BIN_PHASE1})
add_custom_command(TARGET ${BIN_PHASE15} POST_BUILD COMMAND umps2-elf2umps -k ${BIN_PHASE15})
add_custom_command(TARGET ${BIN_PHASE2} POST_BUILD COMMAND umps2-elf2umps -k ${BIN_PHASE2})
add_custom_command(TARGET ${BIN_TEST_PCB} POST_BUILD COMMAND umps2-elf2umps -k ${BIN_TEST_PCB})
add_custom_command(TARGET ${BIN_HELLO_WORLD} POST_BUILD COMMAND umps2-elf2umps -k ${BIN_HELLO_WORLD})
add_custom_command(TARGET ${BIN_PRODUCER_CONSUMER} POST_BUILD COMMAND umps2-elf2umps -k ${BIN_PRODUCER_CONSUMER})
