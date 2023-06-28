cmake_minimum_required(VERSION 3.25)
project(test_lua LANGUAGES C)

add_executable(main main.c)

add_subdirectory(LuaJIT)

include_directories(${PROJECT_SOURCE_DIR}/LuaJIT/src)
include_directories(${PROJECT_SOURCE_DIR}/LuaJIT/host)


target_link_libraries(main liblua-shared)

