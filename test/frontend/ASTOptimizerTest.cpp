//
// Created by Kacper Raczy on 2019-09-25.
//

extern "C" {
#define operator op
#include "ast/ast.h"
#undef operator
};

#include "gtest/gtest.h"
#include "utility/ASTOptimizer.hpp"

using namespace std;

ASTNode* trivialNode() {
    // branch with no body
    ASTCondition cond = ASTConditionCreate(ASTOperandConstant(1), ASTOperandConstant(2), kCondOperatorLess);
    ASTBranch* branch = ASTBranchCreate(cond, nullptr);
    ASTNode* node = ASTNodeCreate(kNodeBranch, branch);

    return node;
}

ASTNode* nonTrivialNode() {
    // standard constant assignment
    ASTSymbol symbol = ASTSymbolCreateSimple("a");
    ASTAssignment* assignment = ASTAssignmentCreateWithOperand(symbol, ASTOperandConstant(1));
    ASTNode* node = ASTNodeCreate(kNodeAssignment, assignment);

    return node;
}

TEST(ASTOptimizerTest, TrivialAssignments) {
    auto symbol = ASTSymbolCreateSimple("a");
    auto* a1 = ASTAssignmentCreateWithOperand(symbol, ASTOperandSymbol(symbol));
    EXPECT_EQ(astoptimizer::optimize(a1), ACTION_REMOVE);

    auto arraySymbol1 = ASTSymbolCreateArray("a", ASTIndexCreateValue(1));
    auto arraySymbol2 = ASTSymbolCreateArray("a", ASTIndexCreateValue(5));
    auto* a2 = ASTAssignmentCreateWithOperand(arraySymbol1, ASTOperandSymbol(arraySymbol2));
    EXPECT_EQ(astoptimizer::optimize(a2), ACTION_NOTHING);

    free(a1);
    free(a2);
}

TEST(ASTOptimizerTest, TrivialConditions) {
    // rest of operators?
    auto c1 = ASTConditionCreate(ASTOperandConstant(3), ASTOperandConstant(1), kCondOperatorGreater);
    auto* branch = ASTBranchCreateWithElse(c1, nonTrivialNode(), nonTrivialNode());
    EXPECT_EQ(astoptimizer::optimize(branch), ACTION_EXTRACT_BODY1);

    auto c2 = ASTConditionCreate(ASTOperandConstant(3), ASTOperandConstant(1), kCondOperatorLess);
    branch->condition = c2;
    EXPECT_EQ(astoptimizer::optimize(branch), ACTION_EXTRACT_BODY2);

    auto symbol = ASTSymbolCreateSimple("a");
    auto c3 = ASTConditionCreate(ASTOperandSymbol(symbol), ASTOperandSymbol(symbol), kCondOperatorGreaterEqual);
    branch->condition = c3;
    EXPECT_EQ(astoptimizer::optimize(branch), ACTION_EXTRACT_BODY1);

    auto c4 = ASTConditionCreate(ASTOperandSymbol(symbol), ASTOperandSymbol(symbol), kCondOperatorNotEqual);
    branch->condition = c4;
    EXPECT_EQ(astoptimizer::optimize(branch), ACTION_EXTRACT_BODY2);


    auto* body = ASTNodeCreate(kNodeBranch, branch);
    auto* whileLoop = ASTLoopCreateWhile(c2, body);
    EXPECT_EQ(astoptimizer::optimize(whileLoop), ACTION_REMOVE);
    whileLoop->condition = c1;
    EXPECT_EQ(astoptimizer::optimize(whileLoop), ACTION_NOTHING);

    auto* doWhileLoop = ASTLoopCreateDoWhile(c2, body);
    EXPECT_EQ(astoptimizer::optimize(doWhileLoop), ACTION_EXTRACT_BODY1);
    doWhileLoop->condition = c1;
    EXPECT_EQ(astoptimizer::optimize(doWhileLoop), ACTION_NOTHING);

    ASTNodeFree(branch->ifNode);
    ASTNodeFree(branch->elseNode);
    free(branch);
    free(whileLoop);
    free(doWhileLoop);
    free(body);
}

TEST(ASTOptimizerTest, ForLoopRanges) {
    auto* body = ASTNodeCreate(kNodeIO, ASTIOCreateRead(ASTSymbolCreateSimple("q")));
    auto* forLoop = ASTLoopCreateForTo("i", ASTOperandConstant(1), ASTOperandConstant(0), body);
    EXPECT_EQ(astoptimizer::optimize(forLoop), ACTION_REMOVE);

    forLoop->type = kLoopForDownTo;
    EXPECT_EQ(astoptimizer::optimize(forLoop), ACTION_NOTHING);

    forLoop->iteratorBound.constant = 1;
    EXPECT_EQ(astoptimizer::optimize(forLoop), ACTION_NOTHING);

    forLoop->type = kLoopForTo;
    EXPECT_EQ(astoptimizer::optimize(forLoop), ACTION_NOTHING);

    forLoop->start = nullptr;
    EXPECT_EQ(astoptimizer::optimize(forLoop), ACTION_REMOVE);

    free(forLoop);
    free(body);
}

TEST(ASTOptimizerTest, ExpressionCompression) {
    auto symbol = ASTSymbolCreateSimple("a");
    auto expr = ASTExpressionCreate(ASTOperandConstant(2), ASTOperandConstant(5), kOperatorAdd);
    auto* a1 = ASTAssignmentCreateWithExpression(symbol, expr);
    EXPECT_EQ(astoptimizer::optimize(a1), ACTION_EDIT);
    EXPECT_EQ(a1->rtype, kRTypeOperand);
    ASSERT_EQ(a1->operand.type, kOperandConstant);
    ASSERT_EQ(a1->operand.constant, (ASTConstant) 7);

    expr.op = kOperatorDiv;
    expr.operand2 = ASTOperandConstant(0);
    auto* a2 = ASTAssignmentCreateWithExpression(symbol, expr);
    EXPECT_EQ(astoptimizer::optimize(a2), ACTION_EDIT);
    EXPECT_EQ(a2->rtype, kRTypeOperand);
    ASSERT_EQ(a2->operand.type, kOperandConstant);
    ASSERT_EQ(a2->operand.constant, (ASTConstant) 0);

    expr.op = kOperatorMod;
    auto* a3 = ASTAssignmentCreateWithExpression(symbol, expr);
    EXPECT_EQ(astoptimizer::optimize(a3), ACTION_EDIT);
    EXPECT_EQ(a3->rtype, kRTypeOperand);
    ASSERT_EQ(a3->operand.type, kOperandConstant);
    ASSERT_EQ(a3->operand.constant, (ASTConstant) 0);

    free(a1);
    free(a2);
    free(a3);
}

TEST(ASTOptimizerTest, NodeListManipulation) {
    ASTNode* trivial;
    ASTNode* nonTrivial;
    ASTNode* nonTrivial2;

    // null  -> Node0 -> null
    trivial = trivialNode();
    EXPECT_EQ(astoptimizer::optimize(trivial), OPTIMIZER_BODY_REMOVED);

    // null  -> Node0 -> Node1
    trivial = trivialNode();
    nonTrivial = nonTrivialNode();
    trivial->next = nonTrivial;
    nonTrivial->prev = trivial;
    EXPECT_EQ(astoptimizer::optimize(trivial), OPTIMIZER_NEXT_NEW_START);
    EXPECT_TRUE(nonTrivial->prev == nullptr);
    free(nonTrivial);

    // Node1 -> Node0 -> mull
    trivial = trivialNode();
    nonTrivial = nonTrivialNode();
    nonTrivial->next = trivial;
    trivial->prev = nonTrivial;
    EXPECT_EQ(astoptimizer::optimize(trivial), 0);
    EXPECT_TRUE(nonTrivial->next == nullptr);
    free(nonTrivial);

    // Node1 -> Node0 -> Node1
    trivial = trivialNode();
    nonTrivial = nonTrivialNode();
    nonTrivial2 = nonTrivialNode();
    nonTrivial->next = trivial;
    trivial->prev = nonTrivial;
    trivial->next = nonTrivial2;
    nonTrivial2->prev = trivial;
    EXPECT_EQ(astoptimizer::optimize(trivial), 0);
    EXPECT_TRUE(nonTrivial->next == nonTrivial2);
    EXPECT_TRUE(nonTrivial2->prev == nonTrivial);
    free(nonTrivial);
    free(nonTrivial2);
}

TEST(ASTOptimizerTest, NestedOptimimalizations) {
    auto* body1 = nonTrivialNode();
    auto* body2 = nonTrivialNode();
    auto* forLoopCommand = ASTLoopCreateForTo("i", ASTOperandConstant(1), ASTOperandConstant(0), body1);
    auto* forLoopNode = ASTNodeCreate(kNodeForLoop, forLoopCommand);
    forLoopNode->next = body2;
    body2->prev = forLoopNode;

    ASTCondition cond = ASTConditionCreate(ASTOperandSymbol(ASTSymbolCreateSimple("a")), ASTOperandConstant(2), kCondOperatorLess);
    auto* branch = ASTBranchCreate(cond, forLoopNode);
    auto* branchNode = ASTNodeCreate(kNodeBranch, branch);

    ASSERT_EQ(astoptimizer::optimize(branchNode), 0);
    EXPECT_TRUE(branch->ifNode == body2);
    EXPECT_TRUE(branch->ifNode->next == nullptr);
    EXPECT_TRUE(branch->ifNode->prev == nullptr);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}