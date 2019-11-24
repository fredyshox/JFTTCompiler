//
// Created by Kacper Raczy on 2019-09-13.
//

extern "C" {
#define operator op
#include "frontend.h"
#undef operator
};

#include "gtest/gtest.h"
#include <iostream>
#include <vector>

static inline void testTokens(std::vector<int> expected, LEXER_BUFFER_STATE state) {
    std::vector<int> actual;
    int token;
    while ((token = lexer_lex())) {
        actual.push_back(token);
    }
    lexer_deleteScanBuffer(state);
    ASSERT_EQ(expected.size(), actual.size());
    for (unsigned long i = 0; i < expected.size(); i++) {
        ASSERT_EQ(expected.at(i), actual.at(i)) << "Tokens differ at index " << i;
    }
}

TEST(LexerTest, Keywords) {
    const char* str =
            "DECLARE IN END READ WRITE "
            "IF THEN ELSE ENDIF "
            "WHILE DO ENDWHILE ENDDO "
            "FOR FROM TO DOWNTO ENDFOR ";
    auto state = lexer_setScanString(str);
    std::vector<int> expected = {
            DECLARE, IN, END, READ, WRITE,
            IF, THEN, ELSE, ENDIF,
            WHILE, DO, ENDWHILE, ENDDO,
            FOR, FROM, TO, DOWNTO, ENDFOR
    };
    testTokens(expected, state);
}

TEST(LexerTest, WhitespaceSeparator) {
    const char* str = "DECLAREINENDREADWRITE IF THEN ELSE";
    auto state = lexer_setScanString(str);
    int token;
    token = lexer_lex();
    ASSERT_EQ(token, ERROR) << "Lexer recognized non-whitespace-separated token";
    lexer_deleteScanBuffer(state);
}

TEST(LexerTest, IdentifiersAndConstants) {
    const char* str = "abcds 443332 jjdkkkss 330";
    auto state = lexer_setScanString(str);
    std::vector<int> expected = {
            ID, NUMBER, ID, NUMBER
    };
    testTokens(expected, state);
}

TEST(LexerTest, MixedCases) {
    const char* str = "abcdssdDDEERR sdfsdf3333 DECLAREssdff END444";
    auto state = lexer_setScanString(str);

    int token;
    while ((token = lexer_lex())) {
        EXPECT_EQ(ERROR, token);
    }
    lexer_deleteScanBuffer(state);
}

TEST(LexerTest, IdentifiersNextToOperators) {
    const char* str = "READ a;\n b:=a*2; utility(1:2); IN\n";
    auto state = lexer_setScanString(str);
    std::vector<int> expected = {
            READ, ID, SEMICOLON,
            ID, ASSIGN, ID, MUL, NUMBER, SEMICOLON,
            ID, OPEN, NUMBER, COLON, NUMBER, CLOSE, SEMICOLON,
            IN,
    };
    testTokens(expected, state);
}

TEST(LexerTest, OperatorsSeparated) {
    const char* str = " * + - / % := ( ) != <= >= = < > ; : ";
    auto state = lexer_setScanString(str);
    std::vector<int> expected = {
            MUL, ADD, SUB, DIV, MOD, ASSIGN,
            OPEN, CLOSE, NEQ, LE, GE, EQ,
            LT, GT, SEMICOLON, COLON
    };
    testTokens(expected, state);
}

TEST(LexerTest, OperatorsNonSeparated) {
    const char* str = "*+-/%:=()!=<=>==<>;:";
    auto state = lexer_setScanString(str);
    std::vector<int> expected = {
            MUL, ADD, SUB, DIV, MOD, ASSIGN,
            OPEN, CLOSE, NEQ, LE, GE, EQ,
            LT, GT, SEMICOLON, COLON
    };
    testTokens(expected, state);
}

TEST(LexerTest, Comments) {
    const char* str = "[ ENDDO END IF ELSE ] DECLARE[ IF \n ENDIF ] END";
    auto state = lexer_setScanString(str);
    int token1 = lexer_lex();
    int token2 = lexer_lex();
    lexer_deleteScanBuffer(state);
    ASSERT_EQ(DECLARE, token1);
    ASSERT_EQ(END, token2);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}