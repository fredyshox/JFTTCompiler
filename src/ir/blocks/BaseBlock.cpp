//
// Created by Kacper Raczy on 2019-09-08.
//

#include "BaseBlock.hpp"
#include "ThreeAddressCodeBlock.hpp"

BaseBlock::BaseBlock() {
    _id = genLabel();
    _next = nullptr;
}

BaseBlock* BaseBlock::next() {
    return _next;
}

void BaseBlock::setNext(BaseBlock *next) {
    _next = next;
}

uint64_t BaseBlock::id() {
    return _id;
}

void BaseBlock::setId(uint64_t id) {
    _id = id;
}

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
