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
    MultiplicationBlock(std::unique_ptr<Operand> dest,
                        std::unique_ptr<Operand> op1,
                        std::unique_ptr<Operand> op2,
                        SymbolTable* parentTable);
    MultiplicationBlock(Operand& dest,
                        Operand& firstOperand,
                        Operand& secondOperand,
                        SymbolTable* parentTable);
    /**
     * Block of tacs places before multiplication loop body
     * @return tacBlock
     */
    ThreeAddressCodeBlock init();
    /**
     * Block that adjust values for negativity
     * @return tacBlock
     */
    ThreeAddressCodeBlock negativeInit();
    /**
     * Multiplication loop body
     * @return tacBlock
     */
    ThreeAddressCodeBlock body();
    /**
     * Post multiplication body, preparation for next iteration
     * @return tacBlock
     */
    ThreeAddressCodeBlock postBody();
    std::list<ThreeAddressCodeBlock> flatten() override;
    // MARK: Labels
    LabelIdentifier negativeInitLabel();
    LabelIdentifier bodyLabel();
    LabelIdentifier postBodyLabel();
    // MARK: Temporary variables
    static std::string multiplierRecordName();
    static std::string multiplicandRecordName();
};

#endif /* ir_multiplicationblock_hpp */
