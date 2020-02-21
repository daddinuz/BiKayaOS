cmake_minimum_required(VERSION 3.10)
project(BiKayaOS C)

set(CMAKE_C_VERSION 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Werror")
set(PROJECT_PATH ${CMAKE_CURRENT_LIST_DIR})

option(DEBUG "Debug mode" on)

if (NOT DEBUG)
    add_definitions(-DNDEBUG)
endif ()

if (${TARGET_ARCH} STREQUAL "uARM")
	include(${PROJECT_PATH}/cmake/uarm.cmake)
elseif (${TARGET_ARCH} STREQUAL "uMPS")
	include(${PROJECT_PATH}/cmake/umps.cmake)
else ()
	message(FATAL_ERROR "Unknown arch: ${TARGET_ARCH}")
endif ()
