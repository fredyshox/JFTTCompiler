//
// ir
// SymbolTable.cpp
//

#include "SymbolTable.hpp"
#include <iostream>

RecordNotFound::RecordNotFound(const std::string key): message("RecordNotFound: Record not found for the key: " + key) {}

const char* RecordNotFound::what() const throw() {
    return message.c_str();
}

std::unordered_map<std::string, Record>& GlobalSymbolTable::allRecords() {
    return records;
}

void GlobalSymbolTable::notifyParent(Record& newRecord) {
    records.insert( {newRecord.name, newRecord} );
}

Record& GlobalSymbolTable::search(const std::string key) {
    try {
        return dict.at(key);
    } catch (const std::out_of_range &e) {
        throw RecordNotFound(key);
    }
}

Record& GlobalSymbolTable::searchAllRecords(std::string key) {
    try {
        return records.at(key);
    } catch (const std::out_of_range &e) {
        throw RecordNotFound(key);
    }
}

void GlobalSymbolTable::insert(const std::string key, Record value) {
    dict.insert( {key, value} );
    notifyParent(value);
}

bool GlobalSymbolTable::contains(std::string key) {
    return dict.find(key) != dict.end();
}

bool GlobalSymbolTable::containsAllRecords(std::string key) {
    return records.find(key) != records.end();
}

GlobalSymbolTable* GlobalSymbolTable::copy() const {
    GlobalSymbolTable* table = new GlobalSymbolTable();
    table->dict = std::unordered_map(this->dict);
    table->records = std::unordered_map(this->records);
    return table;
}

void NestedSymbolTable::notifyParent(Record& newRecord) {
    if (parent != nullptr) {
        parent->notifyParent(newRecord);
    }
}

Record& NestedSymbolTable::search(const std::string key) {
    try {
        Record& value = dict.at(key);
        return value;
    } catch (const std::out_of_range &e) {
        if (parent == nullptr) {
            throw RecordNotFound(key);
        }

        return parent->search(key);
    }
}

void NestedSymbolTable::insert(const std::string key, Record value) {
    dict.insert( { key, value} );
    notifyParent(value);
}

bool NestedSymbolTable::contains(const std::string key) {
    if (dict.find(key) == dict.end()) {
        if (parent == nullptr) {
            return false;
        }

        return parent->contains(key);
    } else {
        return true;
    }
}

NestedSymbolTable::NestedSymbolTable(SymbolTable *parent): SymbolTable() {
    this->parent = parent;
}

NestedSymbolTable* NestedSymbolTable::copy() const {
    NestedSymbolTable* table = new NestedSymbolTable(this->parent);
    table->dict = std::unordered_map(this->dict);
    return table;
}

void NestedSymbolTable::setParent(SymbolTable *table) {
    this->parent = table;
}