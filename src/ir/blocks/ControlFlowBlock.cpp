//
// Created by Kacper Raczy on 2019-09-06.
//

#include "ControlFlowBlock.hpp"

// MARK: ControlFlowBlock

ControlFlowBlock::ControlFlowBlock(NestedSymbolTable symbolTable): BaseBlock() {
    this->_localSymbolTable = symbolTable;
}

NestedSymbolTable& ControlFlowBlock::localSymbolTable() {
    return _localSymbolTable;
}

// MARK: LoopBlock

void LoopBlock::setLoopLabel(LabelIdentifier newLabel) {
    _loopLabel = newLabel;
}

void LoopBlock::adjustBodyLabels() {
    if (_body == nullptr) return;

    BaseBlock* current = _body;
    while (current->next() != nullptr) {
        current = current->next();
    }
    current->setEndLabel(postBodyLabel());
}

LoopBlock::LoopBlock(SymbolTable* parentTable): ControlFlowBlock(NestedSymbolTable(parentTable)) {
    this->_loopLabel = genLabel();
    this->_bodyLabel = genLabel();
    this->_postBodyLabel = genLabel();
}

BaseBlock* LoopBlock::body() {
    return _body;
}

void LoopBlock::setBody(BaseBlock *blocks) {
    if (blocks != nullptr)
        blocks->setId(bodyLabel());
    this->_body = blocks;
}

LabelIdentifier LoopBlock::loopLabel() {
    return _loopLabel;
}

LabelIdentifier LoopBlock::bodyLabel() {
    return _bodyLabel;
}

LabelIdentifier LoopBlock::postBodyLabel() {
    return _postBodyLabel;
}

std::list<ThreeAddressCodeBlock> LoopBlock::flatten() {
    if (body() == nullptr) {
        return {};
    }

    // label adjustment
    adjustBodyLabels();

    std::list<ThreeAddressCodeBlock> total;
    ThreeAddressCodeBlock binit = init();
    ThreeAddressCodeBlock bpre = pre();
    ThreeAddressCodeBlock bpost = post();
    if (binit.size() > 0) {
        binit.setEndLabel(loopLabel());
        total.push_back(binit);
    }
    if (bpre.size() > 0) {
        bpre.setId(loopLabel());
        bpre.setEndLabel(bodyLabel());
        total.push_back(bpre);
    }

    std::list<ThreeAddressCodeBlock> flatBody = BaseBlock::flattenBlockList(body());
    total.insert(total.end(), flatBody.begin(), flatBody.end());


    if (bpost.size() > 0) {
        bpost.setId(postBodyLabel());
        bpost.setEndLabel(endLabel());
        total.push_back(bpost);
    }

    // total always > 0
    // first flat block id = id()
    (*total.begin()).setId(id());

    return total;
}
