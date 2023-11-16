add_executable(basic basic.cpp)
target_link_libraries(basic PRIVATE serialize)

add_executable(aggregate aggregate.cpp)
target_link_libraries(aggregate PRIVATE serialize)

add_executable(member member.cpp)
target_link_libraries(member PRIVATE serialize)

add_executable(static static.cpp)
target_link_libraries(static PRIVATE serialize)

add_executable(type_safe type_safe.cpp)
target_link_libraries(type_safe PRIVATE serialize)

add_executable(custom custom.cpp)
target_link_libraries(custom PRIVATE serialize)

if("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU" OR "${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
  target_compile_options(basic PRIVATE -Wall -Wextra)
  target_compile_options(aggregate PRIVATE -Wall -Wextra)
  target_compile_options(member PRIVATE -Wall -Wextra)
  target_compile_options(static PRIVATE -Wall -Wextra)
  target_compile_options(type_safe PRIVATE -Wall -Wextra)
  target_compile_options(custom PRIVATE -Wall -Wextra)
endif()
