#!/usr/bin/env bash
# Simple regression harness: runs each example both optimized and
# unoptimized, asserts identical output (proves optimizations are
# semantics-preserving), and checks expected values for known programs.
set -uo pipefail
cd "$(dirname "$0")/.."
BIN=./build/minilangc
PASS=0
FAIL=0

check_equal_outputs() {
    local file="$1"
    local opt unopt
    opt=$("$BIN" "$file")
    unopt=$("$BIN" "$file" --no-optimize)
    if [[ "$opt" == "$unopt" ]]; then
        echo "PASS: $file (optimized == unoptimized output)"
        PASS=$((PASS+1))
    else
        echo "FAIL: $file (optimized output differs from unoptimized!)"
        echo "  optimized:   $opt"
        echo "  unoptimized: $unopt"
        FAIL=$((FAIL+1))
    fi
}

check_output() {
    local file="$1" expected="$2"
    local actual
    actual=$("$BIN" "$file")
    if [[ "$actual" == "$expected" ]]; then
        echo "PASS: $file matches expected output"
        PASS=$((PASS+1))
    else
        echo "FAIL: $file"
        echo "  expected: $expected"
        echo "  actual:   $actual"
        FAIL=$((FAIL+1))
    fi
}

echo "== Semantics-preservation checks (optimized vs unoptimized) =="
check_equal_outputs examples/fold.mini
check_equal_outputs examples/deadcode.mini
check_equal_outputs examples/fib.mini

echo ""
echo "== Known-value checks =="
check_output examples/fold.mini "42"
check_output examples/fib.mini "$(printf '0\n1\n1\n2\n3\n5\n8\n13\n21\n34')"

echo ""
echo "$PASS passed, $FAIL failed"
[[ $FAIL -eq 0 ]] && exit 0 || exit 1
