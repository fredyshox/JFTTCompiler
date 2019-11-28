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
    std::list<ThreeAddressCodeBlock> expand(BaseBlock& program, GlobalSymbolTable& symbolTable);

    /**
     * Simplifies three address code block.
     * @param block block to simplify
     */
    void simplify(ThreeAddressCodeBlock& block, GlobalSymbolTable& table);

    /**
     * Matches three address code to target isa instructions using abstract jump locations.
     * @param block block to match instructions to
     */
    void match(AssemblyBlock& asmBlock, JumpTable& jtable, ThreeAddressCodeBlock& block, GlobalSymbolTable& table) noexcept(false);

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
     */
    void loadValueToP0(AssemblyBlock& asmBlock, int64_t value);
}

#endif /* backend_instruciton_selector_hpp */
