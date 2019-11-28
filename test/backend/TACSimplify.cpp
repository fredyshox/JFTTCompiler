//
// Created by Kacper Raczy on 2019-11-23.
//


#include "gtest/gtest.h"
#include "InstructionSelector.hpp"
#include <sstream>

class TACSimplify: public ::testing::Test {
protected:
    GlobalSymbolTable* table;
    ConstantOperand* c10;
    ConstantOperand* c72;

    void SetUp() override {
        c10 = new ConstantOperand(10);
        c72 = new ConstantOperand(72);
        table = new GlobalSymbolTable();
        MemoryPosition init = 3;

        table->insert("a", Record("a", Record::INT, 1, std::nullopt, init++)); // 3
        table->insert("b", Record("b", Record::INT, 1, std::nullopt, init++)); // 4
        table->insert("c", Record("c", Record::INT, 1, std::nullopt, init++)); // 5
        table->insert("d", Record("d", Record::ARRAY, 10, std::make_tuple(0, 9), init++)); // 6
        table->insert(c10->recordName(), Record(c10->recordName(), Record::INT, 1, std::nullopt, init++)); // 7
        table->insert(c72->recordName(), Record(c72->recordName(), Record::INT, 1, std::nullopt, init++)); // 8
        table->insert("__md", Record("__md", Record::INT, 1, std::nullopt, init)); // 9
    }

    void testVRTacs(ThreeAddressCode& code, std::vector<MemoryPosition> pos) {
        if (code.op == ThreeAddressCode::LOAD ||
            code.op == ThreeAddressCode::LOAD_IND) {
            ASSERT_EQ(pos.size(), (uint64_t) 2);
        } else if (code.op == ThreeAddressCode::ADD ||
                   code.op == ThreeAddressCode::SUB ||
                   code.op == ThreeAddressCode::MUL ||
                   code.op == ThreeAddressCode::DIV ||
                   code.op == ThreeAddressCode::MOD) {
            ASSERT_EQ(pos.size(), (uint64_t) 3);
        } else if (code.op == ThreeAddressCode::STDIN ||
                   code.op == ThreeAddressCode::STDOUT) {
            ASSERT_EQ(pos.size(), (uint64_t) 1);
        } else {
            return;
        }

        Operand* operands[3] = {code.destination.get(), code.firstOperand.get(), code.secondOperand.get()};
        for (unsigned long i = 0; i < pos.size(); i++) {
            auto* vreg = dynamic_cast<VirtualRegisterOperand*>(operands[i]);
            ASSERT_TRUE(vreg != nullptr);
            EXPECT_EQ(vreg->index, pos[i]);
        }
    }

    void TearDown() override {
        delete c10;
        delete c72;
        delete table;
    }
};

TEST_F(TACSimplify, BinaryInts) {
    SymbolOperand a("a");
    SymbolOperand b("b");
    SymbolOperand c("c");
    ThreeAddressCodeBlock block = ThreeAddressCodeBlock::addition(c, a, b);
    isaselector::simplify(block, *table);

    ASSERT_EQ(block.codes().size(), 2);
    ThreeAddressCode& tac1 = *block.codes().begin();
    ThreeAddressCode& tac2 = *std::next(block.codes().begin());
    // tac1
    ASSERT_EQ(tac1.op, ThreeAddressCode::ADD);
    testVRTacs(tac1, {0, 3, 4});
    // tac2
    ASSERT_EQ(tac2.op, ThreeAddressCode::LOAD);
    testVRTacs(tac2, {5, 0});
}

TEST_F(TACSimplify, BinaryIntsAndConst) {
    SymbolOperand a("a");
    SymbolOperand c("c");
    ThreeAddressCodeBlock block = ThreeAddressCodeBlock::addition(c, a, *c10);
    isaselector::simplify(block, *table);

    ASSERT_EQ(block.codes().size(), 2);
    ThreeAddressCode& tac1 = *block.codes().begin();
    ThreeAddressCode& tac2 = *std::next(block.codes().begin());
    // tac1
    ASSERT_EQ(tac1.op, ThreeAddressCode::ADD);
    testVRTacs(tac1, {0, 3, 7});
    // tac2
    ASSERT_EQ(tac2.op, ThreeAddressCode::LOAD);
    testVRTacs(tac2, {5, 0});
}

TEST_F(TACSimplify, BinaryArraySrc) {
    ConstantOperand c5(5);
    ArraySymbolOperand d(c5.copy(), "d");
    SymbolOperand a("a");
    SymbolOperand c("c");
    ThreeAddressCodeBlock block = ThreeAddressCodeBlock::subtraction(c, a, d);
    isaselector::simplify(block, *table);

    ASSERT_EQ(block.codes().size(), 2);
    ThreeAddressCode& tac1 = *block.codes().begin();
    ThreeAddressCode& tac2 = *std::next(block.codes().begin());
    // tac1
    ASSERT_EQ(tac1.op, ThreeAddressCode::SUB);
    testVRTacs(tac1, {0, 3, 11});
    // tac2
    ASSERT_EQ(tac2.op, ThreeAddressCode::LOAD);
    testVRTacs(tac2, {5, 0});
}

TEST_F(TACSimplify, BinaryArrayAmbiguousSrc) {
    SymbolOperand a("a");
    ArraySymbolOperand da(a.copy(), "d");
    SymbolOperand b("b");
    ArraySymbolOperand db(b.copy(), "d");
    SymbolOperand c("c");
    ThreeAddressCodeBlock block = ThreeAddressCodeBlock::subtraction(c, da, db);
    isaselector::simplify(block, *table);

    ASSERT_EQ(block.codes().size(), 8);
    int i = 0;
    std::vector<std::vector<MemoryPosition>> mmm = {
            {0, 9, 3},
            {0, 0},
            {1, 0},
            {0, 9, 4},
            {0, 0},
            {2, 0},
            {0, 1, 2},
            {5, 0}
    };
    std::vector<ThreeAddressCode::Operator> ooo = {
            ThreeAddressCode::ADD,
            ThreeAddressCode::LOAD_IND,
            ThreeAddressCode::LOAD,
            ThreeAddressCode::ADD,
            ThreeAddressCode::LOAD_IND,
            ThreeAddressCode::LOAD,
            ThreeAddressCode::SUB,
            ThreeAddressCode::LOAD
    };
    for (ThreeAddressCode& tac : block.codes()) {
        ASSERT_EQ(tac.op, ooo[i]);
        testVRTacs(tac, mmm[i]);

        ++i;
    }
}

TEST_F(TACSimplify, BinaryArrayDest) {
    SymbolOperand a("a");
    ArraySymbolOperand da(a.copy(), "d");
    SymbolOperand c("c");
    ThreeAddressCodeBlock block = ThreeAddressCodeBlock::addition(da, c, *c10);
    isaselector::simplify(block, *table);

    ASSERT_EQ(block.codes().size(), 6);
    int i = 0;
    std::vector<std::vector<MemoryPosition>> mmm = {
            {0, 5, 7},
            {1, 0},
            {0, 9, 3},
            {2, 0},
            {0, 1},
            {2, 0}
    };
    std::vector<ThreeAddressCode::Operator> ooo = {
            ThreeAddressCode::ADD,
            ThreeAddressCode::LOAD,
            ThreeAddressCode::ADD,
            ThreeAddressCode::LOAD,
            ThreeAddressCode::LOAD,
            ThreeAddressCode::LOAD_IND
    };
    for (ThreeAddressCode& tac : block.codes()) {
        ASSERT_EQ(tac.op, ooo[i]);
        testVRTacs(tac, mmm[i]);

        ++i;
    }
}

TEST_F(TACSimplify, IORead) {
    SymbolOperand a("a");
    ThreeAddressCodeBlock block = ThreeAddressCodeBlock::read(a);
    isaselector::simplify(block, *table);

    ASSERT_EQ(block.codes().size(), 2);
    int i = 0;
    std::vector<std::vector<MemoryPosition>> mmm = { {0}, {3, 0} };
    std::vector<ThreeAddressCode::Operator> ooo = { ThreeAddressCode::STDIN, ThreeAddressCode::LOAD };
    for (ThreeAddressCode& tac : block.codes()) {
        ASSERT_EQ(tac.op, ooo[i]);
        testVRTacs(tac, mmm[i]);

        ++i;
    }
}

TEST_F(TACSimplify, IOWrite) {
    SymbolOperand a("a");
    ThreeAddressCodeBlock block = ThreeAddressCodeBlock::write(a);
    isaselector::simplify(block, *table);

    ASSERT_EQ(block.codes().size(), 2);
    int i = 0;
    std::vector<std::vector<MemoryPosition>> mmm = { {0, 3}, {0} };
    std::vector<ThreeAddressCode::Operator> ooo = { ThreeAddressCode::LOAD, ThreeAddressCode::STDOUT };
    for (ThreeAddressCode& tac : block.codes()) {
        ASSERT_EQ(tac.op, ooo[i]);
        testVRTacs(tac, mmm[i]);

        ++i;
    }
}

TEST_F(TACSimplify, AddBeforeJumpCond) {
    SymbolOperand a("a");
    SymbolOperand c("c");
    VirtualRegisterOperand v1(1);
    ThreeAddressCodeBlock block = ThreeAddressCodeBlock::addition(v1, a, c);
    block.codes().pop_back();
    ThreeAddressCode jump = ThreeAddressCodeBlock::jump(LABEL_END, JUMP_ZERO);
    block.append(jump);
    isaselector::simplify(block, *table);

    ASSERT_EQ(block.codes().size(), 2);
    ThreeAddressCode tac1 = block.codes().front();
    ThreeAddressCode tac2 = block.codes().back();
    EXPECT_EQ(tac1.op, ThreeAddressCode::ADD);
    testVRTacs(tac1, {0, 3, 5});
    EXPECT_EQ(tac2.op, ThreeAddressCode::JZERO);
}

TEST_F(TACSimplify, LoadBeforeJumpCond) {
    SymbolOperand c("c");
    VirtualRegisterOperand v1(1);
    ThreeAddressCodeBlock block = ThreeAddressCodeBlock({
        ThreeAddressCode(v1.copy(), ThreeAddressCode::LOAD, c.copy()),
        ThreeAddressCodeBlock::jump(LABEL_END, JUMP_ZERO)
    });
    isaselector::simplify(block, *table);

    ASSERT_EQ(block.codes().size(), 2);
    ThreeAddressCode tac1 = block.codes().front();
    ThreeAddressCode tac2 = block.codes().back();
    EXPECT_EQ(tac1.op, ThreeAddressCode::LOAD);
    testVRTacs(tac1, {0, 5});
    EXPECT_EQ(tac2.op, ThreeAddressCode::JZERO);
}

TEST_F(TACSimplify, LoadBeforeJumpAlways) {
    SymbolOperand c("c");
    VirtualRegisterOperand v1(1);
    ThreeAddressCodeBlock block = ThreeAddressCodeBlock({
        ThreeAddressCode(v1.copy(), ThreeAddressCode::LOAD, c.copy()),
        ThreeAddressCodeBlock::jump(LABEL_END, JUMP_ALWAYS)
    });
    isaselector::simplify(block, *table);

    ASSERT_EQ(block.codes().size(), 1);
    ThreeAddressCode tac1 = block.codes().front();
    EXPECT_EQ(tac1.op, ThreeAddressCode::JUMP);
}

TEST_F(TACSimplify, CopyBeforeJumpCond) {
    SymbolOperand a("a");
    SymbolOperand c("c");
    ThreeAddressCodeBlock block = ThreeAddressCodeBlock::copy(a, c);
    ThreeAddressCode jcond = ThreeAddressCodeBlock::jump(LABEL_END, JUMP_ZERO);
    block.append(jcond);
    isaselector::simplify(block, *table);

    ASSERT_EQ(block.codes().size(), 3);
    auto it = block.codes().begin();
    std::vector<std::vector<MemoryPosition>> mmm = {{0, 5},
                                                    {3, 0}};
    for (int i = 0; i < 2; i++) {
        EXPECT_EQ(it->op, ThreeAddressCode::LOAD);
        testVRTacs(*it, mmm[i]);
        ++it;
    }
    EXPECT_EQ(block.codes().back().op, ThreeAddressCode::JZERO);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
