//
// ir
// Created by Kacper Raczy on 2019-09-07.
//

#ifndef ir_threeaddresscode_hpp
#define ir_threeaddresscode_hpp

#include <optional>
#include "Operand.hpp"

struct ThreeAddressCode {
    // MARK: Types
    enum Operator {
        LOAD, ADD, SUB, MUL, DIV, MOD, STDIN, STDOUT, JUMP, JPLUS, JMINUS, JZERO, LOAD_IND
    };

    // MARK: Fields
    std::unique_ptr<Operand> destination;
    Operator op;
    std::unique_ptr<Operand> firstOperand;
    std::unique_ptr<Operand> secondOperand;
    std::optional<LabelIdentifier> label;

    // MARK: Constructors
    ThreeAddressCode(Operand &dest, Operator op, Operand* operand0 = nullptr, Operand* operand1 = nullptr): destination(dest.copy()) {
        if (this->firstOperand != nullptr)
            this->firstOperand = operand0->copy();
        if (this->secondOperand != nullptr)
            this->secondOperand = operand1->copy();

        this->op = op;
    }
    ThreeAddressCode(std::unique_ptr<Operand> destPtr, Operator op, std::unique_ptr<Operand> operandPtr0 = {}, std::unique_ptr<Operand> operandPtr1 = {}):
        destination(std::move(destPtr)),
        firstOperand(std::move(operandPtr0)),
        secondOperand(std::move(operandPtr1)) {
        this->op = op;
    }
    ThreeAddressCode(const ThreeAddressCode& tac) {
        this->destination = tac.destination->copy();
        this->op = tac.op;
        if (tac.firstOperand)
            this->firstOperand = tac.firstOperand->copy();
        if (tac.secondOperand)
            this->secondOperand = tac.secondOperand->copy();
    }
    ThreeAddressCode& operator=(const ThreeAddressCode &tac) {
        this->destination = tac.destination->copy();
        this->op = tac.op;
        if (tac.firstOperand)
            this->firstOperand = tac.firstOperand->copy();
        if (tac.secondOperand)
            this->secondOperand = tac.secondOperand->copy();
        return *this;
    }
    ThreeAddressCode() = delete;
    ~ThreeAddressCode() = default;
};

#endif /* ir_threeaddresscode_hpp */
