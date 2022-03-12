cmake_minimum_required(VERSION 3.0) 

add_executable (mscript_cli main.cpp )
target_link_libraries (mscript_cli LINK_PUBLIC mscript imtjson stdc++fs pthread)

