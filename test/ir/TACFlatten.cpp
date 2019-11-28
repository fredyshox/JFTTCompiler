//
// Created by Kacper Raczy on 2019-11-22.
//

#include "gtest/gtest.h"
#include "blocks/ConditionBlock.hpp"
#include "blocks/ThreeAddressCodeBlock.hpp"
#include "blocks/ForLoopBlock.hpp"
#include "blocks/WhileLoopBlock.hpp"
#include <vector>
#include <list>
#include <string>

using namespace std;

ConstantOperand c10(10);
SymbolOperand a("a");
SymbolOperand b("b");
Condition cond(a.copy(), c10.copy(), Condition::EQ);

#define SAMPLE_BODY(b1, b2) \
    ThreeAddressCodeBlock b1 = ThreeAddressCodeBlock::addition(b, a, c10); \
    ThreeAddressCodeBlock b2 = ThreeAddressCodeBlock::subtraction(a, b, c10); \
    b1.setNext(&b2);

TEST(TACFlatten, ConditionBlockPass) {
    SAMPLE_BODY(add, sub);
    ConditionBlock branch(cond, &add);

    list<ThreeAddressCodeBlock> flat = branch.flatten();
    ASSERT_EQ(flat.size(), 3);
    vector<uint64_t> sizes = {(3 + 2), 4, 4};
    auto it = flat.begin();
    for (int i = 0; i < 3; i++) {
        EXPECT_EQ((*it).size(), sizes[i]);
        ++it;
    }
}

TEST(TACFlatten, ConditionBlockPassFail) {
    SAMPLE_BODY(add1, sub1);
    SAMPLE_BODY(add2, sub2);
    ConditionBlock branch(cond, &add1, &add2);

    list<ThreeAddressCodeBlock> flat = branch.flatten();
    ASSERT_EQ(flat.size(), 6);
    vector<uint64_t> sizes = {(3 + 2), 4, 4, 1, 4, 4};
    auto it = flat.begin();
    for (int i = 0; i < 5; i++) {
        EXPECT_EQ((*it).size(), sizes[i]);
        ++it;
    }
}

TEST(TACFlatten, ForLoopBlock) {
    LoopRange lr = LoopRange(c10, a, 1);
    ForLoopBlock forLoop("i", lr, nullptr);
    // body
    SAMPLE_BODY(add, sub);
    forLoop.setBody(&add);

    list<ThreeAddressCodeBlock> flat = forLoop.flatten();
    ASSERT_EQ(flat.size(), 5);
    vector<uint64_t> sizes = {(4 + 4 + 2), 5, 4, 4, 9};
    auto it = flat.begin();
    for (int i = 0; i < 5; i++) {
        EXPECT_EQ((*it).size(), sizes[i]);
        ++it;
    }
}

TEST(TACFlatten, WhileLoopBlock) {
    WhileLoopBlock whileLoop(cond, nullptr);
    SAMPLE_BODY(add, sub);
    whileLoop.setBody(&add);

    list<ThreeAddressCodeBlock> flat = whileLoop.flatten();
    ASSERT_EQ(flat.size(), 4);
    vector<uint64_t> sizes = {5, 4, 4, 1};
    auto it = flat.begin();
    for (int i = 0; i < 4; i++) {
        EXPECT_EQ((*it).size(), sizes[i]);
        ++it;
    }
}

TEST(TACFlatten, DoWhileLoopBlock) {
    DoWhileLoopBlock whileLoop(cond, nullptr);
    SAMPLE_BODY(add, sub);
    whileLoop.setBody(&add);

    list<ThreeAddressCodeBlock> flat = whileLoop.flatten();
    ASSERT_EQ(flat.size(), 5);
    vector<uint64_t> sizes = {1, 5, 4, 4, 1};
    auto it = flat.begin();
    for (int i = 0; i < 5; i++) {
        EXPECT_EQ((*it).size(), sizes[i]);
        ++it;
    }
}

TEST(TACFlatten, NestedBlocks) {
    // for
    //   while
    //     add1
    //     sub1
    //   if
    //     add2
    //     sub2
    //   add3
    //   sub3

    LoopRange lr = LoopRange(c10, a, 1);
    ForLoopBlock forLoop("i", lr, nullptr);

    WhileLoopBlock whileLoop(cond, nullptr);
    SAMPLE_BODY(add1, sub1);
    whileLoop.setBody(&add1);

    SAMPLE_BODY(add2, sub2);
    ConditionBlock branch(cond, &add2);
    SAMPLE_BODY(add3, sub3);
    branch.setNext(&add3);
    whileLoop.setNext(&branch);

    forLoop.setBody(&whileLoop);

    list<ThreeAddressCodeBlock> flat = forLoop.flatten();
    int expectedSize =  /*forLoop*/ 3 + /* whileLoop */ 4 + /* if */ 3 + /* rest */ 2;
    ASSERT_EQ(flat.size(), expectedSize);
    vector<uint64_t> sizes = {
            /* forLoop */ (4 + 4 + 2), 5,
            /* whileLoop */ 5, 4, 4, 1,
            /* if */ (3 + 2), 4, 4,
            /* rest */ 4, 4,
            /* forLoopPost */ 9
    };
    auto it = flat.begin();
    for (int i = 0; i < expectedSize; i++) {
        EXPECT_EQ((*it).size(), sizes[i]);
        ++it;
    }
}

TEST(TACFlatten, StartEndLabels) {
    // for
    //   while
    //     add1
    //     sub1
    //   if
    //     add2
    //     sub2
    //   else
    //     add3
    //     sub3
    //   if
    //     add5
    //     sub5
    //   dowhile
    //     add6
    //     sub6
    //   add7
    //   sub7

    LoopRange lr = LoopRange(c10, a, 1);
    ForLoopBlock forLoop("i", lr, nullptr);

    WhileLoopBlock whileLoop(cond, nullptr);
    SAMPLE_BODY(add1, sub1);
    whileLoop.setBody(&add1);

    SAMPLE_BODY(add2, sub2);
    SAMPLE_BODY(add3, sub3);
    ConditionBlock branch1(cond, &add2, &add3);
    whileLoop.setNext(&branch1);

    SAMPLE_BODY(add5, sub5);
    ConditionBlock branch2(cond, &add5);
    branch1.setNext(&branch2);

    SAMPLE_BODY(add6, sub6);
    DoWhileLoopBlock dwhile(cond, nullptr);
    dwhile.setBody(&add6);
    branch2.setNext(&dwhile);

    SAMPLE_BODY(add7, sub7);
    dwhile.setNext(&add7);

    forLoop.setBody(&whileLoop);

    list<ThreeAddressCodeBlock> flat = forLoop.flatten();
    std::cout << "Flat tac code size: " << flat.size() << std::endl;

    if (flat.size() != 0) {
        std::cout << "Start label test." << std::endl;
        EXPECT_EQ(flat.begin()->id(), LABEL_START);
    }

    int counter = 0;
    for (auto it = flat.begin(); it != flat.end(); ++it) {
        std::cout << "Testing ids at: " << counter << " -> " << counter + 1 << std::endl;
        auto itNext = std::next(it);
        if (itNext == flat.end()) {
            EXPECT_EQ(it->endLabel() ,LABEL_END);
        } else {
            EXPECT_EQ(it->endLabel(), itNext->id());
        }
        ++counter;
    }
}

TEST(TACFlatten, StartEndLabels2) {
    // for
    //   while
    //     add1
    //     sub1
    //   if
    //     add2
    //     sub2
    //   else
    //     add3
    //     sub3
    //   if
    //     add5
    //     sub5
    //   dowhile
    //     add6
    //     sub6
    //   add7
    //   sub7

    LoopRange lr = LoopRange(c10, a, 1);
    ForLoopBlock forLoop("i", lr, nullptr);

    WhileLoopBlock whileLoop(cond, nullptr);
    SAMPLE_BODY(add1, sub1);
    whileLoop.setBody(&add1);

    SAMPLE_BODY(add2, sub2);
    SAMPLE_BODY(add3, sub3);
    ConditionBlock branch1(cond, nullptr, nullptr);
    branch1.setPassBody(&add2);
    branch1.setFailBody(&add3);

    whileLoop.setNext(&branch1);

    SAMPLE_BODY(add5, sub5);
    ConditionBlock branch2(cond, nullptr, nullptr);
    branch2.setPassBody(&add5);
    branch1.setNext(&branch2);

    SAMPLE_BODY(add6, sub6);
    DoWhileLoopBlock dwhile(cond, nullptr);
    dwhile.setBody(&add6);
    branch2.setNext(&dwhile);

    SAMPLE_BODY(add7, sub7);
    dwhile.setNext(&add7);

    forLoop.setBody(&whileLoop);

    list<ThreeAddressCodeBlock> flat = forLoop.flatten();
    std::cout << "Flat tac code size: " << flat.size() << std::endl;

    if (flat.size() != 0) {
        std::cout << "Start label test." << std::endl;
        EXPECT_EQ(flat.begin()->id(), LABEL_START);
    }

    int counter = 0;
    for (auto it = flat.begin(); it != flat.end(); ++it) {
        std::cout << "Testing ids at: " << counter << " -> " << counter + 1 << std::endl;
        auto itNext = std::next(it);
        if (itNext == flat.end()) {
            EXPECT_EQ(it->endLabel() ,LABEL_END);
        } else {
            EXPECT_EQ(it->endLabel(), itNext->id());
        }
        ++counter;
    }
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}