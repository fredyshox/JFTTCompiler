//
// Created by Kacper Raczy on 2019-09-08.
//

#ifndef ir_baseblock_hpp
#define ir_baseblock_hpp

#include <cstdint>

/**
 * Base abstract class for blocks of code
 */
class BaseBlock {
private:
    uint64_t _id;
    BaseBlock* _next;
    BaseBlock();

    friend class ThreeAddressCodeBlock;
    friend class ControlFlowBlock;
public:
    BaseBlock* next();
    void setNext(BaseBlock* next);
    uint64_t id();
    void setId(uint64_t id);
    virtual ~BaseBlock() = default;
};

#endif /* ir_baseblock_hpp */
