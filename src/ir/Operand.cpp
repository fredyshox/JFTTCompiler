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

NoConstantValue::NoConstantValue(std::string opName): message("Operand does not have constant value: " + opName) {}

const char* NoConstantValue::what() const throw() {
    return message.c_str();
}

std::unique_ptr<Operand> Operand::copy() const {
    return std::unique_ptr<Operand>(copyImpl());
}

bool Operand::hasStaticMemoryPosition() const {
    return false;
}

MemoryPosition Operand::memoryPosition(GlobalSymbolTable&) {
    throw NotInMemory("no name");
}

bool Operand::hasConstantValue() const {
    return false;
}

int64_t Operand::constantValue() noexcept(false) {
    throw NoConstantValue(recordName());
}

std::ostream& operator<<(std::ostream& stream, const Operand& operand) {
    stream << operand.recordName();
    return stream;
}

bool PermanentOperand::isPermanent() const {
    return true;
}

ConstantOperand::ConstantOperand(const int64_t value): PermanentOperand(), value(value) {}

ConstantOperand* ConstantOperand::copyImpl() const {
    return new ConstantOperand(this->value);
}

bool ConstantOperand::hasStaticMemoryPosition() const {
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

bool ConstantOperand::hasConstantValue() const {
    return true;
}

int64_t ConstantOperand::constantValue() noexcept(false) {
    return value;
}

std::string ConstantOperand::recordName() const {
    return Record::constantName(value);
}

SymbolOperand::SymbolOperand(std::string symbol): PermanentOperand(), symbol(symbol) {}

SymbolOperand* SymbolOperand::copyImpl() const {
    return new SymbolOperand(std::string(this->symbol));
}

bool SymbolOperand::hasStaticMemoryPosition() const {
    return true;
}

MemoryPosition SymbolOperand::memoryPosition(GlobalSymbolTable &table) {
    if (table.containsAllRecords(symbol)) {
        return table.searchAllRecords(symbol).memoryPosition();
    } else {
        throw NotInMemory(symbol);
    }
}

std::string SymbolOperand::recordName() const {
    return symbol;
}

ArraySymbolOperand::ArraySymbolOperand(std::unique_ptr<Operand> indexPtr, std::string symbol): SymbolOperand(symbol), index(std::move(indexPtr)) {}

ArraySymbolOperand* ArraySymbolOperand::copyImpl() const {
    return new ArraySymbolOperand(index->copy(), this->symbol);
}

bool ArraySymbolOperand::hasStaticMemoryPosition() const {
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

bool VirtualRegisterOperand::isPermanent() const {
    return false;
}

bool VirtualRegisterOperand::hasStaticMemoryPosition() const {
    return true;
}

MemoryPosition VirtualRegisterOperand::memoryPosition(GlobalSymbolTable&) {
    return index;
}
std::string VirtualRegisterOperand::recordName() const {
    return "__v" + std::to_string(index);
}