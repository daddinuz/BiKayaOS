if (NOT DEFINED BIKAYA_LIBS)
    message(FATAL_ERROR "BIKAYA_LIBS is not defined.")
endif ()

set(BIN_PHASE0 kernel-phase0)
add_executable(${BIN_PHASE0} ${PROJECT_PATH}/sources/phase0.c)
target_link_libraries(${BIN_PHASE0} PRIVATE ${BIKAYA_LIBS})

set(BIN_PHASE1 kernel-phase1)
add_executable(${BIN_PHASE1} ${PROJECT_PATH}/sources/phase1.c)
target_link_libraries(${BIN_PHASE1} PRIVATE ${BIKAYA_LIBS})

set(BIN_PHASE15 kernel-phase15)
add_executable(${BIN_PHASE15} ${PROJECT_PATH}/sources/phase15.c ${PROJECT_PATH}/tests/p1.5test_bikaya_v0.c)
target_include_directories(${BIN_PHASE15} PRIVATE ${PROJECT_PATH}/tests)
target_link_libraries(${BIN_PHASE15} PRIVATE ${BIKAYA_LIBS})
