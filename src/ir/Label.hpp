//
// Created by Kacper Raczy on 2019-11-21.
//

#ifndef ir_label_hpp
#define ir_label_hpp

#include <cstdint>

#define LABEL_START 0
#define LABEL_END 1

typedef uint64_t LabelIdentifier;

LabelIdentifier genLabel();

#endif /* ir_label_hpp */
