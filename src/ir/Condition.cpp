//
// Created by Kacper Raczy on 2019-11-22.
//

#include "Condition.hpp"

Condition::Condition(Operand &operand0, Operand &operand1, Operator op):
    firstOperand(operand0.copy()),
    secondOperand(operand1.copy()) {
    this->op = op;
}

Condition::Condition(std::unique_ptr<Operand> operandPtr0, std::unique_ptr<Operand> operandPtr1, Operator op):
    firstOperand(std::move(operandPtr0)),
    secondOperand(std::move(operandPtr1)) {
    this->op = op;
}

Condition::Condition(const Condition &cond) {
    this->firstOperand = cond.firstOperand->copy();
    this->secondOperand = cond.secondOperand->copy();
    this->op = cond.op;
}

Condition& Condition::operator=(const Condition &cond) {
    this->firstOperand = cond.firstOperand->copy();
    this->secondOperand = cond.secondOperand->copy();
    this->op = cond.op;
    return *this;
}

ThreeAddressCodeBlock Condition::toBlock(LabelIdentifier failId) {
    auto dest = VirtualRegisterOperand(1);
    auto sub = ThreeAddressCodeBlock::subtraction(dest, *firstOperand, *secondOperand);
    sub.codes().pop_back();

    auto jumpsLambda = [failId](Operator op) -> std::list<ThreeAddressCode> {
        switch (op) {
            case EQ:
                return {
                        ThreeAddressCodeBlock::jump(failId, JUMP_POSITIVE),
                        ThreeAddressCodeBlock::jump(failId, JUMP_NEGATIVE)
                };
            case NEQ:
                return {
                        ThreeAddressCodeBlock::jump(failId, JUMP_ZERO)
                };
            case GT:
                return {
                        ThreeAddressCodeBlock::jump(failId, JUMP_ZERO),
                        ThreeAddressCodeBlock::jump(failId, JUMP_NEGATIVE)
                };
            case GEQ:
                return {
                        ThreeAddressCodeBlock::jump(failId, JUMP_NEGATIVE)
                };
            case LT:
                return {
                        ThreeAddressCodeBlock::jump(failId, JUMP_POSITIVE),
                        ThreeAddressCodeBlock::jump(failId, JUMP_ZERO)
                };
            case LEQ:
                return {
                        ThreeAddressCodeBlock::jump(failId, JUMP_POSITIVE)
                };
        }
    };

    ThreeAddressCodeBlock jumps(jumpsLambda(op));
    sub.merge(jumps);
    return sub;
}
