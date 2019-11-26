cmake_minimum_required(VERSION 3.7)

# sources
set(compiler_SOURCES
    main.cpp)

set(compiler_LIBS
    ir
    frontend
    backend)

# exec
add_executable(compiler ${compiler_SOURCES})
target_link_libraries(compiler ${compiler_LIBS})
target_include_directories(compiler PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# installation
install(TARGETS compiler DESTINATION bin)
