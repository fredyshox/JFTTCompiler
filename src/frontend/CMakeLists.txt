cmake_minimum_required(VERSION 3.7)

# bison
if (DEFINED BISON_PATH)
  set(bison_NAME ${BISON_PATH})
else()
  set(bison_NAME bison)
endif()

find_program(bison_EXECUTABLE ${bison_NAME}) #/usr/local/Cellar/bison/3.2.2/bin/bison)
if (bison_EXECUTABLE EQUAL bison_EXECUTABLE-NOTFOUND)
  message(FATAL_ERROR "bison is required (>= 3.2.2)")
endif()
message(STATUS "bison path ${bison_EXECUTABLE}")

set(bison_OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/parser.c)
set(bison_HEADER ${CMAKE_CURRENT_BINARY_DIR}/parser.h)
add_custom_command(
  OUTPUT ${bison_OUTPUT} ${bison_HEADER}
  COMMAND ${bison_EXECUTABLE}
  --defines=${bison_HEADER}
  -o ${bison_OUTPUT}
  ${CMAKE_CURRENT_SOURCE_DIR}/parser.y
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/parser.y
  COMMENT "Generating parser.c with bison..."
)

# flex
if (DEFINED FLEX_PATH)
  set(flex_NAME ${FLEX_PATH})
else()
  set(flex_NAME flex)
endif()

find_program(flex_EXECUTABLE ${flex_NAME})
if (flex_EXECUTABLE EQUAL flex_EXECUTABLE-NOTFOUND)
  message(FATAL_ERROR "flex is required (>= 2.5)")
endif()
message(STATUS "flex path ${flex_EXECUTABLE}")

set(flex_OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/tokenize.c)
add_custom_command(
  OUTPUT ${flex_OUTPUT}
  COMMAND ${flex_EXECUTABLE}
  -o ${flex_OUTPUT}
  ${CMAKE_CURRENT_SOURCE_DIR}/tokenize.l
  DEPENDS ${bison_HEADER} ${CMAKE_CURRENT_SOURCE_DIR}/tokenize.l
  COMMENT "Generating tokenize.c with flex..."
)

# set properties of generated files
set_source_files_properties(${flex_OUTPUT} ${bison_OUTPUT} ${bison_HEADER} GENERATED)

# define compile sources
set(frontend_SOURCES
    parser.c
    tokenize.c
    frontend.c
    ast/program.c
    ast/node.c
    ast/assignment.c
    ast/io.c
    ast/branch.c
    ast/loops.c
    ast/expression.c
    ast/operand.c
    ast/condition.c
    utility/Converter.cpp
    utility/SemanticAnalysis.cpp
    utility/ASTOptimizer.cpp 
    ${bison_OUTPUT}
    ${flex_OUTPUT})

# define headers
file(GLOB_RECURSE frontend_HEADERS
     ./*.h)
set(generated_HEADERS
    ${bison_HEADER})

message(STATUS "headers: ${frontend_HEADERS}")
message(STATUS "sources: ${frontend_SOURCES}")

# export header dicectories variables
set(frontend_GEN_HEADER_SEARCH_PATHS
    ${CMAKE_CURRENT_BINARY_DIR}
    PARENT_SCOPE)

set(frontend_HEADER_SEARCH_PATHS
    ${CMAKE_CURRENT_SOURCE_DIR}
    PARENT_SCOPE)

# export as library
add_library(frontend
            STATIC
            ${frontend_SOURCES})
target_include_directories(frontend
                           PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
                           PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(frontend ir)

# installation
install(TARGETS frontend DESTINATION lib)
foreach(file ${frontend_HEADERS})
  file(RELATIVE_PATH rel ${CMAKE_CURRENT_SOURCE_DIR} ${file})
  get_filename_component(dir ${rel} DIRECTORY)
  install(FILES ${file} DESTINATION include/${dir})
endforeach()
install(FILES ${generated_HEADERS} DESTINATION include)
