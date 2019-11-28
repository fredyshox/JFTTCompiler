cmake_minimum_required(VERSION 3.7)

# sources
set(backend_SOURCES
    Assembly.cpp
    AssemblyPrinter.cpp
    InstructionSelector.cpp)

set(GLOB_RECURSE backend_HEADERS ./*.hpp)

# export as library
add_library(backend
            STATIC
            ${backend_SOURCES})
target_include_directories(backend
                           PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(backend ir)

# installation
install(TARGETS backend DESTINATION lib)
foreach(file ${backend_HEADERS})
  file(RELATIVE_PATH rel ${CMAKE_CURRENT_SOURCE_DIR} ${file})
  get_filename_component(dir ${rel} DIRECTORY)
  install(FILES ${file} DESTINATION include/${dir})
endforeach()

