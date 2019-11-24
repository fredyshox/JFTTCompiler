//
// Created by Kacper Raczy on 2019-09-09.
//

#include "ConditionBlock.hpp"

ConditionBlock::ConditionBlock(Condition condition, BaseBlock *pass, BaseBlock *fail): ControlFlowBlock(NestedSymbolTable()), _condition(condition) {
    this->_pass = pass;
    this->_passLabel = genLabel();
    this->_fail = fail;
    this->_failLabel = genLabel();
}

BaseBlock* ConditionBlock::passBody() {
    return _pass;
}

void ConditionBlock::setPassBody(BaseBlock *block) {
    _pass = block;
}

void ConditionBlock::setFailBody(BaseBlock *block) {
    _fail = block;
}

BaseBlock* ConditionBlock::failBody() {
    return _fail;
}

Condition& ConditionBlock::condition() {
    return _condition;
}

ThreeAddressCodeBlock ConditionBlock::condBody() {
    return _condition.toBlock(failLabel());
}

LabelIdentifier ConditionBlock::passLabel() {
    return _passLabel;
}

LabelIdentifier ConditionBlock::failLabel() {
    return _failLabel;
}
