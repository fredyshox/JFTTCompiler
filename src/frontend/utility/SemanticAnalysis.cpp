//
// Created by Kacper Raczy on 2019-09-13.
//

#include "SemanticAnalysis.hpp"

semanticanalysis::SemanticAnalysisException::SemanticAnalysisException(std::string cause): cause(cause) {}

const char* semanticanalysis::SymbolException::what() const throw() {
    std::stringstream ss;
    ss << "SymbolException: " << cause << ".";
    return ss.str().c_str();
}

const char* semanticanalysis::DeclarationException::what() const throw() {
    std::stringstream ss;
    ss << "DeclarationException: " << cause << ".";
    return ss.str().c_str();
}

void semanticanalysis::analyze(ASTDeclarationList *decList) {
    std::unordered_set<std::string> set;
    ASTDeclarationList* current = decList;
    std::string name;
    while (current != nullptr) {
        name = current->value.identifier;
        if (set.find(name) != set.end()) { // found duplicate
            throw DeclarationException("Duplicate declaration of " + name);
        }

        if (current->value.type == kASTIdentifierArray && current->value.lower > current->value.upper) {
            throw DeclarationException("Invalid array indices in declaration of " + name);
        }

        set.insert(name);
        current = current->next;
    }
}

void semanticanalysis::validate(ASTSymbol symbol, Record record) {
    std::string symbolId = symbol.identifier;
    if (record.name.compare(symbolId) != 0) {
        throw SymbolException("Identifier do not match with symbol table - " + std::string(symbolId));
    }

    if (symbol.type == kASTIdentifierSimple && record.type != Record::Type::INT) {
        throw SymbolException(symbolId + " is not INT, and should be indexed");
    }

    if (symbol.type == kASTIdentifierArray && (record.type != Record::Type::ARRAY || !record.bounds.has_value())) {
        throw SymbolException(symbolId + " is not ARRAY, and should not be indexed");
    }

    if (symbol.type == kASTIdentifierArray && symbol.index.indexType == kASTIndexValue) {
        int constantIndex = symbol.index.value;
        if (constantIndex < ArrayBoundsGetLower(record.bounds.value()) || constantIndex > ArrayBoundsGetUpper(record.bounds.value())) {
            std::stringstream ss;
            ss << symbolId << "(" << constantIndex << ")" << " index out of bounds";
            throw SymbolException(ss.str());
        }
    }
}

void semanticanalysis::analyze(ASTAssignment *assignment, SymbolTable *symbolTable) {
    // lvalue validation
    Record lval = symbolTable->search(assignment->symbol.identifier);
    validate(assignment->symbol, lval);
    if (lval.isIterator) {
        throw SymbolException("Loop iterator " + lval.name + " is immutable");
    }
    // rvalue validation
    if (assignment->rtype == kRTypeOperand && assignment->operand.type == kOperandSymbol) {
        ASTSymbol symbol = assignment->symbol;
        Record rval = symbolTable->search(symbol.identifier);
        validate(symbol, rval);
    } else if (assignment->rtype == kRTypeExpression) {
        ASTExpression expr = assignment->expression;
        if (expr.operand1.type == kOperandSymbol) {
            ASTSymbol symbol1 = expr.operand1.symbol;
            Record rval1 = symbolTable->search(symbol1.identifier);
            validate(symbol1, rval1);
        }

        if (expr.operand2.type == kOperandSymbol) {
            ASTSymbol symbol2 = expr.operand2.symbol;
            Record rval2 = symbolTable->search(symbol2.identifier);
            validate(symbol2, rval2);
        }
    }
}

void semanticanalysis::analyze(ASTCondition condition, SymbolTable *symbolTable) {
    if (condition.operand1.type == kOperandSymbol) {
        ASTSymbol symbol = condition.operand1.symbol;
        Record record = symbolTable->search(symbol.identifier);
        validate(symbol, record);
    }

    if (condition.operand2.type == kOperandSymbol) {
        ASTSymbol symbol = condition.operand2.symbol;
        Record record = symbolTable->search(symbol.identifier);
        validate(symbol, record);
    }
}

void semanticanalysis::analyze(ASTIO *io, SymbolTable *symbolTable) {
    if (io->type == kIOReadCommand && io->operand.type != kOperandSymbol) {
        std::stringstream ss;
        ss << "READ cannot be used with constant: " << io->operand.constant;
        throw SymbolException(ss.str());
    }

    if (io->operand.type == kOperandSymbol) {
        ASTSymbol symbol = io->operand.symbol;
        Record record = symbolTable->search(symbol.identifier);
        validate(symbol, record);
    }
}

void semanticanalysis::analyze(ASTBranch *branch, SymbolTable *symbolTable) {
    analyze(branch->condition, symbolTable);
}

void semanticanalysis::analyze(ASTWhileLoop *whileLoop, SymbolTable *symbolTable) {
    analyze(whileLoop->condition, symbolTable);
}

void semanticanalysis::analyze(ASTForLoop *forLoop, SymbolTable* symbolTable) {
    if (symbolTable->contains(forLoop->iterator)) {
        throw SymbolException("Duplicate declaration: " + std::string(forLoop->iterator));
    }

    if (forLoop->iteratorInitial.type == kOperandSymbol) {
        ASTSymbol symbol = forLoop->iteratorInitial.symbol;
        Record record = symbolTable->search(symbol.identifier);
        validate(symbol, record);
    }

    if (forLoop->iteratorBound.type == kOperandSymbol) {
        ASTSymbol symbol = forLoop->iteratorBound.symbol;
        Record record = symbolTable->search(symbol.identifier);
        validate(symbol, record);
    }
}

void semanticanalysis::analyze(ASTNode *node, SymbolTable *symbolTable) {
    if (node == nullptr) {
        return;
    }

    if (node->type == kNodeAssignment) {
        auto* assignment = (ASTAssignment*) node->command;
        analyze(assignment, symbolTable);
    } else if (node->type == kNodeIO) {
        auto* io = (ASTIO*) node->command;
        analyze(io, symbolTable);
    } else if (node->type == kNodeBranch) {
        auto* branch = (ASTBranch*) node->command;
        analyze(branch, symbolTable);
        analyze(branch->ifNode, symbolTable);
        analyze(branch->elseNode, symbolTable);
    } else if (node->type == kNodeForLoop) {
        auto* forLoop = (ASTForLoop*) node->command;
        analyze(forLoop, symbolTable);
        auto* nestedTable = new NestedSymbolTable(symbolTable);
        nestedTable->insert(forLoop->iterator, Record::iterator(forLoop->iterator));
        analyze(forLoop->start, nestedTable);
        delete nestedTable;
    } else if (node->type == kNodeWhileLoop) {
        auto* whileLoop = (ASTWhileLoop*) node->command;
        analyze(whileLoop, symbolTable);
        analyze(whileLoop->start, symbolTable);
    }

    analyze(node->next, symbolTable);
}
