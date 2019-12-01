//
// Created by Kacper Raczy on 2019-12-01.
//

#include "RemainderBlock.hpp"

RemainderBlock::RemainderBlock(Operand &dest,
                               Operand &firstOperand,
                               Operand &secondOperand,
                               SymbolTable *parentTable):
    DivisionBlock(dest, firstOperand, secondOperand, parentTable) {}

ThreeAddressCodeBlock RemainderBlock::signAdjustment3() {
    VirtualRegisterOperand v1(1);
    ConstantOperand c0(0);
    SymbolOperand r(remainderRecordName());
    SymbolOperand sign(signRecordName());

    auto check = ThreeAddressCodeBlock({
        ThreeAddressCode(v1.copy(), ThreeAddressCode::LOAD, sign.copy()),
        ThreeAddressCodeBlock::jump(_returnValueLabel, JUMP_ZERO)
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
