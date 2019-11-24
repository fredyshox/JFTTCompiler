//
// Created by Kacper Raczy on 2019-09-09.
//

#ifndef ir_conditionblock_hpp
#define ir_conditionblock_hpp

#include <cassert>
#include "Condition.hpp"
#include "ControlFlowBlock.hpp"

/**
 * [ COND ]
 * - passLabel
 * [ PASS ]
 * - failLabel
 * [ FAIL ]
 */
class ConditionBlock: public ControlFlowBlock {
private:
    Condition _condition;
    BaseBlock* _pass;
    BaseBlock* _fail;
    LabelIdentifier _passLabel;
    LabelIdentifier _failLabel;
public:
    ConditionBlock(Condition condition, BaseBlock* pass, BaseBlock* fail = nullptr);
    BaseBlock* passBody();
    void setPassBody(BaseBlock* block);
    BaseBlock* failBody();
    void setFailBody(BaseBlock* block);
    Condition& condition();
    ThreeAddressCodeBlock condBody();
    LabelIdentifier passLabel();
    LabelIdentifier failLabel();
};

#endif /* ir_conditionblock_hpp */
