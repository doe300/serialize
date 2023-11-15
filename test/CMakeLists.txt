
include(../cmake/cpptest_lite.cmake)

add_executable(test_serialize
  test_bit_packing.cpp
  test_byte_packing.cpp
  test_main.cpp
  test_simple.cpp
  test_type_safe.cpp
)
target_link_libraries(test_serialize PRIVATE serialize cpptest-lite)
target_compile_options(test_serialize PRIVATE -Wall -Wextra)

add_executable(test_processes test_processes.cpp)
target_link_libraries(test_processes PRIVATE serialize cpptest-lite)
target_compile_options(test_processes PRIVATE -Wall -Wextra)
target_compile_definitions(test_processes PRIVATE TEST_FILES_PATH="${CMAKE_BINARY_DIR}/Testing")

include(${cpptest-lite_SOURCE_DIR}/cmake/CppTest.cmake)
cpptest_discover_tests(test_serialize)
add_test(
  NAME TestProcesses
  COMMAND ${CMAKE_COMMAND} -DTEST_PROGRAM=$<TARGET_FILE:test_processes> -DTEST_FOLDER=${CMAKE_BINARY_DIR}/Testing -P ${CMAKE_CURRENT_SOURCE_DIR}/test_processes.cmake
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
