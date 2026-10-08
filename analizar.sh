#!/usr/bin/env bash
# Ejecuta el análisis completo de cada versión y deja todo en reportes/.
# Requisitos: clang (Xcode CLT), cppcheck (brew install cppcheck), .venv con requirements.txt
set -uo pipefail
cd "$(dirname "$0")"
VERSIONES=(v1.0-incidente v1.1-corregida)
# llvm-cov/llvm-profdata: en macOS vienen con xcrun; en Linux, en el paquete llvm
if command -v xcrun >/dev/null; then LLVM="xcrun "; else LLVM=""; fi
rm -rf reportes build

analizar() {
  local V=$1 SRC=src/$1 R=reportes/$1 B=build/$1
  mkdir -p "$R/cov" "$B/cppcheck"
  echo
  echo "################ $V ################"

  echo "== 1. Compilación (AddressSanitizer + cobertura)"
  clang -g -O0 -std=c11 -fsanitize=address -fno-omit-frame-pointer \
        -fprofile-instr-generate -fcoverage-mapping -fcoverage-compilation-dir=. -Wno-deprecated-declarations \
        "$SRC"/*.c -o "$B/falcon_sim" || return 1

  echo "== 2. Cppcheck $(cppcheck --version | cut -d' ' -f2) (reglas propias + MISRA C 2012)"
  cppcheck -q --enable=all --inconclusive --std=c11 --check-level=exhaustive \
           --suppress=missingIncludeSystem --addon=misra --cppcheck-build-dir="$B/cppcheck" \
           --xml --xml-version=2 "$SRC" 2> "$R/cppcheck.xml"
  .venv/bin/python "$(command -v cppcheck-htmlreport)" --file="$R/cppcheck.xml" \
           --report-dir="$R/cppcheck-html" --source-dir=. --title="falcon-sim $V" >/dev/null 2>&1 \
    || echo "   (cppcheck-htmlreport no disponible, se omite HTML)"

  echo "== 3. Clang Static Analyzer $(clang --version | head -1 | grep -oE '[0-9]+\.[0-9]+\.[0-9]+' | head -1)"
  clang --analyze -std=c11 -Xanalyzer -analyzer-output=text \
        -Xclang -analyzer-checker=core,unix,deadcode,security,alpha.security.ArrayBoundV2 \
        "$SRC"/*.c 2> "$R/clang-sa.txt"

  echo "== 4. Lizard (complejidad ciclomática y duplicación)"
  .venv/bin/lizard -Eduplicate "$SRC" > "$R/lizard.txt" 2>&1
  .venv/bin/lizard --csv "$SRC" > "$R/lizard.csv" 2>/dev/null

  echo "== 5. PyTest (suite de la Tabla 2)"
  FALCON_BIN="$PWD/$B/falcon_sim" LLVM_PROFILE_FILE="$PWD/$R/cov/%p.profraw" \
  ASAN_OPTIONS=detect_leaks=0 \
    .venv/bin/pytest tests -q -p no:cacheprovider --tb=no \
    --junitxml="$R/pytest-junit.xml" --html="$R/pytest.html" --self-contained-html | tail -1

  echo "== 6. Cobertura de código C (llvm-cov)"
  ${LLVM}llvm-profdata merge -sparse "$R"/cov/*.profraw -o "$R/cov/falcon.profdata"
  ${LLVM}llvm-cov report "$B/falcon_sim" -instr-profile="$R/cov/falcon.profdata" "$SRC"/*.c > "$R/cobertura.txt"
  ${LLVM}llvm-cov show "$B/falcon_sim" -instr-profile="$R/cov/falcon.profdata" \
        -format=html -output-dir="$R/cobertura-html"
  rm -f "$R"/cov/*.profraw
}

for V in "${VERSIONES[@]}"; do analizar "$V"; done

echo
echo "== 7. Consolidación de hallazgos, comparación y gráficos"
.venv/bin/python scripts/resumen.py "${VERSIONES[@]/#/reportes/}"
