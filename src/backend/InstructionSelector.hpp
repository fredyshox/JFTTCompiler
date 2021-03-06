//
// Created by Kacper Raczy on 2019-10-02.
//

#ifndef backend_instruciton_selector_hpp
#define backend_instruciton_selector_hpp

#include "blocks/ControlFlowBlock.hpp"
#include "blocks/ConditionBlock.hpp"
#include "blocks/WhileLoopBlock.hpp"
#include "blocks/ForLoopBlock.hpp"
#include "SymbolTable.hpp"
#include "Assembly.hpp"
#include <list>
#include <vector>
#include <exception>

using AssemblyBlock = std::vector<Assembly>;
// label identifier -> code location
using JumpTable = std::unordered_map<LabelIdentifier, uint64_t>;
// memory postion -> constant
using ConstantTable = std::unordered_map<MemoryPosition, int64_t>;

namespace isaselector {
    struct ISAMatchFailed: public std::exception {
        const std::string message;
        explicit ISAMatchFailed(std::string message);
        const char* what() const throw() override;
    };

    /**
     * Expands control flow blocks into basic three address code blocks.
     * @param program program to expand
     */
    std::list<ThreeAddressCodeBlock> expand(BaseBlock& program, GlobalSymbolTable& symbolTable, ConstantTable& values);

    /**
     * Simplifies three address code block.
     * @param block block to simplify
     */
    void simplify(ThreeAddressCodeBlock& block, GlobalSymbolTable& table);

    /**
     * Matches three address code to target isa instructions using abstract jump locations.
     * @param block block to match instructions to
     */
    void match(AssemblyBlock& asmBlock, JumpTable& jtable, ThreeAddressCodeBlock& block, GlobalSymbolTable& table, ConstantTable& constants) noexcept(false);

    /**
     * Adds memory map initialization block
     * @param allRecords
     */
    void initializationBlock(AssemblyBlock& asmBlock, GlobalSymbolTable& symbolTable);

    /**
     * Adds program ending block
     * @param asmBlock
     * @param jtable
     */
    void terminationBlock(AssemblyBlock& asmBlock, JumpTable& jtable);

    /**
     * Converts abstract jump locations into real one
     * @param block block to be fixed
     * @param jtable jump table
     */
    void applyJumpTable(AssemblyBlock& asmBlock, JumpTable& jtable) noexcept(false);

    /**
     * Fills block with instructions of loading constant value into memory location 0
     * @param block block to be filled
     * @param value constant signed value
     * @param location of 1 constant, required for efficient number generation
     */
    void loadValueToP0(AssemblyBlock& asmBlock, int64_t value, MemoryPosition c1Location);

    /**
     * Calculate cost of loading value using standard loading method
     * @param value
     * @return cost
     */
    uint64_t loadingCost1(int64_t value);

    /**
     * Calculate cost of loading value using method optimized for small numbers
     * @param value
     * @return cost
     */
    uint64_t loadingCost2(int64_t value);

    /**
     * Calculate cost of loading value using method optimized for numbers > 28 and < 1155
     * @param value
     * @return cost
     */
    uint64_t loadingCost3(int64_t value);
}

#endif /* backend_instruciton_selector_hpp */
