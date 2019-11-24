cmake_minimum_required(VERSION 3.7)

macro(add_test_exec target source libs)
    add_executable("test_${target}"
            ${source})

    target_link_libraries("test_${target}"
            ${libs})
    target_link_libraries("test_${target}" ${GTEST_LIBRARY})

    target_include_directories("test_${target}"
            PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_include_directories("test_${target}" PRIVATE ${GTEST_INCLUDE_DIRS})

    install(TARGETS "test_${target}" DESTINATION test)
endmacro()

macro(add_unit_test target)
    add_test(NAME "test_${target}"
            COMMAND "test_${target}"
            WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
endmacro()

find_package(GTest 1.8.1 REQUIRED)
if (GTEST_FOUND)
    message(STATUS "GTest 1.8.1 found")
else()
    message(FATAL_ERROR "GTest not found!")
endif()

add_test_exec(parser
        frontend/ParserTest.cpp
        frontend)
add_unit_test(parser)

add_test_exec(lexer
        frontend/LexerTest.cpp
        frontend)
add_unit_test(lexer)

add_test_exec(converter
        frontend/ConverterTest.cpp
        frontend)
add_unit_test(converter)

add_test_exec(semantic_analysis
        frontend/SemanticAnalysisTest.cpp
        frontend)
add_unit_test(semantic_analysis)

add_test_exec(ast_optimizer
        frontend/ASTOptimizerTest.cpp
        frontend)
add_unit_test(ast_optimizer)

add_test_exec(symbol_table
        ir/SymbolTableTest.cpp
        ir)
add_unit_test(symbol_table)

add_test_exec(tac_flatten
        ir/TACFlatten.cpp
        ir)
add_unit_test(tac_flatten)

add_test_exec(tac_simplify
        backend/TACSimplify.cpp
        backend)
add_unit_test(tac_simplify)

add_test_exec(isa_match
        backend/ISAMatch.cpp
        backend)
