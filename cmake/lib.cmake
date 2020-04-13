set(ARCHIVE_NAME bikaya)
set(ARCHIVE_INCLUDE ${PROJECT_PATH}/include)
set(ARCHIVE_SOURCES ${PROJECT_PATH}/sources)

include_directories(${ARCHIVE_INCLUDE})

add_library(${ARCHIVE_NAME}
  ${ARCHIVE_SOURCES}/primitive_types.c
  ${ARCHIVE_SOURCES}/assertions.c
  ${ARCHIVE_SOURCES}/helpers.c
  ${ARCHIVE_SOURCES}/memory.c
  ${ARCHIVE_SOURCES}/printer.c
  ${ARCHIVE_SOURCES}/term.c
  ${ARCHIVE_SOURCES}/asl.c
  ${ARCHIVE_SOURCES}/pcb.c
  ${ARCHIVE_SOURCES}/scheduler.c
  ${ARCHIVE_SOURCES}/handlers.c
  ${ARCHIVE_SOURCES}/core.c
)
