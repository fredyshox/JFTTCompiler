//
// Created by Kacper Raczy on 2019-09-09.
//

#ifndef ir_threeaddresscodeblock_hpp
#define ir_threeaddresscodeblock_hpp

#include <list>
#include "BaseBlock.hpp"
#include "ThreeAddressCode.hpp"

#define JUMP_POSITIVE 1
#define JUMP_NEGATIVE -1
#define JUMP_ZERO 0
#define JUMP_ALWAYS 2

class ThreeAddressCodeBlock: public BaseBlock {
private:
    std::list<ThreeAddressCode> _codes;
    static ThreeAddressCodeBlock binaryOperation(Operand& dest, Operand& op1, Operand& op2, ThreeAddressCode::Operator op);
public:
    explicit ThreeAddressCodeBlock(const std::list<ThreeAddressCode>& codes = {});

    std::list<ThreeAddressCode>& codes();
    void merge(ThreeAddressCodeBlock& block);
    void append(ThreeAddressCode& code);
    // baseBlock to tac
    static ThreeAddressCodeBlock* toTac(BaseBlock* baseBlock);
    // factory
    static ThreeAddressCodeBlock addition(Operand& dest, Operand& op1, Operand& op2);
    static ThreeAddressCodeBlock subtraction(Operand& dest, Operand& op1, Operand& op2);
    static ThreeAddressCodeBlock multiplication(Operand& dest, Operand& op1, Operand& op2);
    static ThreeAddressCodeBlock division(Operand& dest, Operand& op1, Operand& op2);
    static ThreeAddressCodeBlock remainder(Operand& dest, Operand& op1, Operand& op2);
    static ThreeAddressCodeBlock read(Operand& dest);
    static ThreeAddressCodeBlock write(Operand& source);
    static ThreeAddressCodeBlock copy(Operand& dest, Operand& source);
    static ThreeAddressCode jump(LabelIdentifier label, int jtype);
};

#endif /* ir_threeaddresscodeblock_hpp */
