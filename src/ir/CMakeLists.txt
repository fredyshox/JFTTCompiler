cmake_minimum_required(VERSION 3.7)

set(ir_SOURCES
  SymbolTable.cpp
  Operand.cpp
  Label.cpp
  Condition.cpp
  blocks/ControlFlowBlock.cpp
  blocks/BaseBlock.cpp
  blocks/ForLoopBlock.cpp
  blocks/ConditionBlock.cpp
  blocks/WhileLoopBlock.cpp
  blocks/ThreeAddressCodeBlock.cpp)

set(GLOB_RECURSE ir_HEADERS ./*.hpp)

add_library(ir
  STATIC
  ${ir_SOURCES})

target_include_directories(ir
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

install(TARGETS ir DESTINATION lib)
foreach(file ${ir_HEADERS})
  file(RELATIVE_PATH rel ${CMAKE_CURRENT_SOURCE_DIR} ${file})
  get_filename_component(dir ${rel} DIRECTORY)
  install(FILES ${file} DESTINATION include/${dir})
endforeach()


