//
// Created by Kacper Raczy on 2019-10-02.
//

#ifndef backend_assembly_hpp
#define backend_assembly_hpp

#include <string>
#include <sstream>

#define ASSEMBLY_ARGUMENT_NONE (int64_t) -1

class Assembly {
private:
    std::string _mnemonic;
    int64_t _argument;
    explicit Assembly(std::string mnemonic, int64_t argument): _mnemonic(mnemonic), _argument(argument) {};
public:
    Assembly() = delete;
    Assembly(const Assembly &cond) = default;
    Assembly& operator=(const Assembly &cond) = default;
    ~Assembly() = default;

    std::string mnemonic();
    int64_t argument();
    void setArgument(int64_t argument);
    std::string toString();
    static Assembly Put();
    static Assembly Get();
    static Assembly Load(int64_t offset);
    static Assembly Store(int64_t offset);
    static Assembly LoadI(int64_t offset);
    static Assembly StoreI(int64_t offset);
    static Assembly Add(int64_t offset);
    static Assembly Sub(int64_t offset);
    static Assembly Shift(int64_t offset);
    static Assembly Inc();
    static Assembly Dec();
    static Assembly Jump(int64_t pos);
    static Assembly JPos(int64_t pos);
    static Assembly JZero(int64_t pos);
    static Assembly JNeg(int64_t pos);
    static Assembly Halt();
};

#endif /* backend_assembly_hpp */
