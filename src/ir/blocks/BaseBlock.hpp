//
// Created by Kacper Raczy on 2019-09-08.
//

#ifndef ir_baseblock_hpp
#define ir_baseblock_hpp

#include <cstdint>
#include <list>
#include "Label.hpp"

class ThreeAddressCodeBlock;

/**
 * Base abstract class for blocks of code
 */
class BaseBlock {
private:
    LabelIdentifier _id;
    LabelIdentifier _endLabel;
    BaseBlock* _next;
    BaseBlock();

    friend class ThreeAddressCodeBlock;
    friend class ControlFlowBlock;
public:
    BaseBlock* next();
    void setNext(BaseBlock* next);
    virtual LabelIdentifier id();
    void setId(LabelIdentifier id);
    LabelIdentifier endLabel();
    void setEndLabel(LabelIdentifier endLabel);
    virtual void onIdChange(LabelIdentifier newId);
    virtual void onEndLabelChange(LabelIdentifier newLabel);
    virtual std::list<ThreeAddressCodeBlock> flatten() = 0;
    static std::list<ThreeAddressCodeBlock> flattenBlockList(BaseBlock* block);
    virtual ~BaseBlock() = default;
};

#endif /* ir_baseblock_hpp */
