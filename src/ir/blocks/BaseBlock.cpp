//
// Created by Kacper Raczy on 2019-09-08.
//

#include "BaseBlock.hpp"
#include "ThreeAddressCodeBlock.hpp"

BaseBlock::BaseBlock() {
    _id = LABEL_START;
    _endLabel = LABEL_END;
    _next = nullptr;
}

BaseBlock* BaseBlock::next() {
    return _next;
}

void BaseBlock::setNext(BaseBlock *next) {
    _next = next;
    if (next != nullptr) {
        setEndLabel(genLabel());
        next->setId(_endLabel);
    }
}

LabelIdentifier BaseBlock::id() {
    return _id;
}

void BaseBlock::setId(LabelIdentifier id) {
    _id = id;
    onIdChange(id);
}

LabelIdentifier BaseBlock::endLabel() {
    return _endLabel;
}

void BaseBlock::setEndLabel(LabelIdentifier endLabel) {
    _endLabel = endLabel;
    onEndLabelChange(endLabel);
}

void BaseBlock::onIdChange(LabelIdentifier newId) {}

void BaseBlock::onEndLabelChange(LabelIdentifier newLabel) {}

std::list<ThreeAddressCodeBlock> BaseBlock::flattenBlockList(BaseBlock *block) {
    BaseBlock* current = block;
    std::list<ThreeAddressCodeBlock> total;
    while (current != nullptr) {
        std::list<ThreeAddressCodeBlock> f = current->flatten();
        total.insert(total.end(), f.begin(), f.end());
        current = current->next();
    }

    return total;
}
