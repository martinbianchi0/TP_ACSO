; /** defines bool y puntero **/
%define NULL 0
%define TRUE 1
%define FALSE 0

section .data

section .text

global string_proc_list_create_asm
global string_proc_node_create_asm
global string_proc_list_add_node_asm
global string_proc_list_concat_asm

; FUNCIONES auxiliares que pueden llegar a necesitar:
extern malloc
extern free
extern str_concat

; -----------------------------------------------
; string_proc_list_create_asm:
; Crea una nueva lista de procesamiento de cadenas.
; - Reserva 16 bytes en memoria usando malloc.
; - Inicializa los punteros `first` y `last` a NULL (0).
; Retorna:
; - La dirección del bloque de memoria si se asignó correctamente.
; - NULL (0) si la asignación falla.
; -----------------------------------------------
string_proc_list_create_asm:
    push rbp
    mov rbp, rsp

    mov rdi, 16
    call malloc

    test rax, rax
    je .return_null

    mov qword [rax], 0
    mov qword [rax + 8], 0

    pop rbp
    ret

.return_null:
    xor rax, rax     ; rax = 0 (NULL)
    pop rbp
    ret


; ----------------------------------------------------------
; string_proc_node_create_asm:
; Crea un nuevo nodo para una lista de procesamiento de cadenas.
; - Reserva 32 bytes de memoria para el nodo usando malloc.
; - Inicializa los campos:
;   - `next` y `previous` en NULL (0).
;   - `type` con el valor de `dil` (el tipo proporcionado).
;   - `hash` con el valor de `rsi` (el hash proporcionado).
; Retorna:
; - La dirección del nodo creado si la asignación es exitosa.
; - NULL (0) si malloc falla.
; ----------------------------------------------------------
string_proc_node_create_asm:
    push rdi
    push rsi

    mov rdi, 32
    call malloc

    pop rsi
    pop rdi
    test rax, rax
    je .return_null

    mov qword [rax], 0         ; next
    mov qword [rax + 8], 0     ; previous
    mov byte [rax + 16], dil   ; type
    mov qword [rax + 24], rsi  ; hash

    ret

.return_null:
    xor rax, rax
    ret


; ----------------------------------------------------------
; string_proc_list_add_node_asm:
; Agrega un nuevo nodo al final de una lista doblemente enlazada.
; - Argumentos:
;   - `rdi`: Puntero a la lista (`list`).
;   - `sil`: Tipo del nodo (`type`).
;   - `rdx`: Puntero al hash del nodo (`hash`).
; - Retorno:
;   - Si `list` es NULL, no se realiza ninguna operación.
;   - Si la creación del nodo falla, no se realizan cambios en la lista.
;   - Si el nodo se crea correctamente, se actualiza la lista.
; ----------------------------------------------------------
string_proc_list_add_node_asm:
    push rbp
    mov rbp, rsp
    push rbx
    push r12

    mov rbx, rdi        ; list
    test rbx, rbx
    jz .end             ; si list == NULL, salir

    movzx r12, sil
    mov rsi, rdx        ; hash

    movzx edi, r12b
    call string_proc_node_create_asm
    test rax, rax
    jz .end

    cmp qword [rbx], 0
    jne .not_empty

    mov [rbx], rax      ; list->first = node
    mov [rbx + 8], rax  ; list->last = node
    jmp .end

.not_empty:
    mov rcx, [rbx + 8]  ; rcx = list->last
    mov [rax + 8], rcx  ; node->prev = last
    mov [rcx], rax      ; last->next = node
    mov [rbx + 8], rax  ; list->last = node

.end:
    pop r12
    pop rbx
    pop rbp
    ret


; ----------------------------------------------------------
; string_proc_list_concat_asm:
; Genera un nuevo hash concatenando el hash inicial con los hashes
; de los nodos de la lista que coincidan con el tipo especificado.
; - Argumentos:
;   - `rdi`: Puntero a la lista (`list`).
;   - `sil`: Tipo de nodos a considerar para la concatenación (`type`).
;   - `rdx`: Puntero al hash inicial (`hash`).
; - Retorno:
;   - Si `list` es NULL o `hash` es NULL, devuelve NULL.
;   - Si ocurre un error de memoria, devuelve NULL.
;   - En caso de éxito, devuelve un puntero al nuevo hash concatenado
;     (el cual debe liberarse con `free`).
; ----------------------------------------------------------
string_proc_list_concat_asm:
    push rbp
    mov rbp, rsp
    sub rsp, 16
    push rbx
    push r12
    push r13
    push r14
    push r15

    mov rbx, rdi        ; list
    movzx r12, sil      ; type
    mov r13, rdx        ; hash

    ; Chequear si list o hash son NULL
    test rbx, rbx
    jz .return_null
    test r13, r13
    jz .return_null

    ; Calcular la longitud de la cadena hash (strlen) y asignar memoria
    xor rax, rax        ; rax = contador
.len_loop:
    cmp byte [r13 + rax], 0
    je .len_done
    inc rax
    jmp .len_loop
.len_done:
    inc rax             ; Incluir terminador nulo
    mov rdi, rax
    call malloc
    test rax, rax
    jz .error

    ; Copiar hash al buffer resultante
    mov r14, rax        ; r14 = result
    mov rsi, r13        ; source = hash
    mov rdi, r14        ; destination = result
.copy_loop:
    mov cl, [rsi]
    mov [rdi], cl
    test cl, cl
    jz .copy_done
    inc rsi
    inc rdi
    jmp .copy_loop
.copy_done:
    mov r15, [rbx]      ; current_node = list->first

.loop:
    test r15, r15
    jz .success

    movzx eax, byte [r15 + 16]  ; current_node->type
    cmp al, r12b
    jne .next

    mov rdi, r14
    mov rsi, [r15 + 24] ; current_node->hash
    call str_concat
    test rax, rax
    jz .error

    mov rdi, r14
    mov r14, rax
    call free

.next:
    mov r15, [r15]      ; current_node = current_node->next
    jmp .loop

.success:
    mov rax, r14
    jmp .cleanup

.error:
    mov rdi, r14
    call free
    xor rax, rax
    jmp .cleanup

.return_null:
    xor rax, rax
    jmp .cleanup

.cleanup:
    pop r15
    pop r14
    pop r13
    pop r12
    pop rbx
    add rsp, 16
    pop rbp
    ret
