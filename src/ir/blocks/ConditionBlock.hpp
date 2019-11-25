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
    LabelIdentifier _postPassLabel;
    LabelIdentifier _failLabel;
protected:
    void adjustPassBodyLabels();
    void adjustFailBodyLabels();
public:
    ConditionBlock(Condition condition, BaseBlock* pass, BaseBlock* fail = nullptr);
    BaseBlock* passBody();
    void setPassBody(BaseBlock* block);
    BaseBlock* failBody();
    void setFailBody(BaseBlock* block);
    Condition& condition();
    ThreeAddressCodeBlock condBody();
    ThreeAddressCodeBlock postPassBody();
    LabelIdentifier passLabel();
    LabelIdentifier postPassLabel();
    LabelIdentifier failLabel();
    void onEndLabelChange(LabelIdentifier newLabel) override;
    std::list<ThreeAddressCodeBlock> flatten() override;
};

#endif /* ir_conditionblock_hpp */
