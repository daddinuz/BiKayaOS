if (NOT DEFINED BIKAYA_LIBS)
    message(FATAL_ERROR "BIKAYA_LIBS is not defined.")
endif ()

set(TESTS_PATH "${PROJECT_PATH}/tests")
set(BIN_PATH "${PROJECT_PATH}/kernels")

set(BIN_PHASE0 kernel-phase0)
add_executable(${BIN_PHASE0} ${BIN_PATH}/phase0.c)
target_link_libraries(${BIN_PHASE0} PRIVATE ${BIKAYA_LIBS})

set(BIN_PHASE1 kernel-phase1)
add_executable(${BIN_PHASE1} ${BIN_PATH}/phase1.c)
target_link_libraries(${BIN_PHASE1} PRIVATE ${BIKAYA_LIBS})

set(BIN_PHASE15 kernel-phase15)
add_executable(${BIN_PHASE15} ${BIN_PATH}/phase15.c ${TESTS_PATH}/p15test_bikaya.c)
target_include_directories(${BIN_PHASE15} PRIVATE ${TESTS_PATH})
target_link_libraries(${BIN_PHASE15} PRIVATE ${BIKAYA_LIBS})

set(BIN_PHASE2 kernel-phase2)
add_executable(${BIN_PHASE2} ${BIN_PATH}/phase2.c ${TESTS_PATH}/p2test_bikaya.c)
target_include_directories(${BIN_PHASE2} PRIVATE ${TESTS_PATH})
target_link_libraries(${BIN_PHASE2} PRIVATE ${BIKAYA_LIBS})

set(BIN_TEST_PCB kernel-test-pcb)
add_executable(${BIN_TEST_PCB} ${BIN_PATH}/test_pcb.c)
target_link_libraries(${BIN_TEST_PCB} PRIVATE ${BIKAYA_LIBS})

set(BIN_HELLO_WORLD kernel-hello-world)
add_executable(${BIN_HELLO_WORLD} ${BIN_PATH}/hello_world.c)
target_link_libraries(${BIN_HELLO_WORLD} PRIVATE ${BIKAYA_LIBS})

set(BIN_PRODUCER_CONSUMER kernel-producer-consumer)
add_executable(${BIN_PRODUCER_CONSUMER} ${BIN_PATH}/producer_consumer.c)
target_link_libraries(${BIN_PRODUCER_CONSUMER} PRIVATE ${BIKAYA_LIBS})
