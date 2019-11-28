//
// Created by Kacper Raczy on 2019-10-02.
//

#ifndef backend_assembly_hpp
#define backend_assembly_hpp

#include <string>
#include <sstream>
#include <optional>

class Assembly {
private:
    std::string _mnemonic;
    std::optional<uint64_t> _argument;
    explicit Assembly(std::string mnemonic, std::optional<uint64_t> argument = std::nullopt): _mnemonic(mnemonic), _argument(argument) {};
public:
    Assembly() = delete;
    Assembly(const Assembly &cond) = default;
    Assembly& operator=(const Assembly &cond) = default;
    ~Assembly() = default;

    std::string mnemonic() const;
    std::optional<uint64_t> argument() const;
    void setArgument(std::optional<uint64_t> argument);
    std::string toString() const;
    static Assembly Put();
    static Assembly Get();
    static Assembly Load(uint64_t offset);
    static Assembly Store(uint64_t offset);
    static Assembly LoadI(uint64_t offset);
    static Assembly StoreI(uint64_t offset);
    static Assembly Add(uint64_t offset);
    static Assembly Sub(uint64_t offset);
    static Assembly Shift(uint64_t offset);
    static Assembly Inc();
    static Assembly Dec();
    static Assembly Jump(uint64_t pos);
    static Assembly JPos(uint64_t pos);
    static Assembly JZero(uint64_t pos);
    static Assembly JNeg(uint64_t pos);
    static Assembly Halt();
};

#endif /* backend_assembly_hpp */
