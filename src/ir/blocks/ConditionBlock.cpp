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
    this->_fail = fail;
    if (_fail != nullptr) {
        this->_failLabel = genLabel();
        this->_postPassLabel = genLabel();
        this->_fail->setId(this->_failLabel);
    } else {
        this->_failLabel = endLabel();
        this->_postPassLabel = endLabel();
    }
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
        _failLabel = genLabel();
        _postPassLabel = genLabel();
        _fail->setId(_failLabel);
    } else {
        _failLabel = endLabel();
        _postPassLabel = endLabel();
    }
}

BaseBlock* ConditionBlock::failBody() {
    return _fail;
}

void ConditionBlock::adjustPassBodyLabels() {
    if (_pass == nullptr) return;

    BaseBlock* current = _pass;
    while (current->next() != nullptr) {
        current = current->next();
    }
    current->setEndLabel(_postPassLabel);
}

void ConditionBlock::adjustFailBodyLabels() {
    if (_fail == nullptr) return;

    BaseBlock* current = _fail;
    while (current->next() != nullptr) {
        current = current->next();
    }
    current->setEndLabel(endLabel());
}

ThreeAddressCodeBlock ConditionBlock::condBody() {
    ThreeAddressCodeBlock block = _condition.toBlock(failLabel());
    block.setId(id());
    block.setEndLabel(_passLabel);

    return block;
}

ThreeAddressCodeBlock ConditionBlock::postPassBody() {
    ThreeAddressCodeBlock block = ThreeAddressCodeBlock({
        ThreeAddressCodeBlock::jump(endLabel(), JUMP_ALWAYS)
    });
    block.setId(_postPassLabel);
    block.setEndLabel(_failLabel);

    return block;
}

Condition& ConditionBlock::condition() {
    return _condition;
}

LabelIdentifier ConditionBlock::passLabel() {
    return _passLabel;
}

LabelIdentifier ConditionBlock::failLabel() {
    return _failLabel;
}

LabelIdentifier ConditionBlock::postPassLabel() {
    return _postPassLabel;
}

void ConditionBlock::onEndLabelChange(LabelIdentifier newLabel) {
    if (_fail == nullptr) {
        _postPassLabel = newLabel;
        _failLabel = newLabel;
    }
}

std::list<ThreeAddressCodeBlock> ConditionBlock::flatten() {
    if (passBody() == nullptr) {
        return {};
    }

    // label adjustment
    adjustPassBodyLabels();
    adjustFailBodyLabels();

    ThreeAddressCodeBlock cond = condBody();
    std::list<ThreeAddressCodeBlock> total = {cond};
    std::list<ThreeAddressCodeBlock> pass = BaseBlock::flattenBlockList(passBody());
    total.insert(total.end(), pass.begin(), pass.end());

    if (failBody() != nullptr) {
        ThreeAddressCodeBlock pb = postPassBody();
        total.push_back(pb);
        std::list<ThreeAddressCodeBlock> fail = BaseBlock::flattenBlockList(failBody());
        total.insert(total.end(), fail.begin(), fail.end());
    }

    return total;
}
