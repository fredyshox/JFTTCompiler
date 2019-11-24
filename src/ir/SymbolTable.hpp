//
// ir
// Created by Kacper Raczy on 2019-09-06.
//

#ifndef ir_symboltable_hpp
#define ir_symboltable_hpp

#include <iostream>
#include <string>
#include <exception>
#include <sstream>
#include <unordered_map>
#include "Record.hpp"

class SymbolTable {
public:
    virtual ~SymbolTable() = default;
    virtual Record search(std::string key) = 0;
    virtual void insert(std::string key, Record value) = 0;
    virtual bool contains(std::string key) = 0;
    virtual SymbolTable* copy() const = 0;
};

struct RecordNotFound: public std::exception {
    const std::string key;
    explicit RecordNotFound(std::string key);
    const char* what() const throw() override;
};

class GlobalSymbolTable: public SymbolTable {
private:
    std::unordered_map<std::string, Record> dict;
public:
    void insert(std::string key, Record value) override;
    Record search(std::string key) override;
    bool contains(std::string key) override;
    GlobalSymbolTable* copy() const override;
};

class NestedSymbolTable: public SymbolTable {
private:
    std::unordered_map<std::string, Record> dict;
    SymbolTable* parent;
public:
    NestedSymbolTable(SymbolTable* parent = nullptr);
    void insert(std::string key, Record value) override;
    Record search(std::string key) override;
    bool contains(std::string key) override;
    NestedSymbolTable* copy() const override;
    void setParent(SymbolTable* table);
};

#endif /* ir_symboltable_hpp */