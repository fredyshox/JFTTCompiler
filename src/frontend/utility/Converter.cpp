//
// Created by Kacper Raczy on 2019-09-09.
//

#include "Converter.hpp"
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
        block = convert(*command);
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
    ForLoopBlock* block = new ForLoopBlock(forLoop.iterator, range);
    block->localSymbolTable().setParent(parentTable);
    block->setBody(convertList(forLoop.start, &block->localSymbolTable()));

    return block;
}

WhileLoopBlock* irconverter::convert(ASTWhileLoop whileLoop, SymbolTable* parentTable) {
    Condition cond = convert(whileLoop.condition);
    WhileLoopBlock* block = (whileLoop.type == kLoopWhile) ? new WhileLoopBlock(cond) : new DoWhileLoopBlock(cond);
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

ThreeAddressCodeBlock* irconverter::convert(ASTAssignment assignment) {
    auto dest = convert(assignment.symbol);
    if (assignment.rtype == kRTypeExpression) {
        ASTExpression expr = assignment.expression;
        auto op1 = convert(expr.operand1);
        auto op2 = convert(expr.operand2);
        auto v1 = std::make_unique<VirtualRegisterOperand>(1);
        auto v2 = std::make_unique<VirtualRegisterOperand>(2);
        auto v3 = std::make_unique<VirtualRegisterOperand>(3);
        std::list<ThreeAddressCode> instructionList {
            ThreeAddressCode(v1->copy(), ThreeAddressCode::Operator::LOAD, std::move(op1)),
            ThreeAddressCode(v2->copy(), ThreeAddressCode::Operator::LOAD, std::move(op2)),
            ThreeAddressCode(v3->copy(), convert(expr.op), v1->copy(), v2->copy()),
            ThreeAddressCode(std::move(dest), ThreeAddressCode::Operator::LOAD, v3->copy())
        };
        return new ThreeAddressCodeBlock(instructionList);
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
