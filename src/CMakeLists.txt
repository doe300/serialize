
add_library(serialize
  bit_packing.cpp
  byte_packing.cpp
  common.cpp
  simple.cpp
  type_safe.cpp
)
target_include_directories(serialize PUBLIC ../include/)
target_compile_options(serialize PRIVATE -Wall -Wextra)

if(SERIALIZE_BUILD_COVERAGE)
	target_compile_options(serialize PUBLIC --coverage)
	target_link_options(serialize PUBLIC --coverage)
endif(SERIALIZE_BUILD_COVERAGE)
