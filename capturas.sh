#!/usr/bin/env bash
# Muestra, una pantalla a la vez, la salida de cada herramienta para tomar
# las capturas de los "Archivos de soporte" del informe.
# Uso: ./capturas.sh        (Enter = siguiente captura, Ctrl+C = salir)
set -uo pipefail
cd "$(dirname "$0")"

[ -x build/v1.0-incidente/falcon_sim ] && [ -f reportes/comparacion.md ] || ./analizar.sh >/dev/null 2>&1

N=0
paso() {
  N=$((N + 1))
  clear
  printf '\033[1m=== Captura %02d · %s ===\033[0m\n\n' "$N" "$1"
  printf '\033[2m$ %s\033[0m\n\n' "$2"
  eval "$2"
  printf '\n\033[2m[Enter para continuar]\033[0m'
  read -r
}

SA="clang --analyze -std=c11 -Xanalyzer -analyzer-output=text -Xclang -analyzer-checker=core,unix,deadcode,security,alpha.security.ArrayBoundV2"

paso "Herramientas y versiones" \
  "cppcheck --version; clang --version | head -1; .venv/bin/python --version; .venv/bin/pytest --version; .venv/bin/lizard --version"

for V in v1.0-incidente v1.1-corregida; do
  S=src/$V; B=build/$V; R=reportes/$V

  paso "$V · Validador y ejecución con el Channel File del incidente" \
    "$B/falcon_sim validate data/cf291_incidente.txt; ASAN_OPTIONS=abort_on_error=0 $B/falcon_sim run data/cf291_incidente.txt --events 50 2>&1 | grep -v '^ *0x\\|^  \\|Shadow\\|HINT' | head -14; echo \"código de salida: \${PIPESTATUS[0]}\""

  paso "$V · Cppcheck (análisis estático, sin MISRA)" \
    "cppcheck -q --enable=all --inconclusive --std=c11 --suppress=missingIncludeSystem --template='[{severity}] {file}:{line} ({id}) {message}' $S 2>&1 | grep -v checkersReport"

  paso "$V · Cppcheck + MISRA C 2012 (ocurrencias por regla)" \
    "cppcheck -q --enable=all --std=c11 --suppress=missingIncludeSystem --addon=misra --template='{severity} {id} {file}:{line}' $S 2>&1 | grep -v '^information' | awk '{print \$1, \$2}' | sort | uniq -c | sort -rn"

  paso "$V · Clang Static Analyzer" \
    "$SA $S/*.c 2>&1 | grep 'warning:' | sed 's/ is insecure.*\\[/ … [/'"

  paso "$V · Lizard (complejidad ciclomática y duplicación)" \
    ".venv/bin/lizard -Eduplicate $S 2>/dev/null | tail -22"

  paso "$V · PyTest (suite de la Tabla 2)" \
    "FALCON_BIN=$B/falcon_sim LLVM_PROFILE_FILE=/dev/null ASAN_OPTIONS=detect_leaks=0 .venv/bin/pytest tests -v -p no:cacheprovider -p no:html -p no:metadata --tb=no -rN | sed 's|^tests/test_channel_291.py::||'"

  paso "$V · Cobertura de código C (llvm-cov)" \
    "$(command -v xcrun >/dev/null && echo xcrun) llvm-cov report $B/falcon_sim -instr-profile=$R/cov/falcon.profdata $S/*.c | cut -c1-30,66-105,106-185"
done

paso "Comparación v1.0 → v1.1" "cat reportes/comparacion.md"

clear
echo "Listo. Reportes HTML para capturar en el navegador:"
echo "  open reportes/v1.0-incidente/pytest.html"
echo "  open reportes/v1.0-incidente/cppcheck-html/index.html"
echo "  open reportes/v1.0-incidente/cobertura-html/index.html"
echo "  (y los mismos en reportes/v1.1-corregida/)"
