# MiniLang Compiler

A small compiler + interpreter for a C-like expression language, built in
modern C++17 with no external dependencies. Built as a hands-on exercise
in classic compiler pipeline stages: lexing, recursive-descent parsing,
AST-level optimization passes, three-address IR generation, and execution.

```
source (.mini)
    -> Lexer          tokenizes source into a token stream
    -> Parser         recursive-descent parse into an AST
    -> Optimizer       constant folding + dead-code elimination on the AST
    -> IRGen           lowers AST into linear three-address code (TAC)
    -> VM               executes the TAC directly
```

## Language features

- Variables (`let x = ...;`), assignment, integer arithmetic (`+ - * /`)
- Comparisons (`< > == !=`)
- `if` / `else`, `while`
- `print`
- `//` line comments

## Optimization passes

**Constant folding** — evaluates any `BinaryOp`/`UnaryOp` node whose
operands are already compile-time constants, collapsing entire constant
subexpressions into a single value before code generation.

**Dead code elimination** — after folding, any `if` whose condition
reduced to a compile-time constant is replaced by just its live branch;
any `while` whose condition reduced to a compile-time-false constant is
dropped entirely, since it can never execute.

Both passes are conservative: they only act on subtrees that are
*provably* constant (e.g. `2 - 2`), never on expressions involving a
variable's runtime value (e.g. `0 * a` is left untouched, correctly,
since `a` isn't known until the program runs).

Example — `examples/fold.mini` computes `(2+3)*(10-2)+2`:

| | Instructions | 
|---|---|
| Unoptimized | 6 (4 arithmetic ops + move + print) |
| Optimized | 2 (`MOV 42 -> x`, `PRINT x`) |

`examples/deadcode.mini` (constant-false `if`/`while`): 15 unoptimized
instructions collapse to 3, with byte-for-byte identical program output
in both cases — see `tests/run_tests.sh`, which asserts this
semantics-preserving property automatically.

## Build

Requires a C++17 compiler. No external dependencies.

```bash
g++ -std=c++17 -O2 -Wall -Wextra -Iinclude src/*.cpp -o build/minilangc
```

(A `CMakeLists.txt` is also included if you prefer `cmake && make`.)

## Run

```bash
./build/minilangc examples/fib.mini
./build/minilangc examples/fold.mini --dump-ir       # show IR + fold/DCE stats
./build/minilangc examples/fold.mini --no-optimize   # disable optimizer for comparison
```

## Test

```bash
./tests/run_tests.sh
```

Runs every example through both the optimized and unoptimized pipeline
and asserts identical output, plus known-value checks against expected
results.

## Project layout

```
include/   Lexer.h  AST.h  Parser.h  Optimizer.h  IRGen.h  VM.h
src/       matching .cpp files + main.cpp (CLI driver)
examples/  sample .mini programs demonstrating each pass
tests/     regression test harness
```

## Possible extensions

- Register allocation over the generated TAC
- Functions/procedures with a call stack
- Common subexpression elimination, loop-invariant code motion
- A real bytecode format instead of interpreting TAC directly
