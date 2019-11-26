//
// Created by Kacper Raczy on 2019-09-25.
//

extern "C" {
#define operator op
#include "ast/ast.h"
#undef operator
};

#include "gtest/gtest.h"
#include "utility/Converter.hpp"
#include "SymbolTable.hpp"
#include "Record.hpp"
#include <string>

using namespace std;

namespace {

class ConverterTest : public ::testing::Test {
protected:
    const char* recordName = "abc";
    ASTNode* sampleBody;
    SymbolTable* sampleSymbolTable;

    ASTNode *createSampleBody() {
        ASTSymbol s = ASTSymbolCreateSimple((char*) recordName);
        ASTAssignment *a1 = ASTAssignmentCreateWithOperand(s, ASTOperandConstant(12));
        ASTExpression expr = ASTExpressionCreate(ASTOperandSymbol(s), ASTOperandConstant(10), kOperatorAdd);
        ASTAssignment *a2 = ASTAssignmentCreateWithExpression(s, expr);
        ASTNode *n1 = ASTNodeCreate(kNodeAssignment, a1);
        ASTNode *n2 = ASTNodeCreate(kNodeAssignment, a2);
        n1->next = n2;

        return n1;
    }

    ASTOperand abcOperand() {
        auto s = ASTSymbolCreateSimple((char *) recordName);
        return ASTOperandSymbol(s);
    }

    void testSampleBody(BaseBlock* block) {
        std::vector<unsigned long> sizes { 2, 4 };
        BaseBlock* current = block;
        for (int i = 0; i < 2; i++) {
            ASSERT_TRUE(current != nullptr);
            ThreeAddressCodeBlock* tac;
            ASSERT_FALSE((tac = dynamic_cast<ThreeAddressCodeBlock*>(current)) == nullptr);
            EXPECT_EQ(tac->size(), sizes[i]);
            current = current->next();
        }
        ASSERT_TRUE(current == nullptr);
    }

    // setup/teardown

    void SetUp() override {
        sampleBody = createSampleBody();
        sampleSymbolTable = new GlobalSymbolTable();
        sampleSymbolTable->insert(recordName, Record::integer(recordName));
    }

    void TearDown() override {
        ASTNodeFreeList(sampleBody);
        delete sampleSymbolTable;
    }
};

TEST_F(ConverterTest, SymbolTableConvertion) {
    ASTDeclarationList *decList;
    decList = ASTDeclarationListCreate(ASTDeclarationCreateSimple("a"));
    decList->next = ASTDeclarationListCreate(ASTDeclarationCreateArray("b", 1, 10));
    decList->next->next = ASTDeclarationListCreate(ASTDeclarationCreateSimple("c"));
    SymbolTable *st = irconverter::symbolTableFrom(decList);
    EXPECT_NO_THROW({
                        auto a = st->search("a");
                        EXPECT_EQ(a.type, Record::Type::INT);
                        EXPECT_EQ(a.name, "a");
                    });

    EXPECT_NO_THROW({
                        auto b = st->search("b");
                        EXPECT_EQ(b.type, Record::Type::ARRAY);
                        EXPECT_EQ(b.name, "b");
                        ASSERT_TRUE(b.bounds.has_value());
                        EXPECT_EQ(ArrayBoundsGetLower(*b.bounds), 1);
                        EXPECT_EQ(ArrayBoundsGetUpper(*b.bounds), 10);
                    });

    EXPECT_NO_THROW({
                        auto c = st->search("c");
                        EXPECT_EQ(c.type, Record::Type::INT);
                        EXPECT_EQ(c.name, "c");
                    });

    free(decList);
    delete st;
}

TEST_F(ConverterTest, ForLoop) {
    ASTForLoop *forLoop = ASTLoopCreateForTo("i", ASTOperandConstant(1), abcOperand(), sampleBody);
    auto* forBlock = irconverter::convert(*forLoop, sampleSymbolTable->copy());
    // iterator name
    EXPECT_EQ(forBlock->iteratorName(), "i");
    // loop ranges
    ConstantOperand* cOperand;
    SymbolOperand* sOperand;
    ASSERT_FALSE((cOperand = dynamic_cast<ConstantOperand*>(forBlock->loopRange().initial.get())) == nullptr);
    EXPECT_EQ(cOperand->value, 1);
    ASSERT_FALSE((sOperand = dynamic_cast<SymbolOperand*>(forBlock->loopRange().bound.get())) == nullptr);
    EXPECT_EQ(sOperand->symbol, recordName);
    EXPECT_EQ(forBlock->loopRange().step, 1);
    // loop body

    free(forLoop);
    forLoop = ASTLoopCreateForDownTo("i", ASTOperandConstant(1), abcOperand(), sampleBody);
    forBlock = irconverter::convert(*forLoop, sampleSymbolTable->copy());
    EXPECT_EQ(forBlock->loopRange().step, -1);

    free(forLoop);
    delete forBlock;
}

TEST_F(ConverterTest, WhileLoop) {
    auto cond = ASTConditionCreate(ASTOperandConstant(4), abcOperand(), kCondOperatorLess);
    ASTWhileLoop* wloop = ASTLoopCreateWhile(cond, sampleBody);
    auto* whileBlock = irconverter::convert(*wloop, sampleSymbolTable->copy());
    // condition
    ConstantOperand* cOperand;
    SymbolOperand* sOperand;
    ASSERT_FALSE((cOperand = dynamic_cast<ConstantOperand*>(whileBlock->condition().firstOperand.get())) == nullptr);
    EXPECT_EQ(cOperand->value, 4);
    ASSERT_FALSE((sOperand = dynamic_cast<SymbolOperand*>(whileBlock->condition().secondOperand.get())) == nullptr);
    EXPECT_EQ(sOperand->symbol, recordName);
    EXPECT_EQ(whileBlock->condition().op, Condition::Operator::LT);
    ASSERT_TRUE(dynamic_cast<DoWhileLoopBlock*>(whileBlock) == nullptr);
    // loop body
    free(wloop);
    delete whileBlock;

    // dowhile
    wloop = ASTLoopCreateDoWhile(cond, sampleBody);
    auto* doWhileBlock = irconverter::convert(*wloop, sampleSymbolTable->copy());
    ASSERT_FALSE(dynamic_cast<DoWhileLoopBlock*>(doWhileBlock) == nullptr);

    free(wloop);
    delete doWhileBlock;
}

TEST_F(ConverterTest, Branch) {
    auto cond = ASTConditionCreate(abcOperand(), ASTOperandConstant(11), kCondOperatorEqual);
    ASTBranch* branch = ASTBranchCreateWithElse(cond, sampleBody, sampleBody);
    auto* condBlock = irconverter::convert(*branch, sampleSymbolTable);
    // condition
    ConstantOperand* cOperand;
    SymbolOperand* sOperand;
    ASSERT_FALSE((sOperand = dynamic_cast<SymbolOperand*>(condBlock->condition().firstOperand.get())) == nullptr);
    EXPECT_EQ(sOperand->symbol, recordName);
    ASSERT_FALSE((cOperand = dynamic_cast<ConstantOperand*>(condBlock->condition().secondOperand.get())) == nullptr);
    EXPECT_EQ(cOperand->value, 11);
    EXPECT_EQ(condBlock->condition().op, Condition::Operator::EQ);
    // passBody
    // failBody
}

TEST_F(ConverterTest, SampleBody) {
    BaseBlock* block = irconverter::convertList(sampleBody, sampleSymbolTable);
    testSampleBody(block);
    delete block;
}

};

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}