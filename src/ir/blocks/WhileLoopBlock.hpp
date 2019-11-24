//
// Created by Kacper Raczy on 2019-09-09.
//

#ifndef ir_whileloopblock_hpp
#define ir_whileloopblock_hpp

#include "ControlFlowBlock.hpp"
#include "ThreeAddressCodeBlock.hpp"
#include "Condition.hpp"

/**
 * Structure:
 * [ INIT ]
 * - loopLabel
 * [ PRE ]
 * - bodyLabel
 * [ BODY ]
 * [ POST ]
 * - endLabel
 */
class WhileLoopBlock: public LoopBlock {
private:
    Condition _condition;
public:
    WhileLoopBlock(Condition condition);
    ThreeAddressCodeBlock init() override;
    ThreeAddressCodeBlock pre() override;
    ThreeAddressCodeBlock post() override;
    Condition& condition();
};

class DoWhileLoopBlock: public WhileLoopBlock {
public:
    DoWhileLoopBlock(Condition condition);
    ThreeAddressCodeBlock init() override;
};

#endif /* ir_whileloopblock_hpp */
