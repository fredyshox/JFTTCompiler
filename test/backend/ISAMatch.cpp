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
#include <unordered_set>

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

TEST(ISAMatch, SymbolTableConstants1) {
    ConstantOperand c10u(10);
    ConstantOperand c71s(-71);
    SymbolOperand a("a");
    ThreeAddressCodeBlock block = ThreeAddressCodeBlock::addition(a, c10u, c71s);

    GlobalSymbolTable st;
    st.insert("a", Record::integer("a"));

    auto res = isaselector::expand(block, st);
    EXPECT_EQ(st.allRecords().size(), 3); // a, 10, -71
    EXPECT_TRUE(st.contains(c10u.recordName()));
    EXPECT_TRUE(st.contains(c71s.recordName()));
}

TEST(ISAMatch, SymbolTableConstants2) {
    ConstantOperand c10u(10);
    ConstantOperand c71s(-71);
    SymbolOperand a("a");
    ArraySymbolOperand b(c10u.copy(), "b");
    ThreeAddressCodeBlock block = ThreeAddressCodeBlock::addition(b, a, c71s);

    GlobalSymbolTable st;
    st.insert("a", Record::integer("a"));
    st.insert("b", Record::array("b", {5, 15}));

    auto res = isaselector::expand(block, st);
    EXPECT_EQ(st.allRecords().size(), 5); // a, b, __mb, 10, -72
    EXPECT_TRUE(st.contains(c10u.recordName()));
    EXPECT_TRUE(st.contains(c71s.recordName()));
}

TEST(ISAMatch, MemoryMapOffsets) {
    ConstantOperand c10u(10);
    ConstantOperand c71s(-71);
    SymbolOperand a("a");
    ArraySymbolOperand b(c10u.copy(), "b");
    ThreeAddressCodeBlock block = ThreeAddressCodeBlock::addition(b, a, c71s);

    GlobalSymbolTable st;
    st.insert("a", Record::integer("a"));
    st.insert("b", Record::array("b", {5, 15}));

    auto res = isaselector::expand(block, st);
    auto allRecords = st.allRecords();
    unordered_set<MemoryPosition> offsets;
    for (auto it : allRecords) {
        Record record = it.second;
        ASSERT_TRUE(offsets.find(record.offset) == offsets.end());
        offsets.insert(record.offset);
    }

    // check all array elements
    for (auto it : allRecords) {
        Record record = it.second;
        if (record.type == Record::ARRAY) {
            for (int64_t i = 1; i < record.size; i++) {
                ASSERT_TRUE(offsets.find(record.offset + i) == offsets.end());
            }
        }
    }
}

void testNumberGenerator(int64_t expected, AssemblyBlock& asmBlock) {
    int64_t mem[10];
    memset(mem, 0, 10);
    mem[5] = 1;
    for (Assembly& a : asmBlock) {
        if (a.mnemonic() == "ADD") {
            mem[0] = mem[0] + mem[a.argument().value()];
        } else if (a.mnemonic() == "SUB") {
            mem[0] = mem[0] - mem[a.argument().value()];
        } else if (a.mnemonic() == "INC") {
            mem[0] += 1;
        } else if (a.mnemonic() == "DEC") {
            mem[0] -= 1;
        } else if (a.mnemonic() == "SHIFT") {
            mem[0] <<= mem[a.argument().value()];
        } else if (a.mnemonic() == "LOAD") {
            mem[0] = mem[a.argument().value()];
        } else if (a.mnemonic() == "STORE") {
            mem[a.argument().value()] = mem[0];
        }
    }

    ASSERT_EQ(expected, mem[0]);
}

TEST(ISAMatch, NumberGenerator1) {
    AssemblyBlock block;
    isaselector::loadValueToP0(block, 10, 5);
    testNumberGenerator(10, block);
}

TEST(ISAMatch, NumberGenerator2) {
    AssemblyBlock block;
    isaselector::loadValueToP0(block, 31, 5);
    testNumberGenerator(31, block);
}

TEST(ISAMatch, NumberGenerator3) {
    AssemblyBlock block;
    isaselector::loadValueToP0(block, -71, 5);
    testNumberGenerator(-71, block);
}

TEST(ISAMatch, NumberGenerator4) {
    AssemblyBlock block;
    isaselector::loadValueToP0(block, -27, 5);
    testNumberGenerator(-27, block);
}

TEST(ISAMatch, NumberGenerator5) {
    AssemblyBlock block;
    isaselector::loadValueToP0(block, 987, 5);
    testNumberGenerator(987, block);
}

TEST(ISAMatch, NumberGenerator6) {
    AssemblyBlock block;
    isaselector::loadValueToP0(block, -512, 5);
    testNumberGenerator(-512, block);
}

TEST(ISAMatch, NumberGenerator7) {
    AssemblyBlock block;
    isaselector::loadValueToP0(block, 2569, 5);
    testNumberGenerator(2569, block);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
