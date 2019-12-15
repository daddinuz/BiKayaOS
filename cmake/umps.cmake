add_definitions(-DTARGET_UMPS=1)

if (NOT DEFINED UMPS_OBJ_PATH)
    set(UMPS_OBJ_PATH "/usr/local/lib/umps2")
endif ()

if (NOT DEFINED UMPS_LINKER_SCRIPT_FILE)
    set(UMPS_LINKER_SCRIPT_FILE "/usr/local/share/umps2/umpscore.ldscript")
endif ()

if (NOT DEFINED UMPS_INCLUDE_PATH)
    set(UMPS_INCLUDE_PATH "/usr/local/include/umps2/umps")
endif ()

include_directories(${UMPS_INCLUDE_PATH})

# C compiler setup
set(CMAKE_C_COMPILER mipsel-linux-gnu-gcc)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-pic -mips1 -mabi=32 -mfp32 -mno-gpopt -G 0")

# WARN: this hack is needed to change the underlying linker
set(CMAKE_LINKER mipsel-linux-gnu-ld)
set(CMAKE_C_LINK_FLAGS "${CMAKE_C_LINK_FLAGS} -nostdlib -G 0 -T ${UMPS_LINKER_SCRIPT_FILE}")
set(CMAKE_C_LINK_EXECUTABLE "<CMAKE_LINKER> <CMAKE_C_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")

# lib target
include(${PROJECT_PATH}/cmake/lib.cmake)

# bin target
set(BIN_NAME kernel)
add_executable(${BIN_NAME} ${PROJECT_PATH}/sources/main.c)
target_link_libraries(${BIN_NAME} PRIVATE ${ARCHIVE_NAME} ${UMPS_OBJ_PATH}/crtso.o ${UMPS_OBJ_PATH}/libumps.o)
add_custom_command(TARGET ${BIN_NAME} POST_BUILD COMMAND umps2-elf2umps -k ${BIN_NAME})
