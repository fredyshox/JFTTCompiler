//
// ir
// Created by Kacper Raczy on 2019-09-06.
//

#ifndef ir_condition_hpp
#define ir_condition_hpp

#include <list>
#include "blocks/ThreeAddressCodeBlock.hpp"
#include "Operand.hpp"
#include "Label.hpp"

struct Condition {
    // MARK: Types
    enum Operator {
        EQ, NEQ, GT, GEQ, LT, LEQ
    };

    // MARK: Fields
    std::unique_ptr<Operand> firstOperand;
    std::unique_ptr<Operand> secondOperand;
    Operator op;

    // MARK: Constructors
    Condition(Operand &operand0, Operand &operand1, Operator op);
    Condition(std::unique_ptr<Operand> operandPtr0, std::unique_ptr<Operand> operandPtr1, Operator op);
    Condition(const Condition &cond);
    Condition& operator=(const Condition &cond);
    ~Condition() = default;
    // MARK: Methods
    ThreeAddressCodeBlock toBlock(LabelIdentifier failId);
};

#endif /* ir_condition_hpp */
