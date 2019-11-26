//
// ir
// Created by Kacper Raczy on 2019-09-06.
//

#ifndef ir_record_hpp
#define ir_record_hpp

#include <string>
#include <tuple>
#include <optional>

typedef std::tuple<int64_t, int64_t> ArrayBounds;
typedef uint64_t MemoryPosition;

#define ArrayBoundsGetLower(bounds) std::get<0>(bounds)
#define ArrayBoundsGetUpper(bounds) std::get<1>(bounds)

struct Record {
    const static MemoryPosition OFFSET_UNINITIALIZED = 0;

    enum Type {
        INT, ARRAY
    };

    std::string name;
    Type type;
    std::optional<ArrayBounds> bounds;
    int64_t size;
    MemoryPosition offset;
    int occurrences = 0;
    bool isIterator = false;
    bool isConstant = false;
    bool isMemoryLocation = false;

    Record(std::string name, Type type, int size, std::optional<ArrayBounds> bounds, MemoryPosition offset = OFFSET_UNINITIALIZED) {
        this->name = name;
        this->type = type;
        this->bounds = bounds;
        this->size = size;
        this->offset = offset;
    }

    static Record array(std::string name, ArrayBounds bounds) {
        int size = ArrayBoundsGetUpper(bounds) - ArrayBoundsGetLower(bounds) + 1;
        return Record(name, Type::ARRAY, size, std::optional{bounds});
    }

    static Record integer(std::string name) {
        return Record(name, Type::INT, 1, std::nullopt);
    }

    static Record iterator(std::string name) {
        Record r = Record::integer(name);
        r.isIterator = true;
        return r;
    }

    static Record constant(int64_t value) {
        Record r = Record::integer(constantName(value));
        r.isConstant = true;
        return r;
    }
    
    static Record memoryLocation(std::string name) {
        Record r = Record::integer(memoryLocationName(name));
        r.isMemoryLocation = true;
        return r;
    }

    static std::string constantName(int64_t value) {
        return "__c" + std::to_string(value);
    }

    static std::string memoryLocationName(std::string& recordName) {
        return "__m" + recordName;
    }

    MemoryPosition memoryPosition() const {
        if (!this->bounds.has_value()) {
            return offset;
        } else {
            int lbound = ArrayBoundsGetLower(this->bounds.value());
            return offset - lbound;
        }
    }
};

#endif /* ir_record_hpp */
