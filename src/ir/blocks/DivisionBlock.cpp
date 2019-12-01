//
// Created by Kacper Raczy on 2019-11-28.
//

#include "DivisionBlock.hpp"

DivisionBlock::DivisionBlock(Operand &dest,
                             Operand &firstOperand,
                             Operand &secondOperand,
                             SymbolTable *parentTable):
    ControlFlowBlock(NestedSymbolTable(parentTable)),
    _destination(dest.copy()),
    _firstOperand(firstOperand.copy()),
    _secondOperand(secondOperand.copy()) {
    _bitCountLabel = genLabel();
    _body1Label = genLabel();
    _body2Label = genLabel();
    _postBodyLabel = genLabel();
    _signAdjustment1Label = genLabel();
    _signAdjustment2Label = genLabel();
    _signAdjustment3Label = genLabel();
    _returnValueLabel = genLabel();
    Record quotient = Record::integer(quotientRecordName());
    Record remainder = Record::integer(remainderRecordName());
    Record divident = Record::integer(dividentRecordName());
    Record divisor = Record::integer(divisorRecordName());
    Record temporary = Record::integer(tempRecordName());
    Record bitIndex = Record::integer(bitIndexRecordName());
    Record sign = Record::integer(signRecordName());
    _localSymbolTable.insert(quotient.name, quotient);
    _localSymbolTable.insert(remainder.name, remainder);
    _localSymbolTable.insert(divident.name, divident);
    _localSymbolTable.insert(divisor.name, divisor);
    _localSymbolTable.insert(temporary.name, temporary);
    _localSymbolTable.insert(bitIndex.name, bitIndex);
    _localSymbolTable.insert(sign.name, sign);
}

ThreeAddressCodeBlock DivisionBlock::init() {
    ConstantOperand c0(0);
    SymbolOperand q(quotientRecordName());
    SymbolOperand r(remainderRecordName());
    SymbolOperand na(dividentRecordName());
    SymbolOperand nb(divisorRecordName());
    SymbolOperand bi(bitIndexRecordName());

    Operand& op1 = *_firstOperand;
    Operand& op2 = *_secondOperand;

    ThreeAddressCodeBlock zeroQ = ThreeAddressCodeBlock::copy(q, c0);
    ThreeAddressCodeBlock zeroR = ThreeAddressCodeBlock::copy(r, c0);
    ThreeAddressCodeBlock jzeros = ThreeAddressCodeBlock({
        ThreeAddressCode(std::make_unique<VirtualRegisterOperand>(1), ThreeAddressCode::LOAD, op1.copy()),
        ThreeAddressCodeBlock::jump(_returnValueLabel, JUMP_ZERO),
        ThreeAddressCode(std::make_unique<VirtualRegisterOperand>(2), ThreeAddressCode::LOAD, op2.copy()),
        ThreeAddressCodeBlock::jump(_returnValueLabel, JUMP_ZERO),
    });
    ThreeAddressCodeBlock zeroC = ThreeAddressCodeBlock::copy(bi, c0);

    zeroQ.merge(zeroR);
    zeroQ.merge(jzeros);
    zeroQ.merge(zeroC);

    return zeroQ;
}

ThreeAddressCodeBlock DivisionBlock::signAdjustment1() {
    ConstantOperand c0(0);
    ConstantOperand c1(1);
    SymbolOperand na(dividentRecordName());
    SymbolOperand nb(divisorRecordName());
    SymbolOperand sign(signRecordName());
    VirtualRegisterOperand v1(1);

    Operand& op1 = *_firstOperand;
    Operand& op2 = *_secondOperand;

    auto sInit = ThreeAddressCodeBlock::copy(sign, c0);
    auto naInit = ThreeAddressCodeBlock::copy(na, op1);
    auto nbInit = ThreeAddressCodeBlock::copy(nb, op2);
    auto aCheck = ThreeAddressCodeBlock({
        ThreeAddressCode(v1.copy(), ThreeAddressCode::LOAD, op1.copy()),
        ThreeAddressCodeBlock::jump(_signAdjustment2Label, JUMP_POSITIVE)
    });
    auto naUpdate = ThreeAddressCodeBlock::subtraction(na, c0, na);
    auto signUpdate= ThreeAddressCodeBlock::addition(sign, sign, c1);

    sInit.merge(naInit);
    sInit.merge(nbInit);
    sInit.merge(aCheck);
    sInit.merge(naUpdate);
    sInit.merge(signUpdate);

    return sInit;
}

ThreeAddressCodeBlock DivisionBlock::signAdjustment2() {
    ConstantOperand c0(0);
    ConstantOperand c1(1);
    SymbolOperand nb(divisorRecordName());
    SymbolOperand sign(signRecordName());
    VirtualRegisterOperand v1(1);

    auto bCheck = ThreeAddressCodeBlock({
        ThreeAddressCode(v1.copy(), ThreeAddressCode::LOAD, nb.copy()),
        ThreeAddressCodeBlock::jump(_bitCountLabel, JUMP_POSITIVE)
    });
    auto nbUpdate = ThreeAddressCodeBlock::subtraction(nb, c0, nb);
    auto signUpdate= ThreeAddressCodeBlock::subtraction(sign, sign, c1);

    bCheck.merge(nbUpdate);
    bCheck.merge(signUpdate);

    return bCheck;
}

ThreeAddressCodeBlock DivisionBlock::bitCount() {
    ConstantOperand c1u(1);
    ConstantOperand c1s(-1);
    SymbolOperand bi(bitIndexRecordName());
    SymbolOperand na(dividentRecordName());
    SymbolOperand temp(tempRecordName());

    auto tempInit = ThreeAddressCodeBlock::copy(temp, na, 1);
    ThreeAddressCode cond = ThreeAddressCodeBlock::jump(_body1Label, JUMP_ZERO);
    auto biInc = ThreeAddressCodeBlock::addition(bi, bi, c1u, 3);
    auto naDiv = ThreeAddressCodeBlock::shift(temp, temp, c1s, 7);
    ThreeAddressCode cont = ThreeAddressCodeBlock::jump(_bitCountLabel, JUMP_ALWAYS);

    tempInit.append(cond);
    tempInit.merge(biInc);
    tempInit.merge(naDiv);
    tempInit.append(cont);

    return tempInit;
}

ThreeAddressCodeBlock DivisionBlock::body1() {
    ConstantOperand c1u(1);
    ConstantOperand c0(0);
    SymbolOperand r(remainderRecordName());
    SymbolOperand na(dividentRecordName());
    SymbolOperand bi(bitIndexRecordName());
    SymbolOperand temp(tempRecordName());
    auto v1 = VirtualRegisterOperand(1), v2 = VirtualRegisterOperand(2);
    auto v3 = VirtualRegisterOperand(3), v4 = VirtualRegisterOperand(4);

    auto cond = ThreeAddressCodeBlock({
        ThreeAddressCode(std::make_unique<VirtualRegisterOperand>(1), ThreeAddressCode::LOAD, bi.copy()),
        ThreeAddressCodeBlock::jump(_returnValueLabel, JUMP_ZERO)
    });
    auto remShift = ThreeAddressCodeBlock::shift(r, r, c1u);
    auto negBI = ThreeAddressCodeBlock::subtraction(temp, c0, bi);
    auto tempShift = ThreeAddressCodeBlock::shift(temp, na, temp);
    auto biUpdate = ThreeAddressCodeBlock::addition(bi, bi, c1u);
    auto oddCheck = ThreeAddressCodeBlock({
        ThreeAddressCode(v1.copy(), ThreeAddressCode::LOAD, temp.copy()),
        ThreeAddressCode(v2.copy(), ThreeAddressCode::LOAD, c1u.copy()),
        ThreeAddressCode(v3.copy(), ThreeAddressCode::LSHIFT, v1.copy(), v2.copy()),
        ThreeAddressCode(v4.copy(), ThreeAddressCode::SUB, v3.copy(), temp.copy()),
        ThreeAddressCodeBlock::jump(_body2Label, JUMP_ZERO)
    });
    auto remUpdate = ThreeAddressCodeBlock::addition(r, r, c1u);

    cond.merge(remShift);
    cond.merge(negBI);
    cond.merge(tempShift);
    cond.merge(biUpdate);
    cond.merge(oddCheck);
    cond.merge(remUpdate);

    return cond;
}

ThreeAddressCodeBlock DivisionBlock::body2() {
    ConstantOperand c1u(1);
    SymbolOperand q(quotientRecordName());
    SymbolOperand r(remainderRecordName());
    SymbolOperand nb(divisorRecordName());
    SymbolOperand bi(bitIndexRecordName());
    SymbolOperand temp(tempRecordName());
    VirtualRegisterOperand v1(1);
    VirtualRegisterOperand v2(2);
    VirtualRegisterOperand v3(3);

    auto cond = ThreeAddressCodeBlock({
        ThreeAddressCode(v1.copy(), ThreeAddressCode::LOAD, r.copy()),
        ThreeAddressCode(v2.copy(), ThreeAddressCode::LOAD, nb.copy()),
        ThreeAddressCode(v3.copy(), ThreeAddressCode::SUB, v1.copy(), v2.copy()),
        ThreeAddressCodeBlock::jump(_postBodyLabel, JUMP_NEGATIVE),
        ThreeAddressCode(r.copy(), ThreeAddressCode::LOAD, v3.copy())
    });
    auto tempShift = ThreeAddressCodeBlock::shift(temp, c1u, bi);
    auto qUpdate = ThreeAddressCodeBlock::addition(q, q, temp);

    cond.merge(tempShift);
    cond.merge(qUpdate);

    return cond;
}

ThreeAddressCodeBlock DivisionBlock::postBody() {
    return ThreeAddressCodeBlock({
        ThreeAddressCodeBlock::jump(_body1Label, JUMP_ALWAYS)
    });
}

ThreeAddressCodeBlock DivisionBlock::signAdjustment3() {
    VirtualRegisterOperand v1(1);
    ConstantOperand c0(0);
    SymbolOperand q(quotientRecordName());
    SymbolOperand sign(signRecordName());

    auto check = ThreeAddressCodeBlock({
        ThreeAddressCode(v1.copy(), ThreeAddressCode::LOAD, sign.copy()),
        ThreeAddressCodeBlock::jump(_returnValueLabel, JUMP_ZERO)
    });
    auto qUpdate = ThreeAddressCodeBlock::subtraction(q, c0, q);

    check.merge(qUpdate);

    return check;
}

ThreeAddressCodeBlock DivisionBlock::returnValue() {
    SymbolOperand q(quotientRecordName());
    Operand& dest = *_destination;

    return ThreeAddressCodeBlock::copy(dest, q);
}

std::list<ThreeAddressCodeBlock> DivisionBlock::flatten() {
    ThreeAddressCodeBlock binit = init();
    ThreeAddressCodeBlock bsa1 = signAdjustment1();
    ThreeAddressCodeBlock bsa2 = signAdjustment2();
    ThreeAddressCodeBlock bbitCount = bitCount();
    ThreeAddressCodeBlock bbody1 = body1();
    ThreeAddressCodeBlock bbody2 = body2();
    ThreeAddressCodeBlock bpost = postBody();
    ThreeAddressCodeBlock bsa3 = signAdjustment3();
    ThreeAddressCodeBlock bret = returnValue();

    binit.setId(id());
    binit.setEndLabel(_signAdjustment1Label);
    bsa1.setId(_signAdjustment1Label);
    bsa1.setEndLabel(_signAdjustment2Label);
    bsa2.setId(_signAdjustment2Label);
    bsa2.setEndLabel(_bitCountLabel);
    bbitCount.setId(_bitCountLabel);
    bbitCount.setEndLabel(_body1Label);
    bbody1.setId(_body1Label);
    bbody1.setEndLabel(_body2Label);
    bbody2.setId(_body2Label);
    bbody2.setEndLabel(_postBodyLabel);
    bpost.setId(_postBodyLabel);
    bpost.setEndLabel(_signAdjustment3Label);
    bsa3.setId(_signAdjustment3Label);
    bsa3.setEndLabel(_returnValueLabel);
    bret.setId(_returnValueLabel);
    bret.setEndLabel(endLabel());

    std::list<ThreeAddressCodeBlock> total = {
            binit, bsa1, bsa2, bbitCount, bbody1, bbody2, bpost, bsa3, bret
    };

    return total;
}

std::string DivisionBlock::quotientRecordName() {
    return "__tquotient";
}

std::string DivisionBlock::remainderRecordName() {
    return "__tremainder";
}

std::string DivisionBlock::dividentRecordName() {
    return "__tdivident";
}

std::string DivisionBlock::divisorRecordName() {
    return "__tdivisor";
}

std::string DivisionBlock::tempRecordName() {
    return "__tdivtemp";
}

std::string DivisionBlock::bitIndexRecordName() {
    return "__tdivbit";
}

std::string DivisionBlock::signRecordName() {
    return "__tdivsign";
}
