#!/usr/bin/env bash
set -euo pipefail

# Directorio raíz del proyecto (un nivel arriba de tests/)
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(dirname "$SCRIPT_DIR")"

TEST_DIR="$(mktemp -d /tmp/mlfq-test-XXXXXX)"
EXECUTABLE="$TEST_DIR/mlfq_test"

cleanup() {
    rm -f "$EXECUTABLE"
    rm -rf "$TEST_DIR"
}
trap cleanup EXIT

INPUT="4
20
0
8
1
4
2
9
3
5"

normalize() {
    # Quita \r y espacios/líneas en blanco al final del archivo
    sed 's/\r$//' "$1" | sed -e :a -e '/^\n*$/{$d;N;ba' -e '}'
}

assert_file_matches() {
    local actual_path="$1"
    local expected_path="$2"
    local name
    name="$(basename "$actual_path")"

    if [ ! -f "$actual_path" ]; then
        echo "FAIL: $name (no se generó el archivo)"
        return 1
    fi

    if diff <(normalize "$actual_path") <(normalize "$expected_path") > /dev/null; then
        echo "PASS: $name"
        return 0
    else
        echo "FAIL: $name"
        echo "Resultado esperado: $expected_path"
        echo "Resultado obtenido: $actual_path"
        echo "--- Diferencias ---"
        diff <(normalize "$actual_path") <(normalize "$expected_path") || true
        return 1
    fi
}

cd "$ROOT"

echo "Compilando..."
gcc -std=c99 -Wall -Wextra -pedantic main.c process.c queue.c scheduler.c -o "$EXECUTABLE"

echo "Ejecutando..."
cd "$TEST_DIR"
echo "$INPUT" | "$EXECUTABLE" > /dev/null

results_passed=0
schedule_passed=0

assert_file_matches "$TEST_DIR/results.csv" "$SCRIPT_DIR/expected_results.csv" && results_passed=1 || results_passed=0
assert_file_matches "$TEST_DIR/schedule.txt" "$SCRIPT_DIR/expected_schedule.txt" && schedule_passed=1 || schedule_passed=0

if [ "$results_passed" -eq 1 ] && [ "$schedule_passed" -eq 1 ]; then
    echo "PASS: escenario completo"
    exit 0
else
    echo "FAIL: escenario completo"
    exit 1
fi
