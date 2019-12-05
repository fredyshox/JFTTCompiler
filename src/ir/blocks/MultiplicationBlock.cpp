//
// Created by Kacper Raczy on 2019-11-27.
//

#include "MultiplicationBlock.hpp"

MultiplicationBlock::MultiplicationBlock(std::unique_ptr<Operand> dest,
                                         std::unique_ptr<Operand> op1,
                                         std::unique_ptr<Operand> op2,
                                         SymbolTable *parentTable):
    ControlFlowBlock(NestedSymbolTable(parentTable)),
    _destination(std::move(dest)),
    _firstOperand(std::move(op1)),
    _secondOperand(std::move(op2)) {
    _negativeInitLabel = genLabel();
    _bodyLabel = genLabel();
    _postBodyLabel = genLabel();
    Record multiplier = Record::integer(multiplierRecordName());
    Record multiplicand = Record::integer(multiplicandRecordName());
    _localSymbolTable.insert(multiplier.name, multiplier);
    _localSymbolTable.insert(multiplicand.name, multiplicand);
}

MultiplicationBlock::MultiplicationBlock(Operand &dest,
                                         Operand &firstOperand,
                                         Operand &secondOperand,
                                         SymbolTable* parentTable):
    MultiplicationBlock(dest.copy(), firstOperand.copy(), secondOperand.copy(), parentTable) {}

ThreeAddressCodeBlock MultiplicationBlock::init() {
    auto v7 = VirtualRegisterOperand(7);
    auto v8 = VirtualRegisterOperand(8);
    auto v9 = VirtualRegisterOperand(9);
    auto c0 = ConstantOperand(0);
    auto multiplier = SymbolOperand(multiplierRecordName());
    auto multiplicand = SymbolOperand(multiplicandRecordName());

    Operand& dest = *_destination;
    Operand& op1 = *_firstOperand;
    Operand& op2 = *_secondOperand;

    auto prepareMultiplicandBlock = ThreeAddressCodeBlock::copy(multiplicand, op2, 1);
    auto prepareMultiplierBlock = ThreeAddressCodeBlock::copy(multiplier, op1, 3);
    auto zeroResBlock = ThreeAddressCodeBlock::copy(dest, c0, 5);
    auto checkZeroBlock = ThreeAddressCodeBlock({
        ThreeAddressCode(v7.copy(), ThreeAddressCode::LOAD, multiplicand.copy()),
        ThreeAddressCodeBlock::jump(endLabel(), JUMP_ZERO),
        ThreeAddressCode(v8.copy(), ThreeAddressCode::LOAD, multiplier.copy()),
        ThreeAddressCodeBlock::jump(endLabel(), JUMP_ZERO)
    });
    auto negativeCheckBlock = ThreeAddressCodeBlock({
        ThreeAddressCode(v9.copy(), ThreeAddressCode::LOAD, multiplier.copy()),
        ThreeAddressCodeBlock::jump(negativeInitLabel(), JUMP_NEGATIVE),
        ThreeAddressCodeBlock::jump(bodyLabel(), JUMP_ALWAYS)
    });

    ThreeAddressCodeBlock block;
    block.merge(prepareMultiplicandBlock);
    block.merge(prepareMultiplierBlock);
    block.merge(zeroResBlock);
    block.merge(checkZeroBlock);
    block.merge(negativeCheckBlock);

    return block;
}

ThreeAddressCodeBlock MultiplicationBlock::negativeInit() {
    auto c0 = ConstantOperand(0);
    auto multiplicand = SymbolOperand(multiplicandRecordName());
    auto multiplier = SymbolOperand(multiplierRecordName());
    auto block1 = ThreeAddressCodeBlock::subtraction(multiplicand, c0, multiplicand);
    auto block2 = ThreeAddressCodeBlock::subtraction(multiplier, c0, multiplier);
    block1.merge(block2);

    return block1;
}

ThreeAddressCodeBlock MultiplicationBlock::body() {
    auto v1 = VirtualRegisterOperand(1), v2 = VirtualRegisterOperand(2);
    auto v3 = VirtualRegisterOperand(3), v4 = VirtualRegisterOperand(4);
    auto v5 = VirtualRegisterOperand(5), v6 = VirtualRegisterOperand(6);
    auto multiplier = SymbolOperand(multiplierRecordName());
    auto multiplicand = SymbolOperand(multiplicandRecordName());
    auto c1u = ConstantOperand(1);
    auto c1s = ConstantOperand(-1);
    Operand& dest = *_destination;

    ThreeAddressCodeBlock bodyBlock = ThreeAddressCodeBlock({
        ThreeAddressCode(v1.copy(), ThreeAddressCode::LOAD, multiplier.copy()),
        ThreeAddressCodeBlock::jump(endLabel(), JUMP_ZERO),
        ThreeAddressCode(v2.copy(), ThreeAddressCode::LOAD, c1s.copy()),
        ThreeAddressCode(v3.copy(), ThreeAddressCode::LSHIFT, v1.copy(), v2.copy()),
        ThreeAddressCode(v4.copy(), ThreeAddressCode::LOAD, c1u.copy()),
        ThreeAddressCode(v5.copy(), ThreeAddressCode::LSHIFT, v3.copy(), v4.copy()),
        ThreeAddressCode(v6.copy(), ThreeAddressCode::SUB, v5.copy(), v1.copy()),
        ThreeAddressCodeBlock::jump(postBodyLabel(), JUMP_ZERO)
    });
    ThreeAddressCodeBlock resUpdate = ThreeAddressCodeBlock::addition(dest, dest, multiplicand, 7);
    bodyBlock.merge(resUpdate);

    return bodyBlock;
}

ThreeAddressCodeBlock MultiplicationBlock::postBody() {
    auto multiplier = SymbolOperand(multiplierRecordName());
    auto multiplicand = SymbolOperand(multiplicandRecordName());
    auto c1u = ConstantOperand(1);
    auto c1s = ConstantOperand(-1);

    auto multiplierUpdateBlock = ThreeAddressCodeBlock::shift(multiplier, multiplier, c1s, 1);
    auto multiplicandUpdateBlock = ThreeAddressCodeBlock::shift(multiplicand, multiplicand, c1u, 5);
    auto continueTac = ThreeAddressCodeBlock::jump(bodyLabel(), JUMP_ALWAYS);

    multiplicandUpdateBlock.append(continueTac);
    multiplierUpdateBlock.merge(multiplicandUpdateBlock);

    return multiplierUpdateBlock;
}

LabelIdentifier MultiplicationBlock::negativeInitLabel() {
    return _negativeInitLabel;
}

LabelIdentifier MultiplicationBlock::bodyLabel() {
    return _bodyLabel;
}

LabelIdentifier MultiplicationBlock::postBodyLabel() {
    return _postBodyLabel;
}

std::list<ThreeAddressCodeBlock> MultiplicationBlock::flatten() {
    ThreeAddressCodeBlock binit = init();
    ThreeAddressCodeBlock bneg = negativeInit();
    ThreeAddressCodeBlock bbody = body();
    ThreeAddressCodeBlock bpostBody = postBody();
    
    binit.setId(id());
    binit.setEndLabel(negativeInitLabel());
    bneg.setId(negativeInitLabel());
    bneg.setEndLabel(bodyLabel());
    bbody.setId(bodyLabel());
    bbody.setEndLabel(postBodyLabel());
    bpostBody.setId(postBodyLabel());
    bpostBody.setEndLabel(endLabel());
    
    std::list<ThreeAddressCodeBlock> total = {binit, bneg, bbody, bpostBody};
    return total;
}

std::string MultiplicationBlock::multiplierRecordName() {
    return "__tmultiplier";
}

std::string MultiplicationBlock::multiplicandRecordName() {
    return "__tmultiplicand";
}
