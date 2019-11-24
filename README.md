# Compiler

To save stuff from the first version, we gonna keep AST IR
and translate it to new IR in the additional pass. (still frontend)

Communication project:
frontend -> shared IR -> backend

## IR Project
Hybrid IR - control flow graph with basic blocks of three address codes.

Control flow blocks:
If<condition>Else
* * Condition struct
* * two child nodes
ForLoop
* * iterator - initial value, target value downto or to
* * computed condition from iterator value
* * child node
While
* * Condition struct
* * child node
DoWhile
* * same as while, but with one run without condition

Three address codes:
- IO operations
- Assignments

## Semantic Analysis

Types:
- int cannot be indexed
- array must be indexed
- array bounds (l:u) l <= u


Identifiers:
- identifier must be in declaration list, except for iterators

Iterator:
- iterator is immutable (cannot be: lvalue in assignment or operand in read)


# Old stuff

## TODO list
* lexer
* parser

## Development stack
* plain C with libCoreFoundation, bison, flex
* immediate representation bridgable to swift
* swift for further optimalizations
* swift for machine code generation
* swift for user interface  
* so C frontend and Swift backend

## IR
Symbol table:
* support for symbol - name, value
* support for blocks - local variables (local contexts)

Type controller:
* identifier - has to exist and be available in given context
* declaration - avoid redeclaration
* available types - natual number, arrays
* arrays has bounds - startIndex, endIndex - endIndex >= startIndex
* arithmetic operators - lhs and rhs must have supported type
* comparison operators - same as ^
* for-loop iterator is constant

Three address code or higher level ir?

Architecture:
Graph nodes - represent block of code and has references to outcomes of its operation
* * plain node with 1-child
* * composite node with n-children

Declaration
* * identifier for both number and array types
* * bounds for array type
* * multiple declarations in single block

Identifier
* * single identifier for number type
* * identifier with index for array type

Value
* * number or identifier

Expression - operator, two operands
* * arithmetic operator
* * operands
Condition - operator, two operands
* * logical operator
* * operands

Commands:
If<condition>Else
* * Condition struct
* * two child nodes
ForLoop
* * iterator - initial value, target value downto or to
* * computed condition from iterator value
* * child node
While
* * Condition struct
* * child node
DoWhile
* * same as while, but with one run without condition
Read
* * identifier - where read value will be saved
Write
* * value
Assignment
* * identifier
* * expression to be assigned

## Register allocation:
### Members:
Implement Hashable to produce the same hashValue when alternation
Implement Equatable to lhs.hashValue == rhs.hashValue
