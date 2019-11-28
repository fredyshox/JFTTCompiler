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
    WhileLoopBlock(Condition condition, SymbolTable* parentTable);
    ThreeAddressCodeBlock init() override;
    ThreeAddressCodeBlock pre() override;
    ThreeAddressCodeBlock post() override;
    Condition& condition();
    void onIdChange(LabelIdentifier newId) override;
};

class DoWhileLoopBlock: public WhileLoopBlock {
public:
    DoWhileLoopBlock(Condition condition, SymbolTable* parentTable);
    ThreeAddressCodeBlock init() override;
    void onIdChange(LabelIdentifier newId) override;
};

#endif /* ir_whileloopblock_hpp */
