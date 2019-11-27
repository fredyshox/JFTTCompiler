//
// Created by Kacper Raczy on 2019-11-27.
//

#ifndef ir_multiplicationblock_hpp
#define ir_multiplicationblock_hpp

#include "ControlFlowBlock.hpp"
#include <memory>

class MultiplicationBlock: public ControlFlowBlock {
private:
    LabelIdentifier _negativeInitLabel;
    LabelIdentifier _bodyLabel;
    LabelIdentifier _postBodyLabel;
    std::unique_ptr<Operand> _destination;
    std::unique_ptr<Operand> _firstOperand;
    std::unique_ptr<Operand> _secondOperand;
public:
    MultiplicationBlock(Operand& dest, Operand& firstOperand, Operand& secondOperand);
    /**
     * Block if tacs places before multiplication loop body
     * @return
     */
    ThreeAddressCodeBlock init();
    ThreeAddressCodeBlock negativeInit();
    ThreeAddressCodeBlock body();
    ThreeAddressCodeBlock postBody();
    LabelIdentifier negativeInitLabel();
    LabelIdentifier bodyLabel();
    LabelIdentifier postBodyLabel();
    std::list<ThreeAddressCodeBlock> flatten() override;
    // MARK: Temporary variables
    static std::string multiplierRecordName();
    static std::string multiplicandRecordName();
};

#endif /* ir_multiplicationblock_hpp */
