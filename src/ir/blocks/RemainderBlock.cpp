//
// Created by Kacper Raczy on 2019-12-01.
//

#include "RemainderBlock.hpp"

RemainderBlock::RemainderBlock(std::unique_ptr<Operand> dest,
                               std::unique_ptr<Operand> firstOperand,
                               std::unique_ptr<Operand> secondOperand,
                               SymbolTable *parentTable):
    DivisionBlock(std::move(dest), std::move(firstOperand), std::move(secondOperand), parentTable) {}

RemainderBlock::RemainderBlock(Operand &dest,
                               Operand &firstOperand,
                               Operand &secondOperand,
                               SymbolTable *parentTable):
    DivisionBlock(dest, firstOperand, secondOperand, parentTable) {}

ThreeAddressCodeBlock RemainderBlock::signAdjustment3() {
    VirtualRegisterOperand v1(1);
    ConstantOperand c0(0);
    SymbolOperand r(remainderRecordName());
    Operand& op2 = *_secondOperand;


    auto check = ThreeAddressCodeBlock({
        ThreeAddressCode(v1.copy(), ThreeAddressCode::LOAD, op2.copy()),
        ThreeAddressCodeBlock::jump(_returnValueLabel, JUMP_POSITIVE)
    });
    auto rUpdate = ThreeAddressCodeBlock::subtraction(r, c0, r);

    check.merge(rUpdate);

    return check;
}

ThreeAddressCodeBlock RemainderBlock::returnValue() {
    SymbolOperand r(remainderRecordName());
    Operand& dest = *_destination;

    return ThreeAddressCodeBlock::copy(dest, r);
}
