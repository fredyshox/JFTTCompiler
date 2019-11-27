//
// Created by Kacper Raczy on 2019-09-09.
//

#include "ForLoopBlock.hpp"

LoopRange::LoopRange(Operand &init, Operand &bound, int step):
    initial(init.copy()),
    bound(bound.copy()) {
    this->step = step;
}

LoopRange::LoopRange(std::unique_ptr<Operand> &initPtr, std::unique_ptr<Operand> &boundPtr, int step):
    initial(std::move(initPtr)),
    bound(std::move(boundPtr)) {
    this->step = step;
}

LoopRange::LoopRange(const LoopRange &range): initial(range.initial->copy()), bound(range.bound->copy()) {
    this->step = range.step;
}

LoopRange& LoopRange::operator=(const LoopRange &range) {
    initial = range.initial->copy();
    bound = range.initial->copy();
    step = range.step;
    return *this;
}

ForLoopBlock::ForLoopBlock(std::string iterator, LoopRange range): LoopBlock(), _iteratorName(iterator), _loopRange(range) {
    Record rIter = Record::iterator(_iteratorName); 
    std::stringstream ss;
    ss << "_" << iterator << "_counter";
    _counterName = ss.str();
    Record rCounter = Record::iterator(_counterName);
    _localSymbolTable.insert(_iteratorName, rIter);
    _localSymbolTable.insert(_counterName, rCounter);
}

ThreeAddressCodeBlock ForLoopBlock::init() {
    SymbolOperand iterator(_iteratorName);
    SymbolOperand counter(_counterName);
    Operand& initial = *_loopRange.initial;
    Operand& bound = *_loopRange.bound;
    ConstantOperand step(1);

    ThreeAddressCodeBlock block;
    ThreeAddressCodeBlock temp = ThreeAddressCodeBlock::copy(iterator, initial);
    block.merge(temp);
    if (_loopRange.step > 0) {
        temp = ThreeAddressCodeBlock::addition(counter, bound, step);
        block.merge(temp);
        temp = ThreeAddressCodeBlock::subtraction(counter, counter, initial);
        block.merge(temp);
    } else {
        temp = ThreeAddressCodeBlock::addition(counter, initial, step);
        block.merge(temp);
        temp = ThreeAddressCodeBlock::subtraction(counter, counter, bound);
        block.merge(temp);
    }

    return block;
}

ThreeAddressCodeBlock ForLoopBlock::pre() {
    SymbolOperand counter(_counterName);
    ConstantOperand zero(0);

    Condition cond(counter, zero, Condition::Operator::NEQ);
    ThreeAddressCodeBlock block = cond.toBlock(endLabel());
    block.setId(loopLabel());
    return block;
}

ThreeAddressCodeBlock ForLoopBlock::post() {
    SymbolOperand iterator(_iteratorName);
    SymbolOperand counter(_counterName);
    ConstantOperand step(_loopRange.step);

    ThreeAddressCodeBlock iteratorUpdate;
    if (_loopRange.step > 0) {
        iteratorUpdate = ThreeAddressCodeBlock::addition(iterator, iterator, step);
    } else {
        iteratorUpdate = ThreeAddressCodeBlock::subtraction(iterator, iterator, step);
    }
    ThreeAddressCodeBlock counterUpdate = ThreeAddressCodeBlock::subtraction(counter, counter, step);
    ThreeAddressCode loopJump = ThreeAddressCodeBlock::jump(loopLabel(), JUMP_ALWAYS);

    ThreeAddressCodeBlock block;
    block.merge(iteratorUpdate);
    block.merge(counterUpdate);
    block.append(loopJump);
    return block;
}

std::string ForLoopBlock::iteratorName() {
    return _iteratorName;
}

std::string ForLoopBlock::counterName() {
    return _counterName;
}

LoopRange& ForLoopBlock::loopRange() {
    return _loopRange;
}