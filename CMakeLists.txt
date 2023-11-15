cmake_minimum_required(VERSION 3.17)

project(serialize)

# Option whether to enable code coverage analysis via gcov
option(SERIALIZE_BUILD_COVERAGE "Enables collection of code coverage via gcov" OFF)
option(SERIALIZE_BUILD_CLANG_TIDY "Enables running cang-tidy while building" OFF)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(SERIALIZE_BUILD_CLANG_TIDY)
  set(CMAKE_CXX_CLANG_TIDY clang-tidy)
else()
  unset(CMAKE_CXX_CLANG_TIDY)
endif()

add_subdirectory(src)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  include(CTest)
endif()
if((CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME OR SERIALIZE_BUILD_TESTING) AND BUILD_TESTING)
  add_subdirectory(examples)
  add_subdirectory(test)
endif()
