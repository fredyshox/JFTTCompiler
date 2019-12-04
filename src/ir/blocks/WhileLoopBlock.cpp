//
// Created by Kacper Raczy on 2019-09-09.
//

#include "WhileLoopBlock.hpp"

WhileLoopBlock::WhileLoopBlock(Condition condition, SymbolTable* parentTable): 
    LoopBlock(parentTable), 
    _condition(condition) {}

ThreeAddressCodeBlock WhileLoopBlock::init() {
    return ThreeAddressCodeBlock();
}

ThreeAddressCodeBlock WhileLoopBlock::pre() {
    ThreeAddressCodeBlock block = _condition.toBlock(endLabel());
    block.setId(loopLabel());
    return block;
}

ThreeAddressCodeBlock WhileLoopBlock::post() {
    return ThreeAddressCodeBlock({
        ThreeAddressCodeBlock::jump(loopLabel(), JUMP_ALWAYS)
    });
}

Condition& WhileLoopBlock::condition() {
    return _condition;
}

void WhileLoopBlock::onIdChange(LabelIdentifier newId) {
    setLoopLabel(newId);
}

DoWhileLoopBlock::DoWhileLoopBlock(Condition condition, SymbolTable* parentTable): 
    WhileLoopBlock(condition, parentTable) {}

ThreeAddressCodeBlock DoWhileLoopBlock::init() {
    return ThreeAddressCodeBlock({
         ThreeAddressCodeBlock::jump(bodyLabel(), JUMP_ALWAYS)
    });
}

void DoWhileLoopBlock::onIdChange(LabelIdentifier) {}
