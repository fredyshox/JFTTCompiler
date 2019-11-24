//
// Created by Kacper Raczy on 2019-10-02.
//

#include "InstructionSelector.hpp"
#include <vector>
#include <unordered_map>
#include <stdexcept>

using Operator = ThreeAddressCode::Operator;

// Memory map
#define MM_ACC 0
// Temporary memory positions
#define MM_TEMP1 1
#define MM_TEMP2 2
#define MM_TEMP3 3

ThreeAddressCodeBlock* isaselector::expand(BaseBlock &program) {
    // flatten the program into tac blocks
}

void isaselector::simplify(ThreeAddressCodeBlock &block, SymbolTable& table) {
    std::unordered_map<uint64_t, MemoryPosition> vrMemoryPos;
    std::list<ThreeAddressCode>& codes = block.codes();
    MemoryPosition currentTemp = MM_TEMP1;

    auto it = codes.begin();
    while (it != codes.end()) {
        ThreeAddressCode& code = (*it);
        if (code.op == Operator::LOAD) {
            if (auto* vreg = dynamic_cast<VirtualRegisterOperand*>(code.destination.get())) {
                if (code.firstOperand->hasStaticMemoryPosition()) {
                    // vreg <- a/1/b[1]/v
                    MemoryPosition pos = code.firstOperand->memoryPosition(table);
                    vrMemoryPos[vreg->index] = pos;
                } else {
                    // vreg <- a[b]
                    // unroll
                    auto* aso = dynamic_cast<ArraySymbolOperand*>(code.firstOperand.get());
                    std::string memoryPosRecordName = "__m" + code.firstOperand->recordName();
                    Record memoryPosRecord = table.search(memoryPosRecordName);
                    std::string indexRecordName = aso->index->recordName();
                    Record indexRecord = table.search(indexRecordName);

                    auto v0 = VirtualRegisterOperand(MM_ACC);
                    auto vmb = VirtualRegisterOperand(memoryPosRecord.memoryPosition());
                    auto vi = VirtualRegisterOperand(indexRecord.memoryPosition());
                    auto vtemp = VirtualRegisterOperand(currentTemp);

                    std::vector<ThreeAddressCode> unroll = {
                            ThreeAddressCode(v0.copy(), Operator::ADD, vmb.copy(), vi.copy()),
                            ThreeAddressCode(v0.copy(), Operator::LOAD_IND, v0.copy()),
                            ThreeAddressCode(vtemp.copy(), Operator::LOAD, v0.copy())
                    };
                    codes.insert(it, unroll.begin(), unroll.end());
                    vrMemoryPos[vreg->index] = currentTemp;
                    ++currentTemp;
                }
                it = codes.erase(it);
                continue;
            } else {
                auto* vro = static_cast<VirtualRegisterOperand*>(code.firstOperand.get());
                auto v0 = VirtualRegisterOperand(MM_ACC);

                vro->index = vrMemoryPos[vro->index];
                if (code.destination->hasStaticMemoryPosition()) {
                    // a/a[1] <- vreg
                    MemoryPosition pos = code.destination->memoryPosition(table);
                    auto vdest = VirtualRegisterOperand(pos);
                    if (vro->index != 0) {
                        // v0 <- vro
                        // vdest <- v0
                        codes.insert(it, ThreeAddressCode(v0.copy(), Operator::LOAD, vro->copy()));
                        codes.insert(it, ThreeAddressCode(vdest.copy(), Operator::LOAD, v0.copy()));
                    } else {
                        codes.insert(it, ThreeAddressCode(vdest.copy(), Operator::LOAD, v0.copy()));
                    }
                } else {
                    // a[b] <- vreg
                    auto* aso = dynamic_cast<ArraySymbolOperand*>(code.destination.get());
                    std::string memoryPosRecordName = "__m" + code.destination->recordName();
                    Record memoryPosRecord = table.search(memoryPosRecordName);
                    std::string indexRecordName = aso->index->recordName();
                    Record indexRecord = table.search(indexRecordName);

                    auto vmb = VirtualRegisterOperand(memoryPosRecord.memoryPosition());
                    auto vi = VirtualRegisterOperand(indexRecord.memoryPosition());
                    auto vtemp1 = VirtualRegisterOperand(currentTemp++);
                    auto vtemp2 = VirtualRegisterOperand(currentTemp++);

                    std::vector<ThreeAddressCode> unroll;
                    if (vro->index == MM_ACC) {
                        unroll = {
                                ThreeAddressCode(vtemp1.copy(), Operator::LOAD, v0.copy()),
                                ThreeAddressCode(v0.copy(), Operator::ADD, vmb.copy(), vi.copy()),
                                ThreeAddressCode(vtemp2.copy(), Operator::LOAD, v0.copy()),
                                ThreeAddressCode(v0.copy(), Operator::LOAD, vtemp1.copy()),
                                ThreeAddressCode(vtemp2.copy(), Operator::LOAD_IND, v0.copy())
                        };
                    } else {
                        unroll = {
                                ThreeAddressCode(v0.copy(), Operator::ADD, vmb.copy(), vi.copy()),
                                ThreeAddressCode(vtemp1.copy(), Operator::LOAD, v0.copy()),
                                ThreeAddressCode(v0.copy(), Operator::LOAD, vro->copy()),
                                ThreeAddressCode(vtemp1.copy(), Operator::LOAD_IND, v0.copy())
                        };
                    }
                    codes.insert(it, unroll.begin(), unroll.end());
                }
                it = codes.erase(it);
                continue;
            }
        } else if (code.op == Operator::ADD || code.op == Operator::SUB) {
            auto* vreg = static_cast<VirtualRegisterOperand*>(code.destination.get());
            vrMemoryPos[vreg->index] = MM_ACC;
            vreg->index = MM_ACC;
            // wh about other
            auto* vop1 = static_cast<VirtualRegisterOperand*>(code.firstOperand.get());
            auto* vop2 = static_cast<VirtualRegisterOperand*>(code.secondOperand.get());
            vop1->index = vrMemoryPos[vop1->index];
            vop2->index = vrMemoryPos[vop2->index];
        } else if (code.op == Operator::STDIN) {
            auto* vreg = static_cast<VirtualRegisterOperand*>(code.destination.get());
            vrMemoryPos[vreg->index] = MM_ACC;
            vreg->index = MM_ACC;
        } else if (code.op == Operator::STDOUT) {
            auto* vreg = static_cast<VirtualRegisterOperand*>(code.destination.get());
            MemoryPosition srcIndex = vrMemoryPos[vreg->index];

            auto v0 = VirtualRegisterOperand(MM_ACC);
            auto vi = VirtualRegisterOperand(srcIndex);
            codes.insert(it, ThreeAddressCode(v0.copy(), Operator::LOAD, vi.copy()));

            vrMemoryPos[vreg->index] = MM_ACC;
            vreg->index = MM_ACC;
        }

        ++it;
    }

    // optimize tac block
}

void isaselector::match(ThreeAddressCodeBlock &block) {
    SymbolTable* table;
    // vregMap data structure for vreg value monitoring
    // vreg value can be not in symbol table
    // vRegPos => index -> memory pos
    // vRegCode => index -> assembly code
    AssemblyBlock asmBlock;
    std::unordered_map<int, MemoryPosition> vrMemoryPos;
    std::unordered_map<int, AssemblyBlock> vrBlocks;

    for (const ThreeAddressCode& tac : block.codes()) {
        if (tac.op == Operator::LOAD) {
//            if (auto* dest = dynamic_cast<VirtualRegisterOperand*>(tac.destination.get())) {
//                Operand* operand = tac.firstOperand.get();
//                if (operand->staticMemoryPosition()) {
//                    // check symbol table
//                    //vrBlocks[dest->index]
//                } else {
//                    vrBlocks[dest->index] = loadToP0(operand, *table);
//                }
//            }
        } else if (tac.op == Operator::STDIN) {
            asmBlock.push_back(Assembly::Get());
            // GET
            // STORE (table[tac.dest].memoryPosition())
        } else if (tac.op == Operator::STDOUT) {
            if (ConstantOperand* cop = dynamic_cast<ConstantOperand*>(tac.firstOperand.get())) {
                AssemblyBlock cBlock = loadToP0(cop->value);
                asmBlock.insert(asmBlock.end(), cBlock.begin(), cBlock.end());
                asmBlock.push_back(Assembly::Put());
            } else {
                // LOAD $operand
                // PUT
            }
        } else if (tac.op == Operator::ADD) {
            // LOAD $firstOperand
            // ADD $secondOperand
            // STORE $dest
        } else if (tac.op == Operator::SUB) {
            // LOAD $firstOperand
            // SUB $secondOperand
            // STORE $dest
        } else if (tac.op == Operator::MUL) {

        } else if (tac.op == Operator::DIV) {

        } else if (tac.op == Operator::MOD) {

        }
    }
}

AssemblyBlock isaselector::matchStdin(Operand* operand, SymbolTable& table) {
    AssemblyBlock block = { Assembly::Get() };
    if (operand->hasStaticMemoryPosition()) {
        MemoryPosition pos = operand->memoryPosition(table);
        block.push_back(Assembly::Store(pos));
    } else {
        AssemblyBlock loadBlock = storeP0(operand, table);
        block.insert(block.begin(), loadBlock.begin(), loadBlock.end());
    }

    return block;
}

AssemblyBlock isaselector::matchStdout(Operand* operand, SymbolTable& table) {
    AssemblyBlock block;
    if (operand->hasStaticMemoryPosition()) {
        MemoryPosition pos = operand->memoryPosition(table); // can throw
        block.push_back(Assembly::Load(pos));
    } else {
        AssemblyBlock loadBlock = loadToP0(operand, table);
        block.insert(block.begin(), loadBlock.begin(), loadBlock.end());
    }

    block.push_back(Assembly::Put());

    return block;
}

AssemblyBlock isaselector::loadToP0(int64_t value) {
    AssemblyBlock block;

    block.push_back(Assembly::Sub(0));
    if (value == 0) {
        return block;
    }

    Assembly inc = value > 0 ? Assembly::Inc() : Assembly::Dec();
    value = abs(value);

    if (value % 2 == 0) {
        block.push_back(inc);
        value = value - 1;
    }

    while (value != 0) {
        if (value % 2 != 0) {
            block.push_back(inc);
            value = value - 1;
        } else {
            block.push_back(Assembly::Add(0));
            value = value / 2;
        }
    }

    return block;
}

AssemblyBlock isaselector::loadToP0(Operand* operand, SymbolTable& table) noexcept(false) {
    if (operand->hasStaticMemoryPosition()) {
        throw std::invalid_argument("operand");
    }

    AssemblyBlock block;
    if (auto* cop = dynamic_cast<ConstantOperand*>(operand)) {
        block = loadToP0(cop->value);
    } else if (auto* asop = dynamic_cast<ArraySymbolOperand*>(operand)) {
        Record r = table.search(asop->symbol);
        MemoryPosition indexPos = asop->index->memoryPosition(table);
        block = loadToP0((int64_t) r.memoryPosition());
        block.push_back(Assembly::Add(indexPos));
        block.push_back(Assembly::Store(MM_TEMP1));
        block.push_back(Assembly::LoadI(MM_TEMP1));
    }

    return block;
}

AssemblyBlock isaselector::storeP0(Operand* operand, SymbolTable& table) noexcept(false) {
    if (operand->hasStaticMemoryPosition()) {
        throw std::invalid_argument("operand");
    }

    AssemblyBlock block;
    if (auto* asop = dynamic_cast<ArraySymbolOperand*>(operand)) {
        Record r = table.search(asop->symbol);
        MemoryPosition indexPos = asop->index->memoryPosition(table);
        block = loadToP0((int64_t) r.memoryPosition());
        block.push_back(Assembly::Add(indexPos));
        block.push_back(Assembly::Store(MM_TEMP1));
        block.push_back(Assembly::StoreI(MM_TEMP1));
    } else {
        throw std::invalid_argument("operand");
    }

    return block;
}

uint64_t numberGenerationCost(int64_t n) {
    n = abs(n);
    uint64_t cost = 0;

    if (n % 2 == 0) {
        n = n - 1;
        cost += 1;
    }

    while (n != 0) {
        if (n % 2 != 0) {
            // INC
            n = n - 1;
            cost += 1;
        } else {
            // add p0
            n = n / 2;
            cost += 10;
        }
    }

    return cost;
}
