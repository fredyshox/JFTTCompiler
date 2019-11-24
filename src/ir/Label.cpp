//
// Created by Kacper Raczy on 2019-11-22.
//

#include "Label.hpp"

LabelIdentifier genLabel() {
    static LabelIdentifier currentId = 0;
    return currentId++;
}