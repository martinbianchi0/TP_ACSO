#!/usr/bin/env bash
# test_ring_final.sh
# Tester completo para el programa “ring”
# Incluye pruebas funcionales y detección de fugas de memoria.

EXECUTABLE="./ring"

# Verificar si el ejecutable existe
if [[ ! -x "$EXECUTABLE" ]]; then
    echo "❌ ERROR: No se encontró el ejecutable \"$EXECUTABLE\". Por favor, compílalo usando:"
    echo "   gcc -Wall -Wextra -std=c99 -o ring ring.c"
    exit 1
fi

# Verificar si Valgrind está disponible
if ! command -v valgrind &> /dev/null; then
    echo "⚠️ Advertencia: Valgrind no está instalado. Las pruebas de memoria no se ejecutarán."
    VALGRIND_AVAILABLE=0
else
    VALGRIND_AVAILABLE=1
fi

echo "=== 🧪 Test Suite para $EXECUTABLE ==="
echo ""

# Casos únicos para pruebas funcionales
functional_tests=(
    "1 5 1        6"    # Caso básico con un proceso
    "2 10 1       12"   # Dos procesos, arranca el primero
    "3 -5 2       -2"   # Valor inicial negativo, arranca el segundo proceso
    "4 0 4         4"   # Último proceso activa el valor inicial
    "10 -50 7     -40"  # Diez procesos, arranca el séptimo proceso
    "50 500 25    550"  # Número medio de procesos con valor inicial medio
    "256 -1 128   255"  # Caso límite con potencias de 2
    "1000 100 500 1100" # Caso límite VLA
)

# Casos únicos para pruebas de memoria
memory_tests=(
    "3 0 1"
    "10 0 5"
    "50 0 25"
    "256 100 128"
)

# Inicialización de contadores de resultados
PASSED=0
FAILED=0
TOTAL_TESTS=0

# Función para ejecutar pruebas funcionales
run_functionality_test() {
    local n=$1
    local c=$2
    local s=$3
    local expected=$4

    (( TOTAL_TESTS++ ))
    echo "🚀 Prueba funcional: Procesos=$n, Valor Inicial=$c, Inicio=$s → Resultado Esperado: $expected"

    # Ejecutar el programa y capturar la salida esperada
    output_line="$($EXECUTABLE "$n" "$c" "$s" 2>/dev/null | grep 'Valor final recibido')"

    # Extraer el número al final de la salida
    if [[ "$output_line" =~ ([0-9-]+)$ ]]; then
        received="${BASH_REMATCH[1]}"
    else
        received="(ERROR)"
    fi

    # Comparar el resultado con el esperado
    if [[ "$received" == "$expected" ]]; then
        echo "✅ PASADO: Esperado=$expected, Obtenido=$received"
        (( PASSED++ ))
    else
        echo "❌ FALLADO: Esperado=$expected, Obtenido=$received"
        (( FAILED++ ))
    fi
}

# Función para ejecutar pruebas de fugas de memoria
run_memory_check() {
    local n=$1
    local c=$2
    local s=$3

    if [[ $VALGRIND_AVAILABLE -eq 0 ]]; then
        echo "⚠️ Prueba de memoria omitida: Valgrind no disponible."
        return
    fi

    echo "🧠 Prueba de memoria: Procesos=$n, Valor Inicial=$c, Inicio=$s"

    # Ejecutar Valgrind
    valgrind --leak-check=full --show-leak-kinds=all --track-fds=yes --error-exitcode=1 $EXECUTABLE $n $c $s &> /dev/null

    # Evaluar el resultado de Valgrind
    if [[ $? -eq 0 ]]; then
        echo "✅ PASADO: No se detectaron fugas de memoria para Procesos=$n, Valor Inicial=$c, Inicio=$s"
    else
        echo "❌ FALLADO: Se detectaron fugas de memoria para Procesos=$n, Valor Inicial=$c, Inicio=$s"
    fi
}

# Ejecutar las pruebas funcionales
for test_case in "${functional_tests[@]}"; do
    read -r n c s expected <<< "$test_case"
    run_functionality_test "$n" "$c" "$s" "$expected"
done

# Ejecutar las pruebas de memoria
echo ""
echo "=== 🧠 Ejecutando pruebas de memoria ==="
for test_case in "${memory_tests[@]}"; do
    read -r n c s <<< "$test_case"
    run_memory_check "$n" "$c" "$s"
done

# === 🚫 Pruebas de manejo de errores ===
error_tests=(
    ""                              # Sin argumentos
    "1"                             # Faltan 2 argumentos
    "1 2"                           # Falta 1 argumento
    "abc 10 1"                      # 'abc' no es número
    "3 10 xyz"                      # 'xyz' no es número
    "-5 10 1"                       # Número de procesos negativo
    "0 0 0"                         # Cero procesos
    "5 10 6"                        # Proceso inicial fuera de rango
)

echo ""
echo "=== 🚫 Ejecutando pruebas de manejo de errores ==="

for test_case in "${error_tests[@]}"; do
    echo "🚨 Prueba de error con argumentos: $test_case"
    output="$($EXECUTABLE $test_case 2>&1)"
    status=$?
    if [[ $status -ne 0 ]]; then
        echo "✅ PASADO: Código de salida = $status. Mensaje: $(echo "$output" | head -n1)"
    else
        echo "❌ FALLADO: Se esperaba error pero el código de salida fue 0"
        echo "   Salida: $output"
        (( FAILED++ ))
    fi
    (( TOTAL_TESTS++ ))
done