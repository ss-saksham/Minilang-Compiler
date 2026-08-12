#include "VM.h"
#include <iostream>
#include <stdexcept>
#include <cctype>

int VM::resolve(const std::string& s) const {
    if (s.empty()) throw std::runtime_error("VM: empty operand");
    bool isNumeric = true;
    size_t start = (s[0] == '-') ? 1 : 0;
    if (start >= s.size()) isNumeric = false;
    for (size_t i = start; i < s.size() && isNumeric; ++i)
        if (!isdigit(static_cast<unsigned char>(s[i]))) isNumeric = false;

    if (isNumeric) return std::stoi(s);
    auto it = vars.find(s);
    return (it != vars.end()) ? it->second : 0; // uninitialized vars default to 0
}

void VM::run(const std::vector<Instr>& code) {
    // First pass: resolve label -> instruction index.
    std::unordered_map<std::string, size_t> labels;
    for (size_t i = 0; i < code.size(); ++i)
        if (code[i].op == "LABEL") labels[code[i].result] = i;

    size_t pc = 0;
    while (pc < code.size()) {
        const Instr& in = code[pc];

        if (in.op == "LABEL") { pc++; continue; }

        if (in.op == "MOV") { vars[in.result] = resolve(in.arg1); pc++; continue; }
        if (in.op == "NEG") { vars[in.result] = -resolve(in.arg1); pc++; continue; }

        if (in.op == "ADD") { vars[in.result] = resolve(in.arg1) + resolve(in.arg2); pc++; continue; }
        if (in.op == "SUB") { vars[in.result] = resolve(in.arg1) - resolve(in.arg2); pc++; continue; }
        if (in.op == "MUL") { vars[in.result] = resolve(in.arg1) * resolve(in.arg2); pc++; continue; }
        if (in.op == "DIV") {
            int divisor = resolve(in.arg2);
            if (divisor == 0) throw std::runtime_error("VM: division by zero");
            vars[in.result] = resolve(in.arg1) / divisor;
            pc++; continue;
        }
        if (in.op == "LT")  { vars[in.result] = (resolve(in.arg1) <  resolve(in.arg2)) ? 1 : 0; pc++; continue; }
        if (in.op == "GT")  { vars[in.result] = (resolve(in.arg1) >  resolve(in.arg2)) ? 1 : 0; pc++; continue; }
        if (in.op == "EQ")  { vars[in.result] = (resolve(in.arg1) == resolve(in.arg2)) ? 1 : 0; pc++; continue; }
        if (in.op == "NEQ") { vars[in.result] = (resolve(in.arg1) != resolve(in.arg2)) ? 1 : 0; pc++; continue; }

        if (in.op == "PRINT") { std::cout << resolve(in.arg1) << "\n"; pc++; continue; }

        if (in.op == "JMP") { pc = labels.at(in.result); continue; }
        if (in.op == "JZ") {
            if (resolve(in.arg1) == 0) pc = labels.at(in.result);
            else pc++;
            continue;
        }

        throw std::runtime_error("VM: unknown opcode '" + in.op + "'");
    }
}
