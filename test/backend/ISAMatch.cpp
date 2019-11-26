//
// Created by Kacper Raczy on 2019-11-23.
//

#include "gtest/gtest.h"
#include "InstructionSelector.hpp"
#include "blocks/ThreeAddressCodeBlock.hpp"
#include "ThreeAddressCode.hpp"
#include "Operand.hpp"
#include <memory>
#include <sstream>
#include <exception>

using namespace std;

std::string printToString(AssemblyBlock& block) {
    std::stringstream ss;
    for (Assembly& a : block) {
        ss << a.toString() << "\n";
    }

    return ss.str();
}

#define VR(i) std::make_unique<VirtualRegisterOperand>(i)

TEST(ISAMatch, Addition) {
    auto block = ThreeAddressCodeBlock({
        ThreeAddressCode(VR(0), ThreeAddressCode::ADD, VR(1), VR(2)),
        ThreeAddressCode(VR(1), ThreeAddressCode::LOAD, VR(0))
    });

    GlobalSymbolTable st;
    JumpTable jt;
    AssemblyBlock res;
    try {
        isaselector::match(res, jt, block, st);
    } catch (isaselector::ISAMatchFailed& ex) {
        cout << "Exception Message: " << ex.what() << endl;
        FAIL();
    } catch (exception& ex) {
        FAIL();
    }

    EXPECT_EQ(printToString(res), "LOAD 1\n"
                                  "ADD 2\n"
                                  "STORE 1\n");
}

TEST(ISAMatch, Subtraction) {
    auto block = ThreeAddressCodeBlock({
        ThreeAddressCode(VR(0), ThreeAddressCode::SUB, VR(1), VR(2)),
        ThreeAddressCode(VR(1), ThreeAddressCode::LOAD, VR(0))
    });

    GlobalSymbolTable st;
    JumpTable jt;
    AssemblyBlock res;
    try {
        isaselector::match(res, jt, block, st);
    } catch (isaselector::ISAMatchFailed& ex) {
        cout << "Exception Message: " << ex.what() << endl;
        FAIL();
    } catch (exception& ex) {
        FAIL();
    }

    EXPECT_EQ(printToString(res), "LOAD 1\n"
                                  "SUB 2\n"
                                  "STORE 1\n");
}

TEST(ISAMatch, IndirectMemory) {
    auto block = ThreeAddressCodeBlock({
        ThreeAddressCode(VR(0), ThreeAddressCode::LOAD_IND, VR(1)),
        ThreeAddressCode(VR(1), ThreeAddressCode::LOAD_IND, VR(0))
    });

    GlobalSymbolTable st;
    JumpTable jt;
    AssemblyBlock res;
    try {
        isaselector::match(res, jt, block, st);
    } catch (isaselector::ISAMatchFailed& ex) {
        cout << "Exception Message: " << ex.what() << endl;
        FAIL();
    } catch (exception& ex) {
        FAIL();
    }

    EXPECT_EQ(printToString(res), "LOADI 1\n"
                                  "STOREI 1\n");
}

TEST(ISAMatch, JumpTableFix) {
    auto block1 = ThreeAddressCodeBlock({
        ThreeAddressCodeBlock::jump(1, JUMP_ALWAYS),
        ThreeAddressCodeBlock::jump(1, JUMP_ZERO),
        ThreeAddressCodeBlock::jump(1, JUMP_POSITIVE),
        ThreeAddressCodeBlock::jump(1, JUMP_NEGATIVE)
    });
    block1.setId(0);
    auto block2 = ThreeAddressCodeBlock({
        ThreeAddressCode(VR(1), ThreeAddressCode::LOAD, VR(0))
    });
    block2.setId(1);

    GlobalSymbolTable st;
    JumpTable jt;
    AssemblyBlock res;
    try {
        isaselector::match(res, jt, block1, st);
        isaselector::match(res, jt, block2, st);
    } catch (isaselector::ISAMatchFailed& ex) {
        cout << "Exception Message: " << ex.what() << endl;
        FAIL();
    } catch (exception& ex) {
        FAIL();
    }

    EXPECT_EQ(jt[1], 4);
    isaselector::applyJumpTable(res, jt);
    EXPECT_EQ(printToString(res), "JUMP 4\n"
                                  "JZERO 4\n"
                                  "JPOS 4\n"
                                  "JNEG 4\n"
                                  "STORE 1\n");
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
