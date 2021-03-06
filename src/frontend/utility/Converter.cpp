//
// Created by Kacper Raczy on 2019-09-09.
//

#include "Converter.hpp"
#include "blocks/MultiplicationBlock.hpp"
#include "blocks/DivisionBlock.hpp"
#include "blocks/RemainderBlock.hpp"
#include <string>
#include <iostream>

BaseBlock* irconverter::convertList(ASTNode* node, SymbolTable *parentTable) {
    BaseBlock* block = nullptr;
    BaseBlock* startBlock = nullptr;
    BaseBlock* prevBlock = nullptr;
    ASTNode* current = node;

    if (current != nullptr) {
        startBlock = convert(*current, parentTable);
        prevBlock = startBlock;
        current = current->next;
    }

    while (current != nullptr) {
        block = convert(*current, parentTable);
        prevBlock->setNext(block);
        prevBlock = block;
        current = current->next;
    }

    return startBlock;
}

BaseBlock* irconverter::convert(ASTNode node, SymbolTable* parentTable) {
    BaseBlock* block = nullptr;
    if (node.type == kNodeAssignment) {
        auto* command = (ASTAssignment*) node.command;
        block = convert(*command, parentTable);
    } else if (node.type == kNodeBranch) {
        auto* command = (ASTBranch*) node.command;
        block = convert(*command, parentTable);
    } else if (node.type == kNodeIO) {
        auto* command = (ASTIO*) node.command;
        block = convert(*command);
    } else if (node.type == kNodeWhileLoop) {
        auto* command = (ASTWhileLoop*) node.command;
        block = convert(*command, parentTable);
    } else if (node.type == kNodeForLoop) {
        auto* command = (ASTForLoop*) node.command;
        block = convert(*command, parentTable);
    }

    return block;
}

ConditionBlock* irconverter::convert(ASTBranch branch, SymbolTable* parentTable) {
    Condition cond = convert(branch.condition);
    auto* block = new ConditionBlock(cond, nullptr, nullptr);
    block->localSymbolTable().setParent(parentTable);

    BaseBlock* passBody = convertList(branch.ifNode, &block->localSymbolTable());
    BaseBlock* failBody = convertList(branch.elseNode, &block->localSymbolTable());
    block->setPassBody(passBody);
    block->setFailBody(failBody);

    return block;
}

ForLoopBlock* irconverter::convert(ASTForLoop forLoop, SymbolTable* parentTable) {
    int step = (forLoop.type == kLoopForTo) ? 1 : -1;
    auto init = convert(forLoop.iteratorInitial);
    auto bound = convert(forLoop.iteratorBound);
    LoopRange range(init, bound, step);
    ForLoopBlock* block = new ForLoopBlock(forLoop.iterator, range, parentTable);
    block->setBody(convertList(forLoop.start, &block->localSymbolTable()));

    return block;
}

WhileLoopBlock* irconverter::convert(ASTWhileLoop whileLoop, SymbolTable* parentTable) {
    Condition cond = convert(whileLoop.condition);
    WhileLoopBlock* block =
            (whileLoop.type == kLoopWhile) ? new WhileLoopBlock(cond, parentTable) : new DoWhileLoopBlock(cond, parentTable);
    block->localSymbolTable().setParent(parentTable);
    block->setBody(convertList(whileLoop.start, &block->localSymbolTable()));

    return block;
}

ThreeAddressCodeBlock* irconverter::convert(ASTIO io) {
    auto sourceOrDest = convert(io.operand);
    auto v1 = std::make_unique<VirtualRegisterOperand>(1);
    if (io.type == kIOReadCommand) {
        std::list<ThreeAddressCode> instructionList {
            ThreeAddressCode(v1->copy(), ThreeAddressCode::Operator::STDIN),
            ThreeAddressCode(std::move(sourceOrDest), ThreeAddressCode::Operator::LOAD, v1->copy())
        };
        return new ThreeAddressCodeBlock(instructionList);
    } else { // kIOWriteCommand
        std::list<ThreeAddressCode> instructionList {
            ThreeAddressCode(v1->copy(), ThreeAddressCode::Operator::LOAD, std::move(sourceOrDest)),
            ThreeAddressCode(v1->copy(), ThreeAddressCode::Operator::STDOUT),
        };
        return new ThreeAddressCodeBlock(instructionList);
    }
}

bool isPowerOfTwo(int64_t n) {
    return (n != 0) && ((n & (n - 1)) == 0);
}

int64_t integerlog2(int64_t n) {
    int64_t res = 0;
    while (n >>= 1) ++res;

    return res;
}

BaseBlock* optimalMultiplication(ASTExpression expr,
                                 std::unique_ptr<Operand> dest,
                                 std::unique_ptr<Operand> op1,
                                 std::unique_ptr<Operand> op2,
                                 SymbolTable* parentTable) {
    if (expr.operand1.type == kOperandConstant && isPowerOfTwo(expr.operand1.constant)) {
        int64_t constant = expr.operand1.constant;
        ConstantOperand shiftVal(integerlog2(abs(constant)));

        ThreeAddressCodeBlock signAdjust;
        if (constant < 0) {
            ConstantOperand c0(0);
            signAdjust = ThreeAddressCodeBlock::subtraction(*dest, c0, *dest, 5);
        }

        ThreeAddressCodeBlock block = ThreeAddressCodeBlock::binaryOperation(std::move(dest),
                                                                             std::move(op2),
                                                                             shiftVal.copy(),
                                                                             ThreeAddressCode::LSHIFT,
                                                                             1);
        block.merge(signAdjust);
        op1.reset();
        return new ThreeAddressCodeBlock(block);
    } else if (expr.operand2.type == kOperandConstant && isPowerOfTwo(expr.operand2.constant)) {
        int64_t constant = expr.operand2.constant;
        ConstantOperand shiftVal(integerlog2(abs(constant)));

        ThreeAddressCodeBlock signAdjust;
        if (constant < 0) {
            ConstantOperand c0(0);
            signAdjust = ThreeAddressCodeBlock::subtraction(*dest, c0, *dest, 5);
        }

        ThreeAddressCodeBlock block = ThreeAddressCodeBlock::binaryOperation(std::move(dest),
                                                                             std::move(op1),
                                                                             shiftVal.copy(),
                                                                             ThreeAddressCode::LSHIFT,
                                                                             1);
        block.merge(signAdjust);
        op2.reset();
        return new ThreeAddressCodeBlock(block);
    } else {
        return new MultiplicationBlock(std::move(dest), std::move(op1), std::move(op2), parentTable);
    }
}

BaseBlock* optimalDivision(ASTExpression expr,
                           std::unique_ptr<Operand> dest,
                           std::unique_ptr<Operand> op1,
                           std::unique_ptr<Operand> op2,
                           SymbolTable* parentTable) {
    if (expr.operand2.type == kOperandConstant && isPowerOfTwo(expr.operand2.constant)) {
        int64_t constant = expr.operand2.constant;
        ConstantOperand shiftVal(-1 * integerlog2(abs(constant)));

        ThreeAddressCodeBlock signAdjust;
        if (constant < 0) {
            ConstantOperand c0(0);
            signAdjust = ThreeAddressCodeBlock::subtraction(*dest, c0, *dest, 5);
        }

        ThreeAddressCodeBlock block = ThreeAddressCodeBlock::binaryOperation(std::move(dest),
                                                                             std::move(op1),
                                                                             shiftVal.copy(),
                                                                             ThreeAddressCode::LSHIFT,
                                                                             1);
        block.merge(signAdjust);
        op2.reset();
        return new ThreeAddressCodeBlock(block);
    } else {
        return new DivisionBlock(std::move(dest), std::move(op1), std::move(op2), parentTable);
    }
}

BaseBlock* irconverter::convert(ASTAssignment assignment, SymbolTable* parentTable) {
    auto dest = convert(assignment.symbol);
    if (assignment.rtype == kRTypeExpression) {
        ASTExpression expr = assignment.expression;
        auto op1 = convert(expr.operand1);
        auto op2 = convert(expr.operand2);
        auto tacOperator = convert(expr.op);


        if (tacOperator == ThreeAddressCode::Operator::MUL) {
            return optimalMultiplication(expr, std::move(dest), std::move(op1), std::move(op2), parentTable);
        } else if (tacOperator == ThreeAddressCode::Operator::DIV) {
            return optimalDivision(expr, std::move(dest), std::move(op1), std::move(op2), parentTable);
        } else if (tacOperator == ThreeAddressCode::Operator::MOD) {
            return new RemainderBlock(std::move(dest), std::move(op1), std::move(op2), parentTable);
        }

        ThreeAddressCodeBlock binExpr =
                ThreeAddressCodeBlock::binaryOperation(std::move(dest), std::move(op1), std::move(op2), tacOperator, 1);
        return new ThreeAddressCodeBlock(binExpr);
    } else { //kRTypeOperand
        auto op1 = convert(assignment.operand);
        auto v1 = std::make_unique<VirtualRegisterOperand>(1);
        std::list<ThreeAddressCode> instructionList {
            ThreeAddressCode(v1->copy(), ThreeAddressCode::Operator::LOAD, std::move(op1)),
            ThreeAddressCode(std::move(dest), ThreeAddressCode::Operator::LOAD, v1->copy())
        };
        return new ThreeAddressCodeBlock(instructionList);
    }
}

Condition irconverter::convert(ASTCondition astCondition) {
    auto operand1 = convert(astCondition.operand1);
    auto operand2 = convert(astCondition.operand2);
    Condition::Operator op;
    switch (astCondition.op) {
        case kCondOperatorEqual:
            op = Condition::Operator::EQ;
            break;
        case kCondOperatorNotEqual:
            op = Condition::Operator::NEQ;
            break;
        case kCondOperatorGreater:
            op = Condition::Operator::GT;
            break;
        case kCondOperatorLess:
            op = Condition::Operator::LT;
            break;
        case kCondOperatorGreaterEqual:
            op = Condition::Operator::GEQ;
            break;
        case kCondOperatorLessEqual:
            op = Condition::Operator::LEQ;
            break;
        default:
            assert(false); // This should never happen
    }

    return Condition(std::move(operand1), std::move(operand2), op);
}

std::unique_ptr<Operand> irconverter::convert(ASTIndex astIndex) {
    if (astIndex.indexType == kASTIndexIdentifier) {
        return std::make_unique<SymbolOperand>(astIndex.identifier);
    } else {
        return std::make_unique<ConstantOperand>(astIndex.value);
    }
}

std::unique_ptr<Operand> irconverter::convert(ASTOperand astOperand) {
    if (astOperand.type == kOperandConstant) {
        return std::make_unique<ConstantOperand>((const int) astOperand.constant);
    } else {
        ASTSymbol symbol = astOperand.symbol;
        return convert(symbol);
    }
}

std::unique_ptr<Operand> irconverter::convert(ASTSymbol symbol) {
    std::string str(symbol.identifier);
    if (symbol.type == kASTIdentifierSimple) {
        return std::make_unique<SymbolOperand>(str);
    } else {
        auto index = convert(symbol.index);
        return std::make_unique<ArraySymbolOperand>(std::move(index), str);
    }
}

ThreeAddressCode::Operator irconverter::convert(int op) {
    switch (op) {
        case kOperatorAdd:
            return ThreeAddressCode::Operator::ADD;
        case kOperatorSub:
            return ThreeAddressCode::Operator::SUB;
        case kOperatorMul:
            return ThreeAddressCode::Operator::MUL;
        case kOperatorDiv:
            return ThreeAddressCode::Operator::DIV;
        case kOperatorMod:
            return ThreeAddressCode::Operator::MOD;
        default:
            assert(false); // This should never happen
    }
}

GlobalSymbolTable* irconverter::symbolTableFrom(ASTDeclarationList *decList) {
    auto* table = new GlobalSymbolTable();
    ASTDeclarationList* current = decList;
    ASTDeclaration dec;

    while (current != nullptr) {
        dec = current->value;
        if (dec.type == kASTIdentifierSimple) {
            table->insert(dec.identifier, Record::integer(dec.identifier));

        } else if (dec.type == kASTIdentifierArray) {
            table->insert(dec.identifier, Record::array(dec.identifier, std::tuple(dec.lower, dec.upper)));
        }

        current = current->next;
    }

    return table;
}
