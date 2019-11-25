//
// ir
// Created by Kacper Raczy on 2019-09-06.
//

#ifndef ir_controlflowblock_hpp
#define ir_controlflowblock_hpp

#include <string>
#include <list>
#include "BaseBlock.hpp"
#include "SymbolTable.hpp"
#include "ThreeAddressCodeBlock.hpp"
#include "Label.hpp"

/**
 * Abstract class for control-flow blocks
 */
class ControlFlowBlock: public BaseBlock {
protected:
    NestedSymbolTable _localSymbolTable;
public:
    explicit ControlFlowBlock(NestedSymbolTable symbolTable): BaseBlock() {
        this->_localSymbolTable = symbolTable;
    }
    virtual ~ControlFlowBlock() = default;
    NestedSymbolTable& localSymbolTable() {
        return _localSymbolTable;
    }
};

/**
 * Abstract class for loop blocks
 * This block generates some labels:
 * - loop label: before pre block
 * - body label: before body block
 */
class LoopBlock: public ControlFlowBlock {
private:
    LabelIdentifier _loopLabel;
    LabelIdentifier _bodyLabel;
    LabelIdentifier _postBodyLabel;
    BaseBlock* _body = nullptr;
protected:
    void setLoopLabel(LabelIdentifier newLabel) {
        _loopLabel = newLabel;
    }

    void adjustBodyLabels() {
        if (_body == nullptr) return;

        BaseBlock* current = _body;
        while (current->next() != nullptr) {
            current = current->next();
        }
        current->setEndLabel(postBodyLabel());
    }
public:
    /**
    * Returns block of three address codes, that must be placed before loop body.
    * Used as initialization block for loop resources.
    * @return pointer to block of three address codes or nullptr
    */
    virtual ThreeAddressCodeBlock init() = 0;
    /**
     * Returns block of three address codes, that is to be executed at the beginning of each iteration.
     * @return pointer to block of three address codes or nullptr
     */
    virtual ThreeAddressCodeBlock pre() = 0;
    /**
     * Returns block of three address codes, that is to be executed at the end of each iteration.
     * @return pointer to block of three address codes or nullptr
     */
    virtual ThreeAddressCodeBlock post() = 0;

    LoopBlock(): ControlFlowBlock(NestedSymbolTable()) {
        this->_loopLabel = genLabel();
        this->_bodyLabel = genLabel();
        this->_postBodyLabel = genLabel();
    }

    BaseBlock* body() {
        return _body;
    }

    void setBody(BaseBlock* blocks) {
        if (blocks != nullptr)
            blocks->setId(bodyLabel());
        this->_body = blocks;
    }

    LabelIdentifier loopLabel() {
        return _loopLabel;
    }

    LabelIdentifier bodyLabel() {
        return _bodyLabel;
    }

    LabelIdentifier postBodyLabel() {
        return _postBodyLabel;
    }

    std::list<ThreeAddressCodeBlock> flatten() override {
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
};

#endif /* ir_controlflowblock_hpp */