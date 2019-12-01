//
// Created by Kacper Raczy on 2019-11-28.
//

#ifndef ir_divisionblock_hpp
#define ir_divisionblock_hpp

#include "ControlFlowBlock.hpp"
#include <memory>

class DivisionBlock: public ControlFlowBlock {
protected:
    std::unique_ptr<Operand> _destination;
    std::unique_ptr<Operand> _firstOperand;
    std::unique_ptr<Operand> _secondOperand;
    LabelIdentifier _bitCountLabel;
    LabelIdentifier _body1Label;
    LabelIdentifier _body2Label;
    LabelIdentifier _postBodyLabel;
    LabelIdentifier _signAdjustment1Label;
    LabelIdentifier _signAdjustment2Label;
    LabelIdentifier _signAdjustment3Label;
    LabelIdentifier _returnValueLabel;
public:
    DivisionBlock(std::unique_ptr<Operand> dest,
                  std::unique_ptr<Operand> firstOperand,
                  std::unique_ptr<Operand> secondOperand,
                  SymbolTable* parentTable);
    DivisionBlock(Operand& dest,
                  Operand& firstOperand,
                  Operand& secondOperand,
                  SymbolTable* parentTable);
    virtual ThreeAddressCodeBlock init();
    virtual ThreeAddressCodeBlock signAdjustment1();
    virtual ThreeAddressCodeBlock signAdjustment2();
    virtual ThreeAddressCodeBlock bitCount();
    virtual ThreeAddressCodeBlock body1();
    virtual ThreeAddressCodeBlock body2();
    virtual ThreeAddressCodeBlock postBody();
    virtual ThreeAddressCodeBlock signAdjustment3();
    virtual ThreeAddressCodeBlock returnValue();
    std::list<ThreeAddressCodeBlock> flatten() override;
    static std::string quotientRecordName();
    static std::string remainderRecordName();
    static std::string dividentRecordName(); // a
    static std::string divisorRecordName(); // b
    static std::string tempRecordName(); // n3
    static std::string bitIndexRecordName(); // c
    static std::string signRecordName(); // s
};

#endif /* ir_divisionblock_hpp */
