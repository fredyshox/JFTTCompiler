//
// Created by Kacper Raczy on 2019-09-09.
//

#ifndef ir_forloopblock_hpp
#define ir_forloopblock_hpp

#include <string>
#include <cassert>
#include "ControlFlowBlock.hpp"
#include "ThreeAddressCodeBlock.hpp"
#include "Operand.hpp"
#include "Condition.hpp"

struct LoopRange {
    std::unique_ptr<Operand> initial;
    std::unique_ptr<Operand> bound;
    int step;

    LoopRange(Operand& init, Operand& bound, int step);
    LoopRange(std::unique_ptr<Operand>& initPtr, std::unique_ptr<Operand>& boundPtr, int step);
    LoopRange(const LoopRange& range);
    LoopRange& operator=(const LoopRange& range);
    ~LoopRange() = default;
};

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
class ForLoopBlock: public LoopBlock {
private:
    std::string _iteratorName;
    std::string _counterName;
    LoopRange _loopRange;
public:
    ForLoopBlock(std::string iterator, LoopRange range);
    ForLoopBlock() = delete;
    ~ForLoopBlock() override = default;
    ThreeAddressCodeBlock init() override;
    ThreeAddressCodeBlock pre() override;
    ThreeAddressCodeBlock post() override;
    std::string iteratorName();
    std::string counterName();
    LoopRange& loopRange();
};

#endif /* ir_forloopblock_hpp */
