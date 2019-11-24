//
// Created by Kacper Raczy on 2019-11-23.
//

#include "gtest/gtest.h"
#include "InstructionSelector.hpp"
#include <sstream>

using namespace std;

std::string printToString(AssemblyBlock& block) {
    std::stringstream ss;
    for (Assembly& a : block) {
        ss << a.toString() << "\n";
    }

    return ss.str();
}

TEST(ISAMatch, StdoutMatch) {

}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
