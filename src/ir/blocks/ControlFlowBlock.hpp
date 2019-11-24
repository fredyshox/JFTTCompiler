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
    LabelIdentifier _endLabel;
    BaseBlock* _body = nullptr;
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
        this->_endLabel = genLabel();
        this->_bodyLabel = genLabel();
    }

    BaseBlock* body() {
        return _body;
    }

    void setBody(BaseBlock* blocks) {
        this->_body = blocks;
    }

    LabelIdentifier loopLabel() {
        return _loopLabel;
    }

    LabelIdentifier endLabel() {
        return _endLabel;
    }

    LabelIdentifier bodyLabel() {
        return _bodyLabel;
    }
};

#endif /* ir_controlflowblock_hpp */