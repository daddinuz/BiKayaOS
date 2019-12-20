set(ARCHIVE_NAME bikaya)
set(ARCHIVE_INCLUDE ${PROJECT_PATH}/include)
set(ARCHIVE_SOURCES ${PROJECT_PATH}/sources)

include_directories(${ARCHIVE_INCLUDE})

add_library(${ARCHIVE_NAME}
  ${ARCHIVE_SOURCES}/term.c
  ${ARCHIVE_SOURCES}/printer.c
  ${ARCHIVE_SOURCES}/helpers.c
  ${ARCHIVE_SOURCES}/primitive_types.c
)
