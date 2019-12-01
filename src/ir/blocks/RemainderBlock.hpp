//
// Created by Kacper Raczy on 2019-12-01.
//

#ifndef ir_remainderblock_hpp
#define ir_remainderblock_hpp

#include "DivisionBlock.hpp"

class RemainderBlock: public DivisionBlock {
public:
    RemainderBlock(std::unique_ptr<Operand> dest,
                   std::unique_ptr<Operand> firstOperand,
                   std::unique_ptr<Operand> secondOperand,
                   SymbolTable* parentTable);
    RemainderBlock(Operand& dest,
                   Operand& firstOperand,
                   Operand& secondOperand,
                   SymbolTable* parentTable);
    ThreeAddressCodeBlock signAdjustment3() override;
    ThreeAddressCodeBlock returnValue() override;
};

#endif /* ir_remainderblock_hpp */
