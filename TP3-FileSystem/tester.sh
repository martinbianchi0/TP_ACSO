#!/usr/bin/env bash
set -e

### === RUTAS ===
DISK_DIR=samples/testdisks
OUT_DIR=outputs/testout
REF_DIR=outputs/refout
VALGRIND_DIR=outputs/valgrind

mkdir -p "$OUT_DIR" "$REF_DIR" "$VALGRIND_DIR"
rm -f "$OUT_DIR"/*.txt "$REF_DIR"/*.txt "$VALGRIND_DIR"/*.log

### === LISTA DE DISCOS (excluye los .gold) ===
DISKS=($(find "$DISK_DIR" -type f ! -name "*.gold"))

echo "🧪 Ejecutando tests sobre las siguientes imágenes:"
for d in "${DISKS[@]}"; do echo " - $d"; done

### === TESTEO NORMAL + VALGRIND ===
echo "🔧 Corriendo ejecutables..."

for disk in "${DISKS[@]}"; do
  name=$(basename "$disk")

  # salida propia
  ./diskimageaccess -ip "$disk" > "$OUT_DIR/$name.txt"

  # salida gold
  ./samples/diskimageaccess_soln_x86 -ip "$disk" > "$REF_DIR/$name.txt"

  # valgrind
  valgrind --leak-check=full --show-leak-kinds=all \
    --error-exitcode=99 ./diskimageaccess -ip "$disk" \
    > /dev/null 2> "$VALGRIND_DIR/$name.log" || echo "⚠️  Memory leak detectado en $name"
done

### === COMPARACIÓN DE SALIDAS ===
echo "📊 Comparando resultados contra solución oficial..."
MISMATCH=0

for file in "$OUT_DIR"/*.txt; do
  fname=$(basename "$file")
  ref="$REF_DIR/$fname"

  if [ ! -f "$ref" ]; then
    echo "❌ Falta referencia: $ref"
    MISMATCH=1
  elif ! diff -q "$file" "$ref" > /dev/null; then
    echo "❌ Diferencia en salida: $fname"
    MISMATCH=1
  fi
done

if [ "$MISMATCH" -eq 0 ]; then
  echo "✅ Test funcional PASADO (outputs coinciden)"
else
  echo "⚠️  Test funcional FALLIDO. Revisá diferencias en outputs/"
fi

### === CHEQUEO DE LEAKS CON VALGRIND ===
echo "🧠 Chequeando posibles memory leaks..."
LEAKS=0

for log in "$VALGRIND_DIR"/*.log; do
  if grep -qE "definitely lost: [1-9]|indirectly lost: [1-9]" "$log"; then
    echo "❌ Leak detectado en $(basename "$log")"
    LEAKS=1
  fi
done

if [ "$LEAKS" -eq 0 ]; then
  echo "✅ Valgrind OK (sin pérdidas de memoria detectadas)"
else
  echo "⚠️  Hay pérdidas de memoria. Revisá logs en $VALGRIND_DIR/"
fi