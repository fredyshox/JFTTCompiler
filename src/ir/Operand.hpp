//
// Created by Kacper Raczy on 2019-09-08.
//

#ifndef ir_operand_hpp
#define ir_operand_hpp

// constant
// symbol - string
// virtual register

#include <string>
#include <exception>
#include "SymbolTable.hpp"

struct NotInMemory: public std::exception {
    const std::string message;
    explicit NotInMemory(std::string opName);
    const char* what() const throw() override;
};

struct NoConstantValue: public std::exception {
    const std::string message;
    explicit NoConstantValue(std::string opName);
    const char* what() const throw() override;
};

class Operand {
private:
    Operand() = default;
public:
    virtual ~Operand() = default;
    std::unique_ptr<Operand> copy() const;
    virtual bool isPermanent() = 0;
    virtual bool hasStaticMemoryPosition();
    virtual MemoryPosition memoryPosition(GlobalSymbolTable& table) noexcept(false);
    virtual bool hasConstantValue();
    virtual int64_t constantValue() noexcept(false);
    virtual std::string recordName() = 0;

    friend class PermanentOperand;
    friend class VirtualRegisterOperand;
protected:
    virtual Operand* copyImpl() const = 0;
};

class PermanentOperand: public Operand {
public:
    bool isPermanent() override;
};

class ConstantOperand: public PermanentOperand {
public:
    const int64_t value;

    ConstantOperand(int64_t value);
    ConstantOperand() = delete;
    bool hasStaticMemoryPosition() override;
    MemoryPosition memoryPosition(GlobalSymbolTable& table) noexcept(false) override;
    bool hasConstantValue() override;
    int64_t constantValue() noexcept(false) override;
    std::string recordName() override;
protected:
    ConstantOperand* copyImpl() const override;
};

class SymbolOperand: public PermanentOperand {
public:
    const std::string symbol;

    SymbolOperand(std::string symbol);
    SymbolOperand() = delete;
    bool hasStaticMemoryPosition() override;
    MemoryPosition memoryPosition(GlobalSymbolTable& table) noexcept(false) override;
    std::string recordName() override;
protected:
    SymbolOperand* copyImpl() const override;
};

class ArraySymbolOperand: public SymbolOperand {
public:
    std::unique_ptr<Operand> index;

    ArraySymbolOperand(std::unique_ptr<Operand> indexPtr, std::string symbol);
    ArraySymbolOperand() = delete;
    bool hasStaticMemoryPosition() override;
    MemoryPosition memoryPosition(GlobalSymbolTable& table) noexcept(false) override;
protected:
    ArraySymbolOperand* copyImpl() const override;
};

class VirtualRegisterOperand: public Operand {
public:
    uint64_t index;

    VirtualRegisterOperand(uint64_t index);
    VirtualRegisterOperand() = delete;
    bool isPermanent() override;
    bool hasStaticMemoryPosition() override;
    MemoryPosition memoryPosition(GlobalSymbolTable& table) noexcept(false) override;
    std::string recordName() override;
protected:
    VirtualRegisterOperand* copyImpl() const override;
};

#endif /* ir_operand_hpp */
