//
// Created by Kacper Raczy on 2019-10-02.
//

#include "Assembly.hpp"

std::string Assembly::mnemonic() {
    return _mnemonic;
}

int64_t Assembly::argument() {
    return _argument;
}

void Assembly::setArgument(int64_t argument) {
    _argument = argument;
}

std::string Assembly::toString() {
    std::stringstream ss;
    ss << _mnemonic << " ";
    if (_argument != ASSEMBLY_ARGUMENT_NONE) {
        ss << _argument;
    }

    return ss.str();
}

Assembly Assembly::Put() {
    return Assembly("PUT", ASSEMBLY_ARGUMENT_NONE);
}

Assembly Assembly::Get() {
    return Assembly("GET", ASSEMBLY_ARGUMENT_NONE);
}

Assembly Assembly::Load(int64_t offset) {
    return Assembly("LOAD", offset);
}

Assembly Assembly::Store(int64_t offset) {
    return Assembly("STORE", offset);
}

Assembly Assembly::LoadI(int64_t offset) {
    return Assembly("LOADI", offset);
}

Assembly Assembly::StoreI(int64_t offset) {
    return Assembly("STOREI", offset);
}

Assembly Assembly::Add(int64_t offset) {
    return Assembly("ADD", offset);
}

Assembly Assembly::Sub(int64_t offset) {
    return Assembly("SUB", offset);
}

Assembly Assembly::Shift(int64_t offset) {
    return Assembly("SHIFT", offset);
}

Assembly Assembly::Inc() {
    return Assembly("INC", ASSEMBLY_ARGUMENT_NONE);
}

Assembly Assembly::Dec() {
    return Assembly("DEC", ASSEMBLY_ARGUMENT_NONE);
}

Assembly Assembly::Jump(int64_t pos) {
    return Assembly("JUMP", pos);
}

Assembly Assembly::JPos(int64_t pos) {
    return Assembly("JPOS", pos);
}

Assembly Assembly::JZero(int64_t pos) {
    return Assembly("JZERO", pos);
}

Assembly Assembly::JNeg(int64_t pos) {
    return Assembly("JNEG", pos);
}

Assembly Assembly::Halt() {
    return Assembly("HALT", ASSEMBLY_ARGUMENT_NONE);
}
