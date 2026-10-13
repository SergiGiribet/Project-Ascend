#!/usr/bin/env bash
# Resolution equivalence check: does the working tree resolve sorties EXACTLY like BASE_REF?
#
#   app/tests/equivalence.sh [BASE_REF]      (default: phase2)
#
# Builds tests/equivalence.cpp twice -- against BASE_REF's app/src and against the working tree's --
# runs both on a copy of the resources and diffs the outputs. Exit 0 means identical. Everything it
# writes goes under build/equivalence/ (ignored by git). It never runs the game and never touches
# app/save.txt or app/sessions/.
#
# Env: CXX (default g++). The working-tree build must be warning-free, as the project requires;
# warnings are printed and fail the check.
set -euo pipefail

BASE_REF="${1:-phase2}"
CXX="${CXX:-g++}"
FLAGS=(-std=c++17 -O1 -Wall -Wextra -Wpedantic)

ROOT="$(git rev-parse --show-toplevel)"
cd "$ROOT"
BASE_SHA="$(git rev-parse --short "$BASE_REF")"
WORK="build/equivalence"
BASE_DIR="$WORK/base-$BASE_SHA"
RUN_DIR="$WORK/run"
mkdir -p "$BASE_DIR" "$RUN_DIR" "$WORK/bin"

# Baseline sources, one directory per commit so nothing ever has to be cleaned out.
if [ ! -f "$BASE_DIR/.extracted" ]; then
    git archive "$BASE_REF" app/src | tar -x -C "$BASE_DIR"
    touch "$BASE_DIR/.extracted"
fi
cp -r app/resources "$RUN_DIR/"

echo "baseline: $BASE_REF ($BASE_SHA)   compiler: $CXX"

"$CXX" "${FLAGS[@]}" -I "$BASE_DIR/app/src" app/tests/equivalence.cpp "$BASE_DIR"/app/src/*.cpp \
    -o "$WORK/bin/base-$CXX" 2>/dev/null

if ! "$CXX" "${FLAGS[@]}" -I app/src app/tests/equivalence.cpp app/src/*.cpp \
        -o "$WORK/bin/new-$CXX" 2>"$WORK/new-$CXX.warnings"; then
    cat "$WORK/new-$CXX.warnings"
    echo "FAIL: the working tree does not compile"
    exit 2
fi
WARNINGS=$(grep -c "warning:" "$WORK/new-$CXX.warnings" || true)

(cd "$RUN_DIR" && "../bin/base-$CXX" > "../base-$CXX.txt")
(cd "$RUN_DIR" && "../bin/new-$CXX" > "../new-$CXX.txt")

LINES=$(wc -l < "$WORK/base-$CXX.txt")
if diff -u "$WORK/base-$CXX.txt" "$WORK/new-$CXX.txt" > "$WORK/diff-$CXX.txt"; then
    echo "IDENTICAL: $LINES lines of output match"
    STATUS=0
else
    echo "DIFFERENT: first differences (full diff in $WORK/diff-$CXX.txt)"
    head -40 "$WORK/diff-$CXX.txt"
    STATUS=1
fi

if [ "$WARNINGS" -gt 0 ]; then
    cat "$WORK/new-$CXX.warnings"
    echo "FAIL: $WARNINGS warning(s) in the working tree"
    STATUS=3
fi
exit $STATUS
