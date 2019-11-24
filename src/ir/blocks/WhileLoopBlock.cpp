//
// Created by Kacper Raczy on 2019-09-09.
//

#include "WhileLoopBlock.hpp"

WhileLoopBlock::WhileLoopBlock(Condition condition): LoopBlock(), _condition(condition) {}

ThreeAddressCodeBlock WhileLoopBlock::init() {
    return ThreeAddressCodeBlock();
}

ThreeAddressCodeBlock WhileLoopBlock::pre() {
    return _condition.toBlock(endLabel());
}

ThreeAddressCodeBlock WhileLoopBlock::post() {
    return ThreeAddressCodeBlock({
        ThreeAddressCodeBlock::jump(loopLabel(), JUMP_ALWAYS)
    });
}

Condition& WhileLoopBlock::condition() {
    return _condition;
}

DoWhileLoopBlock::DoWhileLoopBlock(Condition condition): WhileLoopBlock(condition) {}

ThreeAddressCodeBlock DoWhileLoopBlock::init() {
    return ThreeAddressCodeBlock({
         ThreeAddressCodeBlock::jump(bodyLabel(), JUMP_ALWAYS)
    });
}