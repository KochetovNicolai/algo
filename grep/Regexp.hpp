#ifndef _REGEXP_
#define _REGEXP_

#include <string>
#include <vector>
#include <map>
#include <stack>
#include <cassert>
#include "DeterministicAutomaton.hpp"

class Regexp {
private:
    struct Operation {
        char letter;
        bool rightAssociative;
        size_t priority;
    };

    struct Token {
        bool isOperation;
        Operation operation;
        Automaton *automaton;
        Token(Operation operation): operation(operation), automaton(NULL), isOperation(true) {}
        Token(Automaton *automaton): automaton(automaton), isOperation(false) {}
    };

    std::map< char, Operation > operations;
    std::map< char, char > screened;
    DeterministicAutomaton automaton;

    void init() {
        std::string op = "(|,+*)";
        int prior[] = {0, 1, 2, 3, 3, 4};
        bool isRight[] = {0, 0, 0, 1, 1, 1};
        for (size_t i = 0; i < op.size(); i++) {
            Operation operation;
            operation.letter = op[i];
            operation.priority = prior[i];
            operation.rightAssociative = isRight[i];
            operations[op[i]] = operation;
        }
        std::string from = "tn";
        std::string to   = "\n\t";
        for (size_t i = 0; i < from.size(); i++) {
            screened[from[i]] = screened[to[i]];
        }
    }
    
    Automaton createAutomatonFromRegexpRPN(std::vector< Token > tokens) {    
        std::stack < Automaton * > automatons;
        for (size_t i = 0; i < tokens.size(); i++) {
            if (tokens[i].isOperation) {
                if (tokens[i].operation.letter == ',') {
                    Automaton *right = automatons.top();
                    automatons.pop();
                    Automaton *left = automatons.top();
                    automatons.pop();
                    left->concatenate(*right);
                    delete right;
                    automatons.push(left);
                } else if (tokens[i].operation.letter == '|') {
                    Automaton *right = automatons.top();
                    automatons.pop();
                    Automaton *left = automatons.top();
                    automatons.pop();
                    left->unite(*right);
                    delete right;
                    automatons.push(left);
                } else if (tokens[i].operation.letter == '*') {
                    Automaton *automaton = automatons.top();
                    automatons.pop();
                    automaton->makeIterationOfKleene();
                    automatons.push(automaton);
                } else if (tokens[i].operation.letter == '+') {
                    Automaton *automaton = automatons.top();
                    automatons.pop();
                    automaton->makePositiveIterationOfKleene();
                    automatons.push(automaton);
                }
            } else {
                automatons.push(tokens[i].automaton);
            }
        }
        assert(automatons.size() == 1);
        Automaton automaton(*automatons.top());
        delete automatons.top();
        return automaton;
    }

    std::vector< Token > resolveString(const std::string &regexp) {
        init();
        std::vector< Token > resolved;
        bool isScreened = false;
        bool isLastLetter = false;
        for (size_t i = 0; i < regexp.size(); i++) {
            if (regexp[i] == '\\') {
                if (isLastLetter)
                    resolved.push_back(Token(operations[',']));
                if (isScreened)
                    resolved.push_back(Token(new Automaton('\\')));
                isLastLetter = !(isScreened = !isScreened);
            } else {
                if (!isScreened && regexp[i] != ',' && operations.find(regexp[i]) != operations.end()) {
                    if (regexp[i] == '(' && isLastLetter)
                        resolved.push_back(Token(operations[',']));
                    isLastLetter = operations[regexp[i]].rightAssociative;
                    resolved.push_back(Token(operations[regexp[i]]));
                } else if (isScreened) {
                    char letter = screened.find(regexp[i]) == screened.end() ? regexp[i] : screened[regexp[i]];
                    resolved.push_back(Token(new Automaton(letter)));
                    isLastLetter = true;
                } else {
                    if (isLastLetter)
                        resolved.push_back(Token(operations[',']));
                    size_t size = 1;
                    if (regexp[i] == '[') {
                        i++;
                        size = 0;
                        while (regexp[i + size] != ']')
                            size++;
                        bool isInverted = regexp[i] == '^';
                        resolved.push_back(Token(new Automaton(regexp.substr(i + isInverted, size - isInverted), isInverted)));
                        i += size;
                    } else {
                        Automaton *automaton;
                        if (regexp[i] == '.') {
                            automaton = new Automaton();
                            automaton->addAnyCaracter(false);
                        } else if (regexp[i] == '?') {
                            automaton = new Automaton();
                            automaton->addAnyCaracter(true);
                        } else {
                            automaton = new Automaton(regexp[i]);
                        }
                        resolved.push_back(Token(automaton));
                    }
                    isLastLetter = true;
                }
                isScreened = false;
            }
        }
        if (resolved.back().isOperation && resolved.back().operation.letter == ',')
            resolved.pop_back();
        return resolved;
    }

    std::vector< Token > ShuntingYardAlgorithm(const std::vector < Token > &input) {
        std::vector< Token > output;
        std::stack< Operation > stack;
        for (size_t i = 0; i < input.size(); i++) {
            if (input[i].isOperation) {
                if (input[i].operation.letter == ')') {
                    while (stack.top().letter != '(') {
                        output.push_back(Token(stack.top()));
                        stack.pop();
                    }
                    stack.pop();
                } else {
                    Operation op = input[i].operation;
                    while (!stack.empty() && op.letter != '(' && ((op.priority < stack.top().priority) ||
                            (op.priority == stack.top().priority && !op.rightAssociative))) {
                        output.push_back(Token(stack.top()));
                        stack.pop();
                    }
                    stack.push(op);
                }
            } else {
                output.push_back(input[i]);
            }
        }
        while (!stack.empty()) {
            output.push_back(Token(stack.top()));
            stack.pop();
        }
        return output;
    }
public:
    Regexp(const std::string &regexp): automaton(createAutomatonFromRegexpRPN(ShuntingYardAlgorithm(resolveString(regexp)))) {}
    std::vector< std::vector< size_t > > grep(const std::string &text) {
        return automaton.grep(text);
    }
};

#endif // _REGEXP_