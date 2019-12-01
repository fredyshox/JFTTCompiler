//
// Created by Kacper Raczy on 2019-09-09.
//

#ifndef ir_threeaddresscodeblock_hpp
#define ir_threeaddresscodeblock_hpp

#include <list>
#include <memory>
#include "BaseBlock.hpp"
#include "ThreeAddressCode.hpp"

#define JUMP_POSITIVE 1
#define JUMP_NEGATIVE -1
#define JUMP_ZERO 0
#define JUMP_ALWAYS 2

class ThreeAddressCodeBlock: public BaseBlock {
private:
    std::list<ThreeAddressCode> _codes;
public:
    explicit ThreeAddressCodeBlock(const std::list<ThreeAddressCode>& codes = {});

    uint64_t size();
    std::list<ThreeAddressCode>& codes();
    void merge(ThreeAddressCodeBlock& block);
    void append(ThreeAddressCode& code);
    std::list<ThreeAddressCodeBlock> flatten() override;
    // factory
    static ThreeAddressCodeBlock binaryOperation(Operand& dest,
                                                 Operand& op1,
                                                 Operand& op2,
                                                 ThreeAddressCode::Operator op,
                                                 uint64_t initialVIndex);
    static ThreeAddressCodeBlock binaryOperation(std::unique_ptr<Operand> dest,
                                                 std::unique_ptr<Operand> op1,
                                                 std::unique_ptr<Operand> op2,
                                                 ThreeAddressCode::Operator op,
                                                 uint64_t initialVIndex);
    static ThreeAddressCodeBlock addition(Operand& dest, Operand& op1, Operand& op2, uint64_t initialVIndex = 1);
    static ThreeAddressCodeBlock subtraction(Operand& dest, Operand& op1, Operand& op2, uint64_t initialVIndex = 1);
    static ThreeAddressCodeBlock multiplication(Operand& dest, Operand& op1, Operand& op2, uint64_t initialVIndex = 1);
    static ThreeAddressCodeBlock division(Operand& dest, Operand& op1, Operand& op2, uint64_t initialVIndex = 1);
    static ThreeAddressCodeBlock remainder(Operand& dest, Operand& op1, Operand& op2, uint64_t initialVIndex = 1);
    static ThreeAddressCodeBlock shift(Operand& dest, Operand& op1, Operand& op2, uint64_t initialVIndex = 1);
    static ThreeAddressCodeBlock read(Operand& dest, uint64_t initialVIndex = 1);
    static ThreeAddressCodeBlock write(Operand& source, uint64_t initialVIndex = 1);
    static ThreeAddressCodeBlock copy(Operand& dest, Operand& source, uint64_t initialVIndex = 1);
    static ThreeAddressCode jump(LabelIdentifier label, int jtype);
};

#endif /* ir_threeaddresscodeblock_hpp */
