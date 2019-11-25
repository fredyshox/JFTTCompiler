//
// ir
// Created by Kacper Raczy on 2019-09-07.
//

#ifndef ir_threeaddresscode_hpp
#define ir_threeaddresscode_hpp

#include <optional>
#include <type_traits>
#include "Operand.hpp"

template<typename T>
struct GenericThreeAddressCode {
    static_assert(std::is_base_of<Operand, T>::value, "T must inherit from Operand");

    // MARK: Types
    enum Operator {
        LOAD, LOAD_IND, ADD, SUB, MUL, DIV, MOD, STDIN, STDOUT, JUMP, JPLUS, JMINUS, JZERO
    };

    static constexpr int OperatorNArgs[] = {2, 2, 3, 3, 3, 3, 3, 1, 1, 0, 0, 0, 0};

    // MARK: Fields
    std::unique_ptr<T> destination;
    Operator op;
    std::unique_ptr<T> firstOperand;
    std::unique_ptr<T> secondOperand;
    std::optional<LabelIdentifier> label;

    // MARK: Constructors
    GenericThreeAddressCode(T* dest, Operator op, T* operand0 = nullptr, T* operand1 = nullptr) {
        if (dest != nullptr)
            this->destination = dest->copy();
        if (operand0 != nullptr)
            this->firstOperand = operand0->copy();
        if (operand1 != nullptr)
            this->secondOperand = operand1->copy();

        this->op = op;
    }
    GenericThreeAddressCode(std::unique_ptr<T> destPtr, Operator op, std::unique_ptr<T> operandPtr0 = {}, std::unique_ptr<T> operandPtr1 = {}):
        destination(std::move(destPtr)),
        firstOperand(std::move(operandPtr0)),
        secondOperand(std::move(operandPtr1)) {
        this->op = op;
    }
    GenericThreeAddressCode(const GenericThreeAddressCode& tac) {
        this->op = tac.op;
        if (tac.destination)
            this->destination = tac.destination->copy();
        if (tac.firstOperand)
            this->firstOperand = tac.firstOperand->copy();
        if (tac.secondOperand)
            this->secondOperand = tac.secondOperand->copy();
    }
    GenericThreeAddressCode& operator=(const GenericThreeAddressCode &tac) {
        this->op = tac.op;
        if (tac.destination)
            this->destination = tac.destination->copy();
        if (tac.firstOperand)
            this->firstOperand = tac.firstOperand->copy();
        if (tac.secondOperand)
            this->secondOperand = tac.secondOperand->copy();
        return *this;
    }
    GenericThreeAddressCode() = delete;
    ~GenericThreeAddressCode() = default;
};

using ThreeAddressCode = GenericThreeAddressCode<Operand>;
using VRThreeAddressCode = GenericThreeAddressCode<VirtualRegisterOperand>;

//VRThreeAddressCode VRTACFromTAC(ThreeAddressCode& code) {
//    VirtualRegisterOperand* vdest = nullptr;
//    VirtualRegisterOperand* vop1 = nullptr;
//    VirtualRegisterOperand* vop2 = nullptr;
//    if ((code.destination != nullptr &&
//          (vdest = dynamic_cast<VirtualRegisterOperand*>(code.destination.get())) != nullptr) ||
//        (code.firstOperand != nullptr &&
//          (vop1 = dynamic_cast<VirtualRegisterOperand*>(code.firstOperand.get())) != nullptr) ||
//        (code.secondOperand != nullptr &&
//          (vop2 = dynamic_cast<VirtualRegisterOperand*>(code.secondOperand.get())) != nullptr)) {
//        throw "TAC to VRTAC conversion failed";
//    }
//
//    int opRaw = static_cast<int>(code.op);
//    auto op = static_cast<VRThreeAddressCode::Operator>(opRaw);
//    return VRThreeAddressCode(vdest, op, vop1, vop2);
//}

#endif /* ir_threeaddresscode_hpp */
