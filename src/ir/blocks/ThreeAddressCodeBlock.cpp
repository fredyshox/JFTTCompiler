//
// Created by Kacper Raczy on 2019-09-09.
//

#include "ThreeAddressCodeBlock.hpp"

ThreeAddressCodeBlock::ThreeAddressCodeBlock(const std::list<ThreeAddressCode> &codes): BaseBlock(), _codes(codes) {}

uint64_t ThreeAddressCodeBlock::size() {
    return this->_codes.size();
}

std::list<ThreeAddressCode>& ThreeAddressCodeBlock::codes() {
    return this->_codes;
}

void ThreeAddressCodeBlock::merge(ThreeAddressCodeBlock &block) {
    _codes.splice(_codes.end(), block._codes);
}

void ThreeAddressCodeBlock::append(ThreeAddressCode& code) {
    _codes.push_back(code);
}

std::list<ThreeAddressCodeBlock> ThreeAddressCodeBlock::flatten() {
    if (this->size() == 0) {
        return {};

    }
    return {*this};
}

ThreeAddressCodeBlock ThreeAddressCodeBlock::binaryOperation(Operand &dest,
                                                              Operand &op1,
                                                              Operand &op2,
                                                              ThreeAddressCode::Operator op) {
    auto v1 = VirtualRegisterOperand(1);
    auto v2 = VirtualRegisterOperand(2);
    auto v3 = VirtualRegisterOperand(3);

    std::list<ThreeAddressCode> instructionList = {
            ThreeAddressCode(v1.copy(), ThreeAddressCode::Operator::LOAD, op1.copy()),
            ThreeAddressCode(v2.copy(), ThreeAddressCode::Operator::LOAD, op2.copy()),
            ThreeAddressCode(v3.copy(), op, v1.copy(), v2.copy()),
            ThreeAddressCode(dest.copy(), ThreeAddressCode::Operator::LOAD, v3.copy())
    };
    return ThreeAddressCodeBlock(instructionList);
}

ThreeAddressCodeBlock ThreeAddressCodeBlock::addition(Operand &dest,
                                                       Operand &op1,
                                                       Operand &op2) {
    return binaryOperation(dest, op1, op2, ThreeAddressCode::Operator::ADD);
}

ThreeAddressCodeBlock ThreeAddressCodeBlock::subtraction(Operand &dest,
                                                           Operand &op1,
                                                           Operand &op2) {
    return binaryOperation(dest, op1, op2, ThreeAddressCode::Operator::SUB);
}

ThreeAddressCodeBlock ThreeAddressCodeBlock::multiplication(Operand &dest,
                                                             Operand &op1,
                                                             Operand &op2) {
    return binaryOperation(dest, op1, op2, ThreeAddressCode::Operator::MUL);
}

ThreeAddressCodeBlock ThreeAddressCodeBlock::division(Operand &dest,
                                                       Operand &op1,
                                                       Operand &op2) {
    return binaryOperation(dest, op1, op2, ThreeAddressCode::Operator::DIV);
}

ThreeAddressCodeBlock ThreeAddressCodeBlock::remainder(Operand &dest,
                                                             Operand &op1,
                                                             Operand &op2) {
    return binaryOperation(dest, op1, op2, ThreeAddressCode::Operator::MOD);
}

ThreeAddressCodeBlock ThreeAddressCodeBlock::read(Operand &dest) {
    auto v1 = VirtualRegisterOperand(1);
    std::list<ThreeAddressCode> instructionList {
            ThreeAddressCode(v1.copy(), ThreeAddressCode::Operator::STDIN),
            ThreeAddressCode(dest.copy(), ThreeAddressCode::Operator::LOAD, v1.copy())
    };
    return ThreeAddressCodeBlock(instructionList);
}

ThreeAddressCodeBlock ThreeAddressCodeBlock::write(Operand &source) {
    auto v1 = VirtualRegisterOperand(1);
    std::list<ThreeAddressCode> instructionList {
            ThreeAddressCode(v1.copy(), ThreeAddressCode::Operator::LOAD, source.copy()),
            ThreeAddressCode(v1.copy(), ThreeAddressCode::Operator::STDOUT),
    };
    return ThreeAddressCodeBlock(instructionList);
}

ThreeAddressCodeBlock ThreeAddressCodeBlock::copy(Operand &dest,
                                                  Operand& source) {
    auto v1 = VirtualRegisterOperand(1);
    std::list<ThreeAddressCode> instructionList {
        ThreeAddressCode(v1.copy(), ThreeAddressCode::Operator::LOAD, source.copy()),
        ThreeAddressCode(dest.copy(), ThreeAddressCode::Operator::LOAD, v1.copy())
    };
    return ThreeAddressCodeBlock(instructionList);
}

ThreeAddressCode ThreeAddressCodeBlock::jump(LabelIdentifier label, int jtype) {
    ThreeAddressCode::Operator op;
    if (jtype == JUMP_POSITIVE) {
        op = ThreeAddressCode::Operator::JPLUS;
    } else if (jtype == JUMP_NEGATIVE) {
        op = ThreeAddressCode::Operator::JMINUS;
    } else if (jtype == JUMP_ZERO) {
        op = ThreeAddressCode::Operator::JZERO;
    } else {
        op = ThreeAddressCode::Operator::JUMP;
    }

    auto tac = ThreeAddressCode({}, op);
    tac.label = label;
    return tac;
}