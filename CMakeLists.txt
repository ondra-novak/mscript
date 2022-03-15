cmake_minimum_required(VERSION 3.0) 
project (mscript)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/)

execute_process(COMMAND git submodule update --init)

include_directories(BEFORE src src/imtjson/src)
add_compile_options(-std=c++17)
add_compile_options(-Wall -Werror -Wno-noexcept-type)

add_subdirectory (src/imtjson/src/imtjson)
add_subdirectory (src/test)
add_subdirectory (src/mscript)

