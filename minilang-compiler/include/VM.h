#pragma once
#include "IRGen.h"
#include <unordered_map>
#include <string>
#include <vector>

// A minimal stack-machine-adjacent interpreter that executes linear
// three-address code directly (no separate bytecode encoding step -
// instructions are dispatched by opcode string).
class VM {
public:
    void run(const std::vector<Instr>& code);
    const std::unordered_map<std::string, int>& variables() const { return vars; }

private:
    std::unordered_map<std::string, int> vars;

    int resolve(const std::string& s) const;
};
