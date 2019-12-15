set(ARCHIVE_NAME bikaya)
set(ARCHIVE_INCLUDE ${PROJECT_PATH}/include)
set(ARCHIVE_SOURCES ${PROJECT_PATH}/sources)

include_directories(${ARCHIVE_INCLUDE})

add_library(${ARCHIVE_NAME}
  # headers
  ${ARCHIVE_INCLUDE}/system.h
  ${ARCHIVE_INCLUDE}/term.h
  ${ARCHIVE_INCLUDE}/printer.h
  # sources
  ${ARCHIVE_SOURCES}/term.c
  ${ARCHIVE_SOURCES}/printer.c
)