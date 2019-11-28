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
#define MM_TEMP4 4
#define MM_TEMP5 5

std::list<ThreeAddressCodeBlock> isaselector::expand(BaseBlock &program, GlobalSymbolTable& symbolTable) {
    // flatten the program into tac blocks
    std::list<ThreeAddressCodeBlock> flatTacs = BaseBlock::flattenBlockList(&program);

    // add all other permanent operands to symbol table and assign memory position.
    // this will add constants
    Operand* ccc[3] = {nullptr, nullptr, nullptr};
    for (ThreeAddressCodeBlock& tacBlock : flatTacs) {
        for (const ThreeAddressCode& tac : tacBlock.codes()) {
            ccc[0] = tac.destination.get();
            ccc[1] = tac.firstOperand.get();
            ccc[2] = tac.secondOperand.get();
            int nArgs = ThreeAddressCode::OperatorNArgs[static_cast<int>(tac.op)];
            for (int i = 0; i < nArgs && i < 3; i++) {
                if (!ccc[i]->isPermanent()) continue;

                std::string rname = ccc[i]->recordName();
                if (!symbolTable.contains(rname)) {
                    Record record = Record::integer(rname);
                    record.isConstant = true;
                    symbolTable.insert(rname, record);
                }

                if (auto aso = dynamic_cast<ArraySymbolOperand*>(ccc[i])) {
                    rname = aso->index->recordName();
                    if (!symbolTable.contains(rname)) {
                        Record record = Record::integer(rname);
                        record.isConstant = true;
                        symbolTable.insert(rname, record);
                    }
                }
            }
        }
    }

    // create memory map
    std::unordered_map<std::string, Record>& recordMap = symbolTable.allRecords();
    std::vector<std::string> keys(recordMap.size());
    auto keySelector = [](auto pair) { return pair.first; };
    std::transform(recordMap.begin(), recordMap.end(), keys.begin(), keySelector);
    MemoryPosition currentPos = MM_TEMP5 + 1;
    for (const std::string& key : keys) {
        Record& record = recordMap.at(key);
        record.offset = currentPos;
        currentPos += record.size;
        if (record.type == Record::ARRAY) {
            Record m = Record::memoryLocation(record.name);
            m.offset = currentPos++;
            symbolTable.insert(m.name, m);
        }
    }

    return flatTacs;
}

void isaselector::simplify(ThreeAddressCodeBlock &block, GlobalSymbolTable& table) {
    std::unordered_map<uint64_t, MemoryPosition> vrMemoryPos;
    std::list<ThreeAddressCode>& codes = block.codes();
    MemoryPosition currentTemp = MM_TEMP1;
    std::optional<MemoryPosition> lastDestinationVIndex = std::nullopt;

    auto it = codes.begin();
    while (it != codes.end()) {
        ThreeAddressCode& code = (*it);
        if (code.op == Operator::LOAD) {
            if (auto* vreg = dynamic_cast<VirtualRegisterOperand*>(code.destination.get())) {
                // vreg dest
                lastDestinationVIndex = vreg->index;
                if (code.firstOperand->hasStaticMemoryPosition()) {
                    // vreg <- a/1/b[1]/v
                    MemoryPosition pos = code.firstOperand->memoryPosition(table);
                    vrMemoryPos[vreg->index] = pos;
                } else {
                    // vreg <- a[b]
                    // unroll array operations
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
                // a/b[1]/b[a] dest
                auto* vro = static_cast<VirtualRegisterOperand*>(code.firstOperand.get());
                auto v0 = VirtualRegisterOperand(MM_ACC);
                lastDestinationVIndex = std::nullopt;

                vro->index = vrMemoryPos[vro->index];
                if (code.destination->hasStaticMemoryPosition()) {
                    // a/a[1] <- vreg
                    MemoryPosition pos = code.destination->memoryPosition(table);
                    auto vdest = VirtualRegisterOperand(pos);
                    if (vro->index != MM_ACC) {
                        // v0 <- vro
                        // vdest <- v0
                        codes.insert(it, ThreeAddressCode(v0.copy(), Operator::LOAD, vro->copy()));
                        codes.insert(it, ThreeAddressCode(vdest.copy(), Operator::LOAD, v0.copy()));
                    } else {
                        codes.insert(it, ThreeAddressCode(vdest.copy(), Operator::LOAD, v0.copy()));
                    }
                } else {
                    // a[b] <- vreg
                    // unroll array operations
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
        } else if (code.op == Operator::ADD || code.op == Operator::SUB || code.op == Operator::LSHIFT) {
            auto* vreg = static_cast<VirtualRegisterOperand*>(code.destination.get());
            vrMemoryPos[vreg->index] = MM_ACC;
            lastDestinationVIndex = vreg->index;
            vreg->index = MM_ACC;
            // wh about other
            auto* vop1 = static_cast<VirtualRegisterOperand*>(code.firstOperand.get());
            auto* vop2 = static_cast<VirtualRegisterOperand*>(code.secondOperand.get());
            lastDestinationVIndex = vreg->index;
            vop1->index = vrMemoryPos[vop1->index];
            vop2->index = vrMemoryPos[vop2->index];
        } else if (code.op == Operator::STDIN) {
            auto* vreg = static_cast<VirtualRegisterOperand*>(code.destination.get());
            vrMemoryPos[vreg->index] = MM_ACC;
            lastDestinationVIndex = vreg->index;
            vreg->index = MM_ACC;
        } else if (code.op == Operator::STDOUT) {
            auto* vreg = static_cast<VirtualRegisterOperand*>(code.destination.get());
            MemoryPosition srcIndex = vrMemoryPos[vreg->index];

            auto v0 = VirtualRegisterOperand(MM_ACC);
            auto vi = VirtualRegisterOperand(srcIndex);
            codes.insert(it, ThreeAddressCode(v0.copy(), Operator::LOAD, vi.copy()));

            vrMemoryPos[vreg->index] = MM_ACC;
            vreg->index = MM_ACC;
        } else if (code.op == Operator::JZERO || code.op == Operator::JPLUS || code.op == Operator::JMINUS) {
            if (lastDestinationVIndex.has_value() && vrMemoryPos[lastDestinationVIndex.value()] != MM_ACC) {
                auto v0 = VirtualRegisterOperand(MM_ACC);
                auto vi = VirtualRegisterOperand(vrMemoryPos[lastDestinationVIndex.value()]);
                codes.insert(it, ThreeAddressCode(v0.copy(), ThreeAddressCode::LOAD, vi.copy()));
            }
        }

        ++it;
    }
}

isaselector::ISAMatchFailed::ISAMatchFailed(std::string message): message("ISA Match failed with reason: " + message) {}

const char* isaselector::ISAMatchFailed::what() const throw() {
    return message.c_str();
}

#define ISAMatchIncorrectFormat ISAMatchFailed("Incorrect format (not simplified)")

#define else_throw_exception(exception) \
    else { \
        throw exception; \
    }

void isaselector::match(AssemblyBlock& asmBlock, JumpTable& jtable, ThreeAddressCodeBlock &block, GlobalSymbolTable& table) noexcept(false) {
    MemoryPosition mdest;
    MemoryPosition mop1;
    MemoryPosition mop2;
    LabelIdentifier ll;
    Operand* vvv[3] = {nullptr, nullptr, nullptr};
    MemoryPosition* mmm[3] = {&mdest, &mop1, &mop2};
    // offset in generated code for jump table
    uint64_t asmOffset = asmBlock.size();

    for (const ThreeAddressCode& tac : block.codes()) {
        vvv[0] = tac.destination.get();
        vvv[1] = tac.firstOperand.get();
        vvv[2] = tac.secondOperand.get();
        int nArgs = ThreeAddressCode::OperatorNArgs[static_cast<int>(tac.op)];
        for (int i = 0; i < nArgs && i < 3; i++) {
            if (vvv[i] == nullptr)
                throw ISAMatchFailed("Operand has too few operands!");

            *(mmm[i]) = vvv[i]->memoryPosition(table); // TODO expeption
        }
        if (nArgs == 0) {
            if (!tac.label.has_value())
                throw ISAMatchFailed("Operand has JUMP op and no label!");
            ll = tac.label.value();
        }

        switch (tac.op) {
            case Operator::LOAD:
                // load if dest v0
                // store if dest vi
                if (mdest == 0) {
                    asmBlock.push_back(Assembly::Load(mop1));
                } else if (mop1 == 0) {
                    asmBlock.push_back(Assembly::Store(mdest));
                } else_throw_exception(ISAMatchIncorrectFormat);
                break;
            case Operator::LOAD_IND:
                // loadi if dest v0
                // store if dest vx
                if (mdest == 0) {
                    asmBlock.push_back(Assembly::LoadI(mop1));
                } else if (mop1 == 0) {
                    asmBlock.push_back(Assembly::StoreI(mdest));
                } else_throw_exception(ISAMatchIncorrectFormat);
                break;
            case Operator::STDIN:
                asmBlock.push_back(Assembly::Get());
                break;
            case Operator::STDOUT:
                asmBlock.push_back(Assembly::Put());
                break;
            case Operator::ADD:
                // load vop1
                // add vop2
                if (mdest == 0) {
                    if (mop1 == 0) {
                        asmBlock.push_back(Assembly::Add(mop2));
                    } else if (mop2 == 0) {
                        asmBlock.push_back(Assembly::Add(mop1));
                    } else {
                        asmBlock.push_back(Assembly::Load(mop1));
                        asmBlock.push_back(Assembly::Add(mop2));
                    }
                } else_throw_exception(ISAMatchIncorrectFormat);
                break;
            case Operator::SUB:
                // load vop1
                // sub vop2
                if (mdest == 0) {
                    if (mop1 == 0) {
                        asmBlock.push_back(Assembly::Sub(mop2));
                    } else if (mop2 == 0) {
                        asmBlock.push_back(Assembly::Sub(mop1));
                    } else {
                        asmBlock.push_back(Assembly::Load(mop1));
                        asmBlock.push_back(Assembly::Sub(mop2));
                    }
                } else_throw_exception(ISAMatchIncorrectFormat);
                break;
            case Operator::LSHIFT:
                if (mdest == 0) {
                    if (mop1 == 0) {
                        asmBlock.push_back(Assembly::Shift(mop2));
                    } else if (mop2 == 0) {
                        asmBlock.push_back(Assembly::Shift(mop1));
                    } else {
                        asmBlock.push_back(Assembly::Load(mop1));
                        asmBlock.push_back(Assembly::Shift(mop2));
                    }
                } else_throw_exception(ISAMatchIncorrectFormat);
                break;
            case Operator::JUMP:
                // jump l
                asmBlock.push_back(Assembly::Jump(ll));
                break;
            case Operator::JMINUS:
                // jneg l
                asmBlock.push_back(Assembly::JNeg(ll));
                break;
            case Operator::JZERO:
                // jzero l
                asmBlock.push_back(Assembly::JZero(ll));
                break;
            case Operator::JPLUS:
                // jplus l
                asmBlock.push_back(Assembly::JPos(ll));
                break;
            default:
                throw ISAMatchFailed("Unsupported operation in isa match: " + std::to_string(tac.op));
        }
    }
    // apply assembly offset to jump table
    jtable[block.id()] = asmOffset;
}

void isaselector::initializationBlock(AssemblyBlock& asmBlock, GlobalSymbolTable& symbolTable) {
    std::unordered_map<std::string, Record>& recordMap = symbolTable.allRecords();
    for (auto& it : recordMap) {
        Record record = it.second;
        if (record.isMemoryLocation) {
            std::string name = record.name.substr(3);
            Record parentRecord = recordMap.at(name);
            // code gen
            loadValueToP0(asmBlock, parentRecord.memoryPosition());
            asmBlock.push_back(Assembly::Store(record.memoryPosition()));
        } else if (record.isConstant) {
            std::string name = record.name.substr(3);
            int64_t value = std::stoll(name);
            loadValueToP0(asmBlock, value);
            asmBlock.push_back(Assembly::Store(record.memoryPosition()));
        }
    }
}

void isaselector::terminationBlock(AssemblyBlock &asmBlock, JumpTable &jtable) {
    asmBlock.push_back(Assembly::Halt());
    jtable[LABEL_END] = asmBlock.size() - 1;
}

void isaselector::applyJumpTable(AssemblyBlock &asmBlock, JumpTable &jtable) noexcept(false) {
    for (Assembly& assembly : asmBlock) {
        std::string mnemonic = assembly.mnemonic();
        // find jumps
        if (mnemonic.size() != 0 && mnemonic[0] == 'J') {
            if (!assembly.argument().has_value()) {
                throw ISAMatchIncorrectFormat;
            }

            uint64_t argument = assembly.argument().value();
            if (jtable.find(argument) == jtable.end()) {
                // not found
                throw ISAMatchFailed("Jump location not found: " + std::to_string(argument));
            } else {
                assembly.setArgument(jtable[argument]);
            }
        }
    }
}

void isaselector::loadValueToP0(AssemblyBlock& asmBlock, int64_t value) {
    asmBlock.push_back(Assembly::Sub(0));
    if (value == 0) {
        return;
    }

    if (value > 0) {
        asmBlock.push_back(Assembly::Inc());
        asmBlock.push_back(Assembly::Store(MM_TEMP4));
        asmBlock.push_back(Assembly::Store(MM_TEMP5));
        asmBlock.push_back(Assembly::Dec());
    } else {
        asmBlock.push_back(Assembly::Inc());
        asmBlock.push_back(Assembly::Store(MM_TEMP5));
        asmBlock.push_back(Assembly::Dec());
        asmBlock.push_back(Assembly::Dec());
        asmBlock.push_back(Assembly::Store(MM_TEMP4));
        asmBlock.push_back(Assembly::Inc());
    }

    value = abs(value);
    while (value != 0) {
        if (value & 1) {
            asmBlock.push_back(Assembly::Add(MM_TEMP4));
        }

        value = value >> 1;

        if (value != 0) {
            asmBlock.push_back(Assembly::Store(MM_TEMP3));
            asmBlock.push_back(Assembly::Load(MM_TEMP4));
            asmBlock.push_back(Assembly::Shift(MM_TEMP5));
            asmBlock.push_back(Assembly::Store(MM_TEMP4));
            asmBlock.push_back(Assembly::Load(MM_TEMP3));
        }
    }
}
