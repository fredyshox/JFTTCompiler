//
// Created by Kacper Raczy on 2019-10-02.
//

#include "Assembly.hpp"

std::string Assembly::mnemonic() const {
    return _mnemonic;
}

std::optional<uint64_t> Assembly::argument() const {
    return _argument;
}

void Assembly::setArgument(std::optional<uint64_t> argument) {
    _argument = argument;
}

std::string Assembly::toString() const {
    std::stringstream ss;
    ss << _mnemonic << " ";
    if (_argument.has_value()) {
        ss << _argument.value();
    }

    return ss.str();
}

Assembly Assembly::Put() {
    return Assembly("PUT");
}

Assembly Assembly::Get() {
    return Assembly("GET");
}

Assembly Assembly::Load(uint64_t offset) {
    return Assembly("LOAD", offset);
}

Assembly Assembly::Store(uint64_t offset) {
    return Assembly("STORE", offset);
}

Assembly Assembly::LoadI(uint64_t offset) {
    return Assembly("LOADI", offset);
}

Assembly Assembly::StoreI(uint64_t offset) {
    return Assembly("STOREI", offset);
}

Assembly Assembly::Add(uint64_t offset) {
    return Assembly("ADD", offset);
}

Assembly Assembly::Sub(uint64_t offset) {
    return Assembly("SUB", offset);
}

Assembly Assembly::Shift(uint64_t offset) {
    return Assembly("SHIFT", offset);
}

Assembly Assembly::Inc() {
    return Assembly("INC");
}

Assembly Assembly::Dec() {
    return Assembly("DEC");
}

Assembly Assembly::Jump(uint64_t pos) {
    return Assembly("JUMP", pos);
}

Assembly Assembly::JPos(uint64_t pos) {
    return Assembly("JPOS", pos);
}

Assembly Assembly::JZero(uint64_t pos) {
    return Assembly("JZERO", pos);
}

Assembly Assembly::JNeg(uint64_t pos) {
    return Assembly("JNEG", pos);
}

Assembly Assembly::Halt() {
    return Assembly("HALT");
}
