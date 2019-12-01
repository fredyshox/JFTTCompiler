//
// Created by Kacper Raczy on 2019-09-25.
//

#include "Operand.hpp"
#include "Record.hpp"

NotInMemory::NotInMemory(std::string opName): message("Operand is not in memory: " + opName) {
    std::cout << opName << std::endl;
}

const char* NotInMemory::what() const throw() {
    return message.c_str();
}

std::unique_ptr<Operand> Operand::copy() const {
    return std::unique_ptr<Operand>(copyImpl());
}

bool Operand::hasStaticMemoryPosition() {
    return false;
}

MemoryPosition Operand::memoryPosition(GlobalSymbolTable &table) {
    throw NotInMemory("no name");
}

bool PermanentOperand::isPermanent() {
    return true;
}

ConstantOperand::ConstantOperand(const int value): PermanentOperand(), value(value) {}

ConstantOperand* ConstantOperand::copyImpl() const {
    return new ConstantOperand(this->value);
}

bool ConstantOperand::hasStaticMemoryPosition() {
    return true;
}

MemoryPosition ConstantOperand::memoryPosition(GlobalSymbolTable &table) {
    std::string name = recordName();
    if (table.containsAllRecords(name)) {
        return table.searchAllRecords(name).memoryPosition();
    } else {
        throw NotInMemory(name);
    }
}

std::string ConstantOperand::recordName() {
    return Record::constantName(value);
}

SymbolOperand::SymbolOperand(std::string symbol): PermanentOperand(), symbol(symbol) {}

SymbolOperand* SymbolOperand::copyImpl() const {
    return new SymbolOperand(std::string(this->symbol));
}

bool SymbolOperand::hasStaticMemoryPosition() {
    return true;
}

MemoryPosition SymbolOperand::memoryPosition(GlobalSymbolTable &table) {
    if (table.containsAllRecords(symbol)) {
        return table.searchAllRecords(symbol).memoryPosition();
    } else {
        throw NotInMemory(symbol);
    }
}

std::string SymbolOperand::recordName() {
    return symbol;
}

ArraySymbolOperand::ArraySymbolOperand(std::unique_ptr<Operand> indexPtr, std::string symbol): SymbolOperand(symbol), index(std::move(indexPtr)) {}

ArraySymbolOperand* ArraySymbolOperand::copyImpl() const {
    return new ArraySymbolOperand(index->copy(), this->symbol);
}

bool ArraySymbolOperand::hasStaticMemoryPosition() {
    return dynamic_cast<ConstantOperand*>(this->index.get()) != nullptr;
}

MemoryPosition ArraySymbolOperand::memoryPosition(GlobalSymbolTable &table) {
    if (!hasStaticMemoryPosition()) {
        throw NotInMemory(symbol);
    }

    ConstantOperand cop = *dynamic_cast<ConstantOperand*>(this->index.get());
    if (table.containsAllRecords(symbol)) {
        MemoryPosition base = table.searchAllRecords(symbol).memoryPosition();
        return base + cop.value;
    } else {
        throw NotInMemory(symbol);
    }
}

VirtualRegisterOperand::VirtualRegisterOperand(uint64_t index): Operand() {
    this->index = index;
}

VirtualRegisterOperand* VirtualRegisterOperand::copyImpl() const {
    return new VirtualRegisterOperand(this->index);
}

bool VirtualRegisterOperand::isPermanent() {
    return false;
}

bool VirtualRegisterOperand::hasStaticMemoryPosition() {
    return true;
}

MemoryPosition VirtualRegisterOperand::memoryPosition(GlobalSymbolTable &table) {
    return index;
}
std::string VirtualRegisterOperand::recordName() {
    return "__v" + std::to_string(index);
}