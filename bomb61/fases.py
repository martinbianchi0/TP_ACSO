from itertools import product
import string


def phase_2():
    """
    Fase 2: Encuentra combinaciones de tres números donde:
    - num3 = (num1 XOR num2) >> 1
    - num3 debe ser negativo
    """
    print("\n[Phase 2] Combinaciones válidas:")
    for num1 in range(-3, -1):
        for num2 in range(3, 5):
            xor_result = num1 ^ num2
            num3 = xor_result >> 1
            if num3 < 0:
                print(num1, num2, num3)


def cuenta(palabra, lista, izq, der):
    """
    Función recursiva similar a una búsqueda binaria modificada.
    Devuelve la suma de índices visitados hasta encontrar la palabra.
    """
    if izq > der:
        return 0

    mid = ((izq ^ der) >> 1) + (izq & der)
    cmp = (palabra > lista[mid]) - (palabra < lista[mid])

    if cmp == 0:
        return mid
    elif cmp < 0:
        return mid + cuenta(palabra, lista, izq, mid - 1)
    else:
        return mid + cuenta(palabra, lista, mid + 1, der)


def phase_3():
    """
    Fase 3: Busca una palabra en una lista ordenada y retorna la suma de los índices visitados.
    """
    print("\n[Phase 3] Resultado de búsqueda con 'cuenta':")
    with open("C:/Users/bianc/Arquitectura/TP_ACSO/TP2-x86_64/bomb61/palabras.txt", "r", encoding="utf-8") as f:
        lista = [line.strip() for line in f]
    resultado = cuenta("agrupar", lista, 0, len(lista) - 1)
    print(f"  Resultado: {resultado}")


def chars_for_index(index):
    """
    Retorna todos los caracteres imprimibles cuyo valor (ord) & 0xf coincide con 'index'.
    """
    return [c for c in string.printable if (ord(c) & 0xf) == index]


def phase_4():
    """
    Fase 4: Dado un array de índices, genera posibles combinaciones de caracteres
    donde (ord(caracter) & 0xf) == valor de índice.
    """
    combo = (6, 7, 10, 14, 15, 15)
    opciones_caracteres = [chars_for_index(idx) for idx in combo]

    print("\n[Phase 4] Algunas combinaciones posibles para input:")
    for i, opcion in enumerate(product(*opciones_caracteres)):
        print(f"  {i+1:2d}: {''.join(opcion)}")
        if i == 9:
            break


def main():
    print("== Simulación de niveles de la bomba ==\n")
    phase_2()
    phase_3()
    phase_4()


if __name__ == "__main__":
    main()