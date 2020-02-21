if (NOT DEFINED BIKAYA_LIBS)
    message(FATAL_ERROR "BIKAYA_LIBS is not defined.")
endif ()

set(BIN_PHASE0 kernel-phase0)
add_executable(${BIN_PHASE0} ${PROJECT_PATH}/sources/phase0.c)
target_link_libraries(${BIN_PHASE0} PRIVATE ${BIKAYA_LIBS})

set(BIN_PHASE1 kernel-phase1)
add_executable(${BIN_PHASE1} ${PROJECT_PATH}/sources/phase1.c)
target_link_libraries(${BIN_PHASE1} PRIVATE ${BIKAYA_LIBS})
