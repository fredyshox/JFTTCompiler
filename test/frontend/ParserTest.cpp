//
// Created by Kacper Raczy on 2019-09-12.
//

extern "C" {
#define operator op
#include "frontend.h"
#undef operator
};

#include "gtest/gtest.h"
#include <cstring>
#include <iostream>
#include <vector>

using namespace std;

TEST(ParserTest, DeclarationsSimple) {
    const char* str =
            "DECLARE a; bc; int; "
            "IN READ a; "
            "END";

    vector<tuple<string, int>> actual;
    vector<tuple<string, int>> expected = {
            {"a", kASTIdentifierSimple},
            {"bc", kASTIdentifierSimple},
            {"int", kASTIdentifierSimple}
    };
    ASTProgram* program = parser_parseString(str);
    ASTDeclarationList* current = program->declarations;
    char* identifier;
    int type;
    while (current != NULL) {
        identifier = current->value.identifier;
        type = current->value.type;
        actual.push_back({identifier, type});
        current = current->next;
    }

    ASSERT_EQ(expected.size(), actual.size());
    for (unsigned long i = 0; i < expected.size(); i++) {
        EXPECT_EQ(expected.at(i), actual.at(i));
    }
    ASTProgramFree(program);
}

TEST(ParserTest, DeclarationsArray) {
    const char* str =
            "DECLARE a(1:10); bc(7:9); int(100:1000); "
            "IN READ a; "
            "END";
    vector<tuple<string, int, int, int>> actual;
    vector<tuple<string, int, int, int>> expected = {
            {"a", kASTIdentifierArray, 1, 10},
            {"bc", kASTIdentifierArray, 7, 9},
            {"int", kASTIdentifierArray, 100, 1000}
    };
    ASTProgram* program = parser_parseString(str);
    ASTDeclarationList* current = program->declarations;
    char* identifier;
    int type, lower, upper;
    while (current != NULL) {
        identifier = current->value.identifier;
        type = current->value.type;
        lower = current->value.lower; upper = current->value.upper;
        actual.push_back({identifier, type, lower, upper});
        current = current->next;
    }

    ASSERT_EQ(expected.size(), actual.size());
    for (unsigned long i = 0; i < expected.size(); i++) {
        EXPECT_EQ(expected.at(i), actual.at(i));
    }
    ASTProgramFree(program);
}

TEST(ParserTest, Assignments) {
    const char* str =
            "DECLARE abc; int; array(1:100); IN "
            "abc := int; "
            "array(2) := abc; "
            "abc := array(2); "
            "abc := int + array(2); "
            "abc := int - array(3); "
            "abc := int * array(4); "
            "abc := int % array(5); "
            "END ";

    vector<string> expected = {
            "abc := int",
            "array(2) := abc",
            "abc := array(2)",
            "abc := int + array(2)",
            "abc := int - array(3)",
            "abc := int * array(4)",
            "abc := int % array(5)",
    };
    ASTProgram* program = parser_parseString(str);
    ASSERT_FALSE(program == NULL);
    ASTNode* current = program->start;
    ASTAssignment* assignment;
    char buffer[64];
    int counter = 0;

    while (current != NULL) {
        ASSERT_EQ(kNodeAssignment, current->type);
        assignment = (ASTAssignment*) current->command;
        ASTAssignmentToString(assignment, buffer);
        EXPECT_EQ(expected.at(counter), buffer);
        current = current->next;
        counter += 1;
    }
    ASSERT_EQ(expected.size(), counter);
    ASTProgramFree(program);
}

TEST(ParserTest, IO) {
    const char* str =
            "DECLARE abc; int; array(1:100); IN "
            "READ int; "
            "READ array(2); "
            "WRITE int; "
            "WRITE array(2); "
            "END";

    vector<string> expected = {
            "READ int",
            "READ array(2)",
            "WRITE int",
            "WRITE array(2)"
    };

    ASTProgram* program = parser_parseString(str);
    ASSERT_FALSE(program == NULL);
    ASTNode* current = program->start;
    ASTIO* astio;
    char buffer[32];
    int counter = 0;

    while (current != NULL) {
        ASSERT_EQ(kNodeIO, current->type);
        astio = (ASTIO*) current->command;
        ASTIOToString(astio, buffer);
        EXPECT_EQ(expected.at(counter), buffer);
        current = current->next;
        counter += 1;
    }
    ASSERT_EQ(expected.size(), counter);
    ASTProgramFree(program);
}

TEST(ParserTest, ForLoop) {
    const char* str =
            "DECLARE int; IN "
            "FOR i FROM 1 TO int DO "
            "WRITE i; "
            "ENDFOR "
            "END ";

    ASTProgram* program = parser_parseString(str);
    ASSERT_FALSE(program == NULL);
    ASTNode* start = program->start;
    ASSERT_EQ(kNodeForLoop, start->type);
    ASTForLoop* forLoop = (ASTForLoop*) start->command;
    char buffer[32];
    ASTForLoopToString(forLoop, buffer);
    ASSERT_STREQ("FOR i IN 1:int:1", buffer);

    int bodyCounter = 0;
    ASTNode* body = forLoop->start;
    while (body != NULL) {
        bodyCounter += 1;
        body = body->next;
    }
    ASSERT_EQ(1, bodyCounter);
    ASTProgramFree(program);
}

static void testWhileLoop(int type, int totalBody, const char* str, const char* conditionStr) {
    ASTProgram* program = parser_parseString(str);
    ASSERT_FALSE(program == NULL);
    ASTNode* start = program->start;
    ASSERT_EQ(kNodeWhileLoop, start->type);
    ASTWhileLoop* whileLoop = (ASTWhileLoop*) start->command;
    EXPECT_EQ(type, whileLoop->type);
    char buffer[32];
    ASTConditionToString(whileLoop->condition, buffer);
    ASSERT_STREQ(conditionStr, buffer);

    int bodyCounter = 0;
    ASTNode* body = whileLoop->start;
    while (body != NULL) {
        bodyCounter += 1;
        body = body->next;
    }
    ASSERT_EQ(totalBody, bodyCounter);
    ASTProgramFree(program);
}

TEST(ParserTest, WhileLoop) {
    const char* str =
            "DECLARE int; IN "
            "WHILE int < 5 DO "
            "WRITE int; "
            "ENDWHILE "
            "END ";
    const char* conditionStr = "int < 5";
    testWhileLoop(kLoopWhile, 1, str, conditionStr);
}

TEST(ParserTest, DoWhileLoop) {
    const char* str =
            "DECLARE int; IN "
            "DO "
            "WRITE int; "
            "WHILE 100 >= int "
            "ENDDO "
            "END ";
    const char* conditionStr = "100 >= int";

    testWhileLoop(kLoopDoWhile, 1, str, conditionStr);
}

TEST(ParserTest, IfElse) {
    const char* str =
            "DECLARE int; IN "
            "IF int = 111 THEN "
            "READ int; "
            "WRITE int; "
            "ELSE "
            "WRITE int; "
            "ENDIF "
            "END ";

    ASTProgram* program = parser_parseString(str);
    ASSERT_FALSE(program == NULL);
    ASTNode* start = program->start;
    ASSERT_EQ(kNodeBranch, start->type);
    ASTBranch* branch = (ASTBranch*) start->command;
    char buffer[32];
    ASTConditionToString(branch->condition, buffer);
    ASSERT_STREQ("int = 111", buffer);

    int ifBodyCount = 0, elseBodyCount = 0;
    ASTNode* ifBody = branch->ifNode; ASTNode* elseBody = branch->elseNode;
    while (ifBody != NULL) {
        ifBodyCount += 1;
        ifBody = ifBody->next;
    }

    while (elseBody != NULL) {
        elseBodyCount += 1;
        elseBody = elseBody->next;
    }

    ASSERT_EQ(2, ifBodyCount);
    ASSERT_EQ(1, elseBodyCount);
    ASTProgramFree(program);
}

TEST(ParserTest, NestedInstructions) {
    const char* str =
            "DECLARE a; b; c; IN "
            "FOR i FROM a TO b DO "
            "a := i * 5; "
            "WHILE a < 5 DO "
            "IF b >= a THEN "
            "READ a; "
            "a := b + 4; "
            "ENDIF "
            "ENDWHILE "
            "ENDFOR "
            "END ";
    int bodyCounts[3] = {1,1,1};
    ASTProgram* program = parser_parseString(str);
    ASSERT_FALSE(program == NULL);
    ASTNode* current = program->start;
    ASSERT_EQ(kNodeForLoop, current->type);
    current = ((ASTForLoop*) current->command)->start;
    while (current->next != NULL) {
        bodyCounts[0] += 1;
        current = current->next;
    }
    ASSERT_EQ(kNodeWhileLoop, current->type);
    current = ((ASTWhileLoop*) current->command)->start;
    while (current->next != NULL) {
        bodyCounts[1] += 1;
        current = current->next;
    }
    ASSERT_EQ(kNodeBranch, current->type);
    ASTBranch* branch = ((ASTBranch* ) current->command);
    ASSERT_TRUE(branch->elseNode == NULL);
    ASSERT_FALSE(branch->ifNode == NULL);
    current = branch->ifNode;
    while (current->next != NULL) {
        bodyCounts[2] += 1;
        current = current->next;
    }

    EXPECT_EQ(2, bodyCounts[0]);
    EXPECT_EQ(1, bodyCounts[1]);
    EXPECT_EQ(2, bodyCounts[2]);
    ASTProgramFree(program);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}