//
// Created by Kacper Raczy on 2019-09-18.
//

#include "ASTOptimizer.hpp"

void optimizeList(ASTNode** source) {
    ASTNode* current = *source;
    ASTNode* start = current;
    int res;
    while (current != nullptr) {
        res = astoptimizer::optimize(current);
        if (res == OPTIMIZER_BODY_REMOVED) {
            start = nullptr;
            break;
        } else if (res == OPTIMIZER_NEXT_NEW_START) {
            start = current->next;
        }

        current = current->next;
    }

    *source = start;
}

void astoptimizer::optimize(ASTProgram* program) {
    ASTNode** source = &program->start;
    optimizeList(source);
}

int astoptimizer::optimize(ASTNode* node) {
    ASTNode* prev = node->prev;
    ASTNode* next = node->next;
    int result = ACTION_NOTHING;

    if (node->type == kNodeAssignment) {
        auto *command = (ASTAssignment *) node->command;
        result = optimize(command);
    } else if (node->type == kNodeIO) {
        return 0;
    } else if (node->type == kNodeWhileLoop) {
        auto* command = (ASTWhileLoop*) node->command;
        result = optimize(command);
        if (result == ACTION_EXTRACT_BODY1) {
            ASTNode* body = command->start;
            if (prev != nullptr) {
                prev->next = body;
                body->prev = prev;
            }

            while (body->next != nullptr) {
                body = body->next;
            }

            if (next != nullptr) {
                body->next = next;
                next->prev = body;
            }
        }

        if (result == ACTION_NOTHING) {
            // TODO optimize body
        }
    } else if (node->type == kNodeForLoop) {
        auto* command = (ASTForLoop*) node->command;
        result = optimize(command);

        if (result == ACTION_NOTHING) {
            // TODO optimize body
        }
    } else if (node->type == kNodeBranch) {
        auto* command = (ASTBranch*) node->command;
        result = optimize(command);
        if (result == ACTION_EXTRACT_BODY1 || result == ACTION_EXTRACT_BODY2) {
            ASTNode* body;
            if (result == ACTION_EXTRACT_BODY1) {
                body = command->ifNode;
                ASTNodeFreeList(command->elseNode);
            } else {
                body = command->elseNode;
                ASTNodeFreeList(command->ifNode);
            }

            if (prev != nullptr) {
                prev->next = body;
                body->prev = prev;
            }

            while (body->next != nullptr) {
                body = body->next;
            }

            if (next != nullptr) {
                body->next = next;
                next->prev = body;
            }
        } else if (result == ACTION_NOTHING) {
            // TODO optimize ifBody
            // TODO optimize elseBody
        }
    }

    if (result == ACTION_REMOVE) {
        if (prev != nullptr) prev->next = next;
        if (next != nullptr) next->prev = prev;
        ASTNodeFree(node); // TODO Free command (nested blocks)
        if (prev == nullptr && next == nullptr) {
            return OPTIMIZER_BODY_REMOVED;
        } else if (prev == nullptr) {
            next->prev = nullptr;
            return OPTIMIZER_NEXT_NEW_START; // TODO how to access next if node is freed
        } else {
            prev->next = next;
            if (next != nullptr) next->prev = prev;
        }
    }

    return 0;
}

ASTConstant evaluateConstantExpression(ASTConstant c1, ASTConstant c2, int op) {
    switch (op) {
        case kOperatorAdd:
            return c1 + c2;
        case kOperatorSub:
            if (c2 > c1) return 0;
            return c1 - c2;
        case kOperatorMul:
            return c1 * c2;
        case kOperatorDiv:
            if (c2 == 0) return 0;
            return c1 / c2;
        default: // kOperatorMod
            if (c2 == 0) return 0;
            return c1 % c2;
    }
}

int astoptimizer::optimize(ASTAssignment* assignment) {
    if (assignment->rtype == kRTypeOperand) {
        ASTOperand operand = assignment->operand;
        if (operand.type == kOperandSymbol && ASTSymbolEqual(assignment->symbol, operand.symbol)) {
            return ACTION_REMOVE;
        }
    } else if (assignment->rtype == kRTypeExpression) {
        ASTExpression expr = assignment->expression;
        if (expr.operand1.type == kOperandConstant && expr.operand2.type == kOperandConstant) {
            assignment->rtype = kRTypeOperand;
            assignment->operand.type = kOperandConstant;
            assignment->operand.constant = evaluateConstantExpression(expr.operand1.constant, expr.operand2.constant, expr.op);
            return ACTION_EDIT;
        }
    }

    return ACTION_NOTHING;
}

bool evaluateConstantCondition(ASTConstant c1, ASTConstant c2, int op) {
    switch (op) {
        case kCondOperatorEqual:
            return c1 == c2;
        case kCondOperatorNotEqual:
            return c1 != c2;
        case kCondOperatorGreater:
            return c1 > c2;
        case kCondOperatorGreaterEqual:
            return c1 >= c2;
        case kCondOperatorLess:
            return c1 < c2;
        default: // kCondOperatorLessEqual
            return c1 <= c2;
    }
}

int astoptimizer::evaluateCondition(ASTCondition condition) {
    ASTOperand operand1 = condition.operand1;
    ASTOperand operand2 = condition.operand2;
    if (operand1.type == kOperandConstant && operand2.type == kOperandConstant) {
        ASTConstant c1 = operand1.constant;
        ASTConstant c2 = operand2.constant;
        if (evaluateConstantCondition(c1, c2, condition.op)) {
            return CONDITION_ATRUE;
        } else {
            return CONDITION_AFALSE;
        }
    } else if (operand1.type == kOperandSymbol &&
               operand2.type == kOperandSymbol &&
               ASTSymbolEqual(operand1.symbol, operand2.symbol)) {
        switch (condition.op) {
            case kCondOperatorEqual:
            case kCondOperatorGreaterEqual:
            case kCondOperatorLessEqual:
                return CONDITION_ATRUE;
            default:
                return CONDITION_AFALSE;
        }
    }

    return CONDITION_OK;
}

int astoptimizer::optimize(ASTBranch* branch) {
    int flag = evaluateCondition(branch->condition);
    if (flag == CONDITION_ATRUE) {
        if (branch->ifNode == nullptr) {
            return ACTION_REMOVE;
        }  else {
            return ACTION_EXTRACT_BODY1;
        }
    } else if (flag == CONDITION_AFALSE) {
        if (branch->elseNode == nullptr) {
            return ACTION_REMOVE;
        } else {
            return ACTION_EXTRACT_BODY2;
        }
    }

    return ACTION_NOTHING;
}

int astoptimizer::optimize(ASTForLoop* forLoop) {
    if (forLoop->start == nullptr) {
        return ACTION_REMOVE;
    }

    ASTOperand initial = forLoop->iteratorInitial;
    ASTOperand bound = forLoop->iteratorBound;
    if (initial.type == kOperandConstant && bound.type == kOperandConstant) {
        if (forLoop->type == kLoopForTo) {
            return (initial.constant <= bound.constant) ? ACTION_NOTHING : ACTION_REMOVE;
        } else {
            return (initial.constant >= bound.constant) ? ACTION_NOTHING : ACTION_REMOVE;
        }
    }

    return ACTION_NOTHING;
}

int astoptimizer::optimize(ASTWhileLoop* loop) {
    if (loop->start == nullptr) {
        return ACTION_REMOVE;
    }

    if (evaluateCondition(loop->condition) == CONDITION_AFALSE) {
        if (loop->type == kLoopWhile) {
            return ACTION_REMOVE;
        } else {
            return ACTION_EXTRACT_BODY1;
        }
    }

    return ACTION_NOTHING;
}
