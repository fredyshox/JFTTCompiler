//
// Created by Kacper Raczy on 2019-11-22.
//

#include "Label.hpp"

LabelIdentifier genLabel() {
    static LabelIdentifier currentId = LABEL_END + 1;
    return currentId++;
}