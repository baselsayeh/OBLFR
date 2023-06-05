sdk_generate_library(trap_interceptor)
sdk_add_include_directories(include)
sdk_library_add_sources(${CMAKE_CURRENT_SOURCE_DIR}/vector.S)
