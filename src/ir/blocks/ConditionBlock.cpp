//
// Created by Kacper Raczy on 2019-09-09.
//

#include "ConditionBlock.hpp"

ConditionBlock::ConditionBlock(Condition condition, BaseBlock *pass, BaseBlock *fail): ControlFlowBlock(NestedSymbolTable()), _condition(condition) {
    this->_passLabel = genLabel();
    this->_pass = pass;
    if (_pass != nullptr) {
        this->_pass->setId(this->_passLabel);
    }
    this->_failLabel = genLabel();
    this->_fail = fail;
    if (_fail != nullptr) {
        this->_fail->setId(this->_failLabel);
    }
    this->_endLabel = genLabel();
}

void ConditionBlock::setPassBody(BaseBlock *block) {
    _pass = block;
    if (_pass != nullptr) {
        _pass->setId(_passLabel);
    }
}

BaseBlock* ConditionBlock::passBody() {
    return _pass;
}

void ConditionBlock::setFailBody(BaseBlock *block) {
    _fail = block;
    if (_fail != nullptr) {
        _fail->setId(_failLabel);
    }
}

BaseBlock* ConditionBlock::failBody() {
    return _fail;
}

Condition& ConditionBlock::condition() {
    return _condition;
}

ThreeAddressCodeBlock ConditionBlock::condBody() {
    ThreeAddressCodeBlock block = _condition.toBlock(failLabel());
    block.setId(id());

    return block;
}

LabelIdentifier ConditionBlock::passLabel() {
    return _passLabel;
}

LabelIdentifier ConditionBlock::failLabel() {
    return _failLabel;
}

LabelIdentifier ConditionBlock::endLabel() {
    if (_fail == nullptr) {
        return _failLabel;
    }
    
    return _endLabel;
}

std::list<ThreeAddressCodeBlock> ConditionBlock::flatten() {
    if (passBody() == nullptr) {
        return {};
    }

    ThreeAddressCodeBlock cond = condBody();
    std::list<ThreeAddressCodeBlock> total = {cond};
    std::list<ThreeAddressCodeBlock> pass = BaseBlock::flattenBlockList(passBody());
    total.insert(total.end(), pass.begin(), pass.end());

    if (failBody() != nullptr) {
        std::list<ThreeAddressCodeBlock> fail = BaseBlock::flattenBlockList(failBody());
        total.insert(total.end(), fail.begin(), fail.end());
    }

    return total;
}
