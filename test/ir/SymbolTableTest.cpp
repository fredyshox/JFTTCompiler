//
// Created by Kacper Raczy on 2019-09-12.
//

#include "gtest/gtest.h"
#include "SymbolTable.hpp"
#include <string>
#include <exception>

using namespace std;

namespace {

class SymbolTableTest: public ::testing::Test {
protected:
    GlobalSymbolTable* gt;
    SymbolTable* ns1;
    SymbolTable* ns2;

    // helpers

    Record genRecord(string name) {
        return Record::integer(name);
    }

    void insertRecord(SymbolTable* table, string str) {
        table->insert(str, genRecord(str));
    }

    // setup/teardown

    void SetUp() override {
        gt = new GlobalSymbolTable();
        ns1 = new NestedSymbolTable(gt);
        ns2 = new NestedSymbolTable(ns1);
        insertRecord(gt, "g1");
        insertRecord(gt, "g2");
        insertRecord(gt, "g3");
        insertRecord(ns1, "n1");
        insertRecord(ns1, "n2");
        insertRecord(ns2, "n3");
    }

    void TearDown() override {
        delete gt;
        delete ns1;
        delete ns2;
    }
};

TEST_F(SymbolTableTest, NestedContains) {
    ASSERT_TRUE(gt->contains("g1"));
    ASSERT_TRUE(gt->contains("g2"));
    ASSERT_TRUE(gt->contains("g3"));
    ASSERT_FALSE(gt->contains("n1"));
    ASSERT_TRUE(ns1->contains("g1"));
    ASSERT_TRUE(ns1->contains("g2"));
    ASSERT_TRUE(ns1->contains("n1"));
    ASSERT_TRUE(ns1->contains("n2"));
    ASSERT_FALSE(ns1->contains("n3"));
    ASSERT_TRUE(ns2->contains("g1"));
    ASSERT_TRUE(ns2->contains("n1"));
    ASSERT_TRUE(ns2->contains("n3"));
}

TEST_F(SymbolTableTest, NestedSearch) {
    string key = "aaaa";
    insertRecord(ns1, key);
    auto r1 = ns1->search(key);
    auto r2 = ns2->search(key);
    EXPECT_EQ(r1.name, key);
    EXPECT_EQ(r2.name, key);
    try {
        gt->search(key);
        FAIL();
    } catch (const RecordNotFound &e) {
    } catch (const exception &e) {
        FAIL();
    }
}

TEST_F(SymbolTableTest, Insert) {
    insertRecord(gt, "hello");
    ASSERT_TRUE(gt->contains("hello"));
}

TEST_F(SymbolTableTest, GlobalAllRecords) {
    vector<string> keys = {"g1", "g2", "g3", "n1", "n2", "n3"};
    auto allRecords = gt->allRecords();
    for (auto& key : keys) {
        cout << "Evaluating: " << key << endl;
        ASSERT_TRUE(allRecords.find(key) != allRecords.end());
    }
}

}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}