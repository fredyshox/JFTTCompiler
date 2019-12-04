cmake_minimum_required(VERSION 3.7)

project(jftt-compiler)

# compiler options
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -std=c++17 -Wall -Wextra -pedantic -stdlib=libc++ -Wno-keyword-macro -frtti")
set(CMAKE_C_FLAGS "-g")

add_subdirectory(src/frontend)
add_subdirectory(src/backend)
add_subdirectory(src/ir)
add_subdirectory(src/compiler)

enable_testing()
add_subdirectory(test)
