//
// Created by Kacper Raczy on 2019-09-08.
//

#include "BaseBlock.hpp"

BaseBlock::BaseBlock() {
    static uint64_t counter = 0;
    _id = counter++;
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
