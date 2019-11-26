//
// Created by Kacper Raczy on 2019-09-25.
//

extern "C" {
#define operator op
#include "ast/ast.h"
#undef operator
};

#include "gtest/gtest.h"
#include "utility/SemanticAnalysis.hpp"
#include "utility/Converter.hpp"
#include "SymbolTable.hpp"
#include <vector>
#include <string>

using namespace std;

namespace {

class SemanticAnalysisTest: public ::testing::Test {
protected:
    ASTDeclarationList *sampleDeclarations;
    SymbolTable *sampleSymbolTable;

    ASTDeclarationList *createSampleDeclarations() {
        ASTDeclarationList *decList;
        decList = ASTDeclarationListCreate(ASTDeclarationCreateSimple("a"));
        decList->next = ASTDeclarationListCreate(ASTDeclarationCreateArray("b", 1, 10));
        decList->next->next = ASTDeclarationListCreate(ASTDeclarationCreateSimple("c"));

        return decList;
    }

    // setup/teardown

    void SetUp() override {
        sampleDeclarations = createSampleDeclarations();
        sampleSymbolTable = irconverter::symbolTableFrom(sampleDeclarations);
    }

    void TearDown() override {
        ASTDeclarationListFree(sampleDeclarations);
        delete sampleSymbolTable;
    }
};

TEST_F(SemanticAnalysisTest, DuplicateDeclarations) {
    ASTDeclarationList *decList;
    decList = ASTDeclarationListCreate(ASTDeclarationCreateSimple("a"));
    decList->next = ASTDeclarationListCreate(ASTDeclarationCreateSimple("b"));
    decList->next->next = ASTDeclarationListCreate(ASTDeclarationCreateSimple("a"));

    EXPECT_THROW(semanticanalysis::analyze(decList), semanticanalysis::DeclarationException);
    ASTDeclarationListFree(decList);

    decList = ASTDeclarationListCreate(ASTDeclarationCreateSimple("a"));
    decList->next = ASTDeclarationListCreate(ASTDeclarationCreateArray("a", 1, 10));
    decList->next->next = ASTDeclarationListCreate(ASTDeclarationCreateSimple("b"));

    EXPECT_THROW(semanticanalysis::analyze(decList), semanticanalysis::DeclarationException);
    ASTDeclarationListFree(decList);
}

TEST_F(SemanticAnalysisTest, ArrayIndices) {
    ASTDeclarationList *decList;
    decList = ASTDeclarationListCreate(ASTDeclarationCreateArray("a", 11, 10));
    EXPECT_THROW(semanticanalysis::analyze(decList), semanticanalysis::DeclarationException);
    ASTDeclarationListFree(decList);

    decList = ASTDeclarationListCreate(ASTDeclarationCreateArray("a", 10, 10));
    EXPECT_NO_THROW(semanticanalysis::analyze(decList));
    ASTDeclarationListFree(decList);

    decList = ASTDeclarationListCreate(ASTDeclarationCreateArray("a", 10, 11));
    EXPECT_NO_THROW(semanticanalysis::analyze(decList));
    ASTDeclarationListFree(decList);
}

TEST_F(SemanticAnalysisTest, SymbolValidation) {
    // non existing identifier
    auto *io = ASTIOCreateRead(ASTSymbolCreateSimple("d"));
    EXPECT_THROW(semanticanalysis::analyze(io, sampleSymbolTable), RecordNotFound);

    // indexing not array element
    auto a1 = ASTAssignmentCreateWithOperand(ASTSymbolCreateArray("a", ASTIndexCreateValue(1)),
                                             ASTOperandConstant(2));
    EXPECT_THROW(semanticanalysis::analyze(a1, sampleSymbolTable), semanticanalysis::SymbolException);

    // not indexing array element
    auto a2 = ASTAssignmentCreateWithOperand(ASTSymbolCreateSimple("a"),
                                             ASTOperandSymbol(ASTSymbolCreateSimple("b")));
    EXPECT_THROW(semanticanalysis::analyze(a1, sampleSymbolTable), semanticanalysis::SymbolException);

    // iterator mutation
    auto a3 = ASTAssignmentCreateWithOperand(ASTSymbolCreateSimple("i"), ASTOperandConstant(1));
    auto f1 = ASTLoopCreateForTo("i", ASTOperandConstant(1), ASTOperandConstant(3),
                                 ASTNodeCreate(kNodeAssignment, a3));
    auto f1node = ASTNodeCreate(kNodeForLoop, f1);
    EXPECT_THROW(semanticanalysis::analyze(f1node, sampleSymbolTable), semanticanalysis::SymbolException);

    // non initalized
    // TODO

    free(io);
    free(a1);
    free(a2);
    ASTNodeFree(f1->start);
    ASTNodeFree(f1node);
}

TEST_F(SemanticAnalysisTest, DuplicateIterator) {
    auto a1 = ASTAssignmentCreateWithOperand(ASTSymbolCreateSimple("c"), ASTOperandConstant(1));
    auto f1 = ASTLoopCreateForTo("a", ASTOperandConstant(1), ASTOperandConstant(3),
                                 ASTNodeCreate(kNodeAssignment, a1));
    EXPECT_THROW(semanticanalysis::analyze(f1, sampleSymbolTable), semanticanalysis::SymbolException);

    free(f1->start);
    free(f1);
}

}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}