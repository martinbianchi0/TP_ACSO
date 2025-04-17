#include <stdbool.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include "shell.h"
#include "inttypes.h"

void decode_instruction();
void decode_adds_extended(uint32_t instruction);
void decode_adds_immediate(uint32_t instruction);
void decode_subs_extended(uint32_t instruction);
void decode_subs_immediate(uint32_t instruction);
void decode_halt(uint32_t instruction);
void decode_cmp_immediate(uint32_t instruction);
void decode_cmp_extended(uint32_t instruction);
void decode_ands(uint32_t instruction);
void decode_eor(uint32_t instruction);
void decode_orr(uint32_t instruction);
void decode_b_cond(uint32_t instruction);
void decode_b(uint32_t instruction);
void decode_br(uint32_t instruction);
void decode_movz(uint32_t instruction);
void decode_add_extended_register(uint32_t instruction);
void decode_add_immediate(uint32_t instruction);
void decode_cbz(uint32_t instruction);
void decode_cbnz(uint32_t instruction);
void decode_mul(uint32_t instruction);
void decode_stur(uint32_t instruction);
void decode_sturb(uint32_t instruction);
void decode_sturh(uint32_t instruction);
void decode_ldur(uint32_t instruction);
void decode_ldurb(uint32_t instruction);
void decode_ldurh(uint32_t instruction);
bool calculate_address(uint32_t instruction, uint64_t *address, uint32_t *Rt);
void decode_lsl_lsr_imm(uint32_t instruction);




void process_instruction()
{
    /* execute one instruction here. You should use CURRENT_STATE and modify
     * values in NEXT_STATE. You can call mem_read_32() and mem_write_32() to
     * access memory. 
     * */
    printf("Processing instruction\n");
    decode_instruction();
}


typedef struct instruction_information{
    uint32_t opcode;
    void* function;
} inst_info; 


inst_info INSTRUCTION_SET[] = {
    {0b10101011000, &decode_adds_extended},
    {0b10110001, &decode_adds_immediate},
    {0b11101011000, &decode_subs_extended},
    {0b11110001, &decode_subs_immediate},
    {0b11010100010, &decode_halt},
    {0b11110001 , &decode_cmp_immediate},
    {0b11101011001, &decode_cmp_extended}, 
    {0b11101010000, &decode_ands},
    {0b11001010000, &decode_eor},
    {0b10101010000, &decode_orr},
    {0b01010100, &decode_b_cond},
    {0b11010010100, &decode_movz},
    {0b000101, &decode_b},
    {0b11010110000, &decode_br},
    {0b10010001, &decode_add_immediate},
    {0b10001011000, &decode_add_extended_register}, //preguntar opcode porque enverdad termina en 1 por el simulador me lo tire con 0
    {0b10110101, &decode_cbnz},
    {0b10110100, &decode_cbz},
    {0b10011011000, &decode_mul},
    {0b11111000000, &decode_stur},
    {0b00111000000, &decode_sturb},
    {0b01111000000, &decode_sturh},
    {0b11111000010, &decode_ldur},
    {0b00111000010, &decode_ldurb},
    {0b01111000010, &decode_ldurh},
    {0b110100110, &decode_lsl_lsr_imm}

};

#define INSTRUCTION_SET_SIZE (sizeof(INSTRUCTION_SET) / sizeof(INSTRUCTION_SET[0]))

/** 
 * Actualiza los flags y opcionalmente almacena el resultado en un registro.  
 * 
 * - FLAG_N se establece según el bit más significativo del resultado.  
 * - FLAG_Z se establece en 1 si el resultado es 0, de lo contrario, 0.  
 * - PC se incrementa en 4 para avanzar a la siguiente instrucción.  
 * - Si `rd == -1`, no almacena el resultado (uso en CMP).  
 *
 * Params:  
 *   - result (uint64_t): Resultado de la operación aritmética o lógica.  
 *   - rd (int32_t): Registro de destino. Si es -1, no se almacena resultado.  
 */
void update_result_and_flags(uint64_t result, int32_t rd) {
    if (rd != -1) {
        NEXT_STATE.REGS[rd] = result;
    }
    NEXT_STATE.FLAG_N = (result >> 63) & 1;
    NEXT_STATE.FLAG_Z = (result == 0) ? 1 : 0;
    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
}


/**
 * Aplica el desplazamiento especificado a un valor inmediato.
 * 
 * Params: imm (uint32_t): Valor inmediato a desplazar.
 *         shift (uint32_t): Tipo de desplazamiento (esperado 0 o 1).
 * 
 * Returns: uint32_t: Valor desplazado o 0 si shift es inválido.
 */
uint32_t apply_shift(uint32_t imm, uint32_t shift) {
    if (shift == 0x1) {
        return imm << 12;
    } else if (shift == 0x0) {
        return imm;
    } else {
        printf("Error: Valor inesperado en shift\n");
        return 0;
    }
}


/**
 * Extiende el signo de un valor entero con signo.
 * Convierte un valor de `bits` bits en su equivalente de 32 bits.
 *
 * Params: value (int32_t): Valor a extender.
 *         bits (int): Número de bits que ocupa el valor original (sin signo extendido).
 *
 * Returns: int32_t: Valor extendido a 32 bits con signo correcto.
 */
int32_t sign_extend(int32_t value, int bits) {
    int32_t mask = 1 << (bits - 1);
    return (value ^ mask) - mask;
}


/**
 * Decodifica, ejecuta y almacena el resultado de ADDS extendida.  
 * Suma los registros fuente, guarda el resultado y actualiza flags.  
 *
 * Params: instruction (uint32_t) - Instrucción codificada en 32 bits.
 */
void decode_adds_extended(uint32_t instruction) {
    uint32_t rd = (instruction >> 0) & 0b11111;
    uint32_t rn = (instruction >> 5) & 0b11111;
    uint32_t rm = (instruction >> 16) & 0b11111;
    uint32_t imm3 = (instruction >> 10) & 0b111;
    uint32_t option = (instruction >> 13) & 0b111;

    uint64_t result = CURRENT_STATE.REGS[rn] + CURRENT_STATE.REGS[rm];
    update_result_and_flags(result, rd);
}


/** 
 * Decodifica, ejecuta y almacena el resultado de SUBS extendida.  
 * Resta los registros fuente, guarda el resultado y actualiza flags.  
 *
 * Params: instruction (uint32_t) - Instrucción codificada en 32 bits.
 */
void decode_subs_extended(uint32_t instruction) {
    uint32_t rd = (instruction >> 0) & 0b11111;
    uint32_t rn = (instruction >> 5) & 0b11111;
    uint32_t rm = (instruction >> 16) & 0b11111;
    uint32_t imm3 = (instruction >> 10) & 0b111;
    uint32_t option = (instruction >> 13) & 0b111;

    uint64_t result = CURRENT_STATE.REGS[rn] - CURRENT_STATE.REGS[rm];
    update_result_and_flags(result, rd);
}


/** 
 * Decodifica, ejecuta y almacena el resultado de ADDS inmediata.  
 * Suma un registro fuente con un inmediato, guarda el resultado y actualiza flags.  
 *
 * Params: instruction (uint32_t) - Instrucción codificada en 32 bits.
 */
void decode_adds_immediate(uint32_t instruction){
    uint32_t rd = (instruction >> 0) & 0b11111;
    uint32_t rn = (instruction >> 5) & 0b11111;
    uint32_t imm12 = (instruction >> 10) & 0b111111111111;

    uint32_t shift = (instruction >> 22) & 0b11;
    imm12 = apply_shift(imm12, shift);
    if (shift != 0x0 && shift != 0x1) return;

    uint64_t result = CURRENT_STATE.REGS[rn] + imm12;
    update_result_and_flags(result, rd);
}


/** 
 * Decodifica, ejecuta y almacena el resultado de SUBS inmediata.  
 * Resta un registro fuente con un inmediato, guarda el resultado y actualiza flags.  
 *
 * Params: instruction (uint32_t) - Instrucción codificada en 32 bits.
 */
void decode_subs_immediate(uint32_t instruction){
    uint32_t rd = (instruction >> 0) & 0b11111;
    uint32_t rn = (instruction >> 5) & 0b11111;
    uint32_t imm12 = (instruction >> 10) & 0b111111111111;

    uint32_t shift = (instruction >> 22) & 0b11;
    imm12 = apply_shift(imm12, shift);
    if (shift != 0x0 && shift != 0x1) return;
    
    uint64_t result = CURRENT_STATE.REGS[rn] - imm12;
    update_result_and_flags(result, rd);
}


/** 
 * Decodifica y ejecuta la instrucción HALT.  
 * Detiene la ejecución del programa estableciendo RUN_BIT en 0.  
 *
 * Params: instruction (uint32_t) - Instrucción codificada en 32 bits.
 */
void decode_halt(uint32_t instruction){
    RUN_BIT = 0;
}


/** 
 * Decodifica, ejecuta y actualiza los flags de CMP inmediata.  
 * Compara un registro con un inmediato y actualiza los flags sin almacenar el resultado.  
 *
 * Params: instruction (uint32_t) - Instrucción codificada en 32 bits.
 */
void decode_cmp_immediate(uint32_t instruction){
    uint32_t rn = (instruction >> 5) & 0b11111;
    uint32_t imm12 = (instruction >> 10) & 0b111111111111;

    uint32_t shift = (instruction >> 22) & 0b11;
    imm12 = apply_shift(imm12, shift);
    if (shift != 0x0 && shift != 0x1) return;
    
    uint64_t result = CURRENT_STATE.REGS[rn] - imm12;
    update_result_and_flags(result, -1);
}


/** 
 * Decodifica, ejecuta y actualiza los flags de CMP extendida.  
 * Compara dos registros y actualiza los flags sin almacenar el resultado.  
 *
 * Params: instruction (uint32_t) - Instrucción codificada en 32 bits.
 */
void decode_cmp_extended(uint32_t instruction){
    uint32_t rn = (instruction >> 5) & 0b11111;
    uint32_t rm = (instruction >> 16) & 0b11111;
    uint32_t imm3 = (instruction >> 10) & 0b111;
    uint32_t option = (instruction >> 13) & 0b111;

    uint64_t result = CURRENT_STATE.REGS[rn] - CURRENT_STATE.REGS[rm];
    update_result_and_flags(result, -1);
}


/** 
 * Decodifica, ejecuta y almacena el resultado de ANDS.  
 * Aplica una operación AND bit a bit entre dos registros,  
 * guarda el resultado y actualiza los flags.  
 *
 * Params: instruction (uint32_t) - Instrucción codificada en 32 bits.
 */
void decode_ands(uint32_t instruction){
    uint32_t rd = (instruction >> 0) & 0b11111;
    uint32_t rn = (instruction >> 5) & 0b11111;
    uint32_t rm = (instruction >> 16) & 0b11111;
    uint32_t imm6 = (instruction >> 10) & 0b111111;

    uint64_t result = CURRENT_STATE.REGS[rn] & CURRENT_STATE.REGS[rm];
    update_result_and_flags(result, rd);
}


/** 
 * Decodifica, ejecuta y almacena el resultado de EOR.  
 * Aplica una operación XOR bit a bit entre dos registros,  
 * guarda el resultado y actualiza los flags.  
 *
 * Params: instruction (uint32_t) - Instrucción codificada en 32 bits.
 */
void decode_eor(uint32_t instruction){
    uint32_t rd = (instruction >> 0) & 0b11111;
    uint32_t rn = (instruction >> 5) & 0b11111;
    uint32_t rm = (instruction >> 16) & 0b11111;
    uint32_t imm6 = (instruction >> 10) & 0b111111;

    uint64_t result = CURRENT_STATE.REGS[rn] ^ CURRENT_STATE.REGS[rm];
    update_result_and_flags(result, rd);
}


/** 
 * Decodifica, ejecuta y almacena el resultado de ORR.  
 * Aplica una operación OR bit a bit entre dos registros,  
 * guarda el resultado y actualiza los flags.  
 *
 * Params: instruction (uint32_t) - Instrucción codificada en 32 bits.
 */
void decode_orr(uint32_t instruction){
    uint32_t rd = (instruction >> 0) & 0b11111;
    uint32_t rn = (instruction >> 5) & 0b11111;
    uint32_t rm = (instruction >> 16) & 0b11111;
    uint32_t imm6 = (instruction >> 10) & 0b111111;

    uint64_t result = CURRENT_STATE.REGS[rn] | CURRENT_STATE.REGS[rm];
    update_result_and_flags(result, rd);
}


/**
 * Decodifica y ejecuta la instrucción B (Branch) en ARM.
 * Realiza un salto incondicional calculando la dirección relativa 
 * basada en el offset de la instrucción.
 *
 * Params: instruction (uint32_t): Instrucción codificada en 32 bits.
 */
void decode_b(uint32_t instruction) {
    int32_t raw_offset = (instruction & 0x03FFFFFF) << 2;
    int32_t offset = sign_extend(raw_offset, 28);
    NEXT_STATE.PC = CURRENT_STATE.PC + offset;
}


/**
 * Decodifica y ejecuta la instrucción BR (Branch Register) en ARM.
 * Realiza un salto incondicional a la dirección almacenada en el registro especificado.
 *
 * Params: instruction (uint32_t): Instrucción codificada en 32 bits.
 */
void decode_br(uint32_t instruction) {
    uint8_t Rn = (instruction >> 16) & 0x1F;
    NEXT_STATE.PC = CURRENT_STATE.REGS[Rn];
}


/**
 * Decodifica y ejecuta la instrucción B.cond en ARM.
 * Realiza un salto condicional basado en los flags del procesador.
 *
 * Params: instruction (uint32_t): Instrucción codificada en 32 bits.
 */
void decode_b_cond(uint32_t instruction) {
    uint8_t cond = instruction & 0xF;  
    int32_t imm19 = (instruction >> 5) & 0x7FFFF;
    int32_t offset = sign_extend(imm19 << 2, 21);

    int should_branch = 0;

    switch (cond) {
        case 0x0: // BEQ (Branch if Equal)
            should_branch = (CURRENT_STATE.FLAG_Z == 1);
            break;

        case 0x1: // BNE (Branch if Not Equal)
            should_branch = (CURRENT_STATE.FLAG_Z == 0);
            break;

        case 0xA: // BGE (Branch if Greater Than or Equal)
            should_branch = (CURRENT_STATE.FLAG_N == 0);
            break;

        case 0xB: // BLT (Branch if Less Than)
            should_branch = (CURRENT_STATE.FLAG_N == 1);
            break;

        case 0xC: // BGT (Branch if Greater Than)
            should_branch = (CURRENT_STATE.FLAG_Z == 0 && CURRENT_STATE.FLAG_N == 0);
            break;

        case 0xD: // BLE (Branch if Less Than or Equal)
            should_branch = (CURRENT_STATE.FLAG_Z == 1 || CURRENT_STATE.FLAG_N == 1);
            break;

        default:
            printf("Error: Condición no reconocida (cond = 0x%X)\n", cond);
            return;
    }

    if (should_branch) {
        NEXT_STATE.PC = CURRENT_STATE.PC + offset;
    } else {
        NEXT_STATE.PC = CURRENT_STATE.PC + 4;
    }
}


void decode_stur(uint32_t instruction) {
    uint32_t rt = instruction & 0b11111;
    uint32_t rn = (instruction >> 5) & 0b11111;
    int32_t imm9 = (instruction >> 12) & 0b111111111;  // Extraer 9 bits
    
    // Extensión de signo para imm9 (de 9 a 32 bits)
    imm9 = (imm9 & 0x100) ? (imm9 | ~0x1FF) : imm9;
    
    uint64_t address = CURRENT_STATE.REGS[rn] + (int64_t)imm9;

    // Escribir los 32 bits menos significativos del registro
    mem_write_32(address, (uint32_t)(CURRENT_STATE.REGS[rt] & 0xFFFFFFFF));

    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
}

void decode_sturb(uint32_t instruction) {
    int rt = instruction & 0b11111;
    int rn = (instruction & (0b11111 << 5)) >> 5;
    int imm9 = (instruction & (0b111111111 << 12)) >> 12;

    uint64_t address = CURRENT_STATE.REGS[rn] + imm9;
    uint8_t byte_value = (uint8_t)(CURRENT_STATE.REGS[rt] & 0xFF);
    mem_write_32(address, byte_value);

    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
}
void decode_sturh(uint32_t instruction) {
    int val5 = 0b11111;
    int val9 = 0b111111111;

    int rt = instruction & val5;
    int rn = (instruction & (val5 << 5)) >> 5;
    int imm9 = (instruction & (val9 << 12)) >> 12;

    uint64_t address = CURRENT_STATE.REGS[rn] + imm9;
    uint16_t halfword_value = (uint16_t)(CURRENT_STATE.REGS[rt] & 0xFFFF);
    mem_write_32(address, halfword_value);

    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
}

void decode_ldur(uint32_t instruction) {
    int val5 = 0b11111;
    int val9 = 0b111111111;
    int rt = instruction & val5;
    int rn = (instruction & (val5 << 5)) >> 5;
    int imm9 = (instruction & (val9 << 12)) >> 12;

    uint64_t address = CURRENT_STATE.REGS[rn] + imm9;

    uint32_t low = mem_read_32(address);
    uint32_t high = mem_read_32(address + 4);
        
    NEXT_STATE.REGS[rt] = ((uint64_t) high << 32) | low;;
    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
}
void decode_ldurh(uint32_t instruction) {
    int val5 = 0b11111;
    int val9 = 0b111111111;
    int rt = instruction & val5;
    int rn = (instruction & (val5 << 5)) >> 5;
    int imm9 = (instruction & (val9 << 12)) >> 12;

    uint64_t address = CURRENT_STATE.REGS[rn] + imm9;
    uint16_t halfword_value = mem_read_32(address) & 0xFFFF;
    NEXT_STATE.REGS[rt] = (uint64_t)halfword_value;

    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
}
void decode_ldurb(uint32_t instruction) {
    int val5 = 0b11111;
    int val9 = 0b111111111;
    int rt = instruction & val5;
    int rn = (instruction & (val5 << 5)) >> 5;
    int imm9 = (instruction & (val9 << 12)) >> 12;

    uint64_t address = CURRENT_STATE.REGS[rn] + imm9;
    uint8_t byte_value = mem_read_32(address) & 0xFF;
    NEXT_STATE.REGS[rt] = (uint64_t)byte_value;

    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
}


/**
 * Decodifica y ejecuta la instrucción MOVZ en ARM.
 * Carga un valor inmediato de 16 bits en un registro sin desplazamiento.
 * 
 * Params: instruction (uint32_t): Instrucción codificada en 32 bits.
 */
void decode_movz(uint32_t instruction) {
    uint16_t imm16 = (instruction >> 5) & 0xFFFF;
    uint32_t Rd = (instruction >> 0) & 0x1F;

    uint32_t hw = (instruction >> 21) & 0x3;
    if (hw != 0) {
        printf("Error: hw debe ser 0 para esta implementación de MOVZ.\n");
        return;
    }

    uint64_t value = imm16;
    NEXT_STATE.REGS[Rd] = value;
    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
}


/** 
 * Decodifica, ejecuta y almacena el resultado de ADD inmediata.  
 * Suma un registro fuente con un inmediato y almacena el resultado.  
 *
 * Params: instruction (uint32_t) - Instrucción codificada en 32 bits.
 */
void decode_add_immediate(uint32_t instruction) {
    uint32_t rd = (instruction >> 0) & 0b11111;
    uint32_t rn = (instruction >> 5) & 0b11111;
    uint32_t imm12 = (instruction >> 10) & 0b111111111111;
    
    uint32_t shift = (instruction >> 22) & 0b11;
    imm12 = apply_shift(imm12, shift);
    if (shift != 0x0 && shift != 0x1) return;

    uint64_t result = CURRENT_STATE.REGS[rn] + imm12;
    NEXT_STATE.REGS[rd] = result;
    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
}


/** 
 * Decodifica, ejecuta y almacena el resultado de ADD extendida.  
 * Suma los registros fuente y almacena el resultado.  
 *
 * Params: instruction (uint32_t) - Instrucción codificada en 32 bits.
 */
void decode_add_extended_register(uint32_t instruction) {
    uint32_t rd = (instruction >> 0) & 0b11111;
    uint32_t rn = (instruction >> 5) & 0b11111;
    uint32_t rm = (instruction >> 16) & 0b11111;

    uint64_t result = CURRENT_STATE.REGS[rn] + CURRENT_STATE.REGS[rm];
    NEXT_STATE.REGS[rd] = result;
    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
}


void decode_mul(uint32_t instruction) {
    uint32_t rd = (instruction >> 0) & 0b11111;
    uint32_t rn = (instruction >> 5) & 0b11111;
    uint32_t rm = (instruction >> 16) & 0b11111;

    uint64_t result = CURRENT_STATE.REGS[rn] * CURRENT_STATE.REGS[rm];
    NEXT_STATE.REGS[rd] = result;
    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
}


/** 
 * Decodifica y ejecuta una instrucción CBZ (Compare and Branch on Zero).  
 * Si el registro es cero, salta a la dirección calculada.  
 *
 * Params: instruction (uint32_t) - Instrucción codificada en 32 bits.
 */
void decode_cbz(uint32_t instruction) {
    uint32_t rt = (instruction >> 0) & 0b11111;
    int32_t imm19 = (instruction >> 5) & 0x7FFFF;
    int32_t offset = sign_extend(imm19 << 2, 21);

    if (CURRENT_STATE.REGS[rt] == 0) {
        NEXT_STATE.PC = CURRENT_STATE.PC + offset;
    } else {
        NEXT_STATE.PC = CURRENT_STATE.PC + 4;
    }
}


/** 
 * Decodifica y ejecuta una instrucción CBNZ (Compare and Branch on Non-Zero).  
 * Si el registro no es cero, salta a la dirección calculada.  
 *
 * Params: instruction (uint32_t) - Instrucción codificada en 32 bits.
 */
void decode_cbnz(uint32_t instruction) {
    uint32_t rt = (instruction >> 0) & 0b11111;
    int32_t imm19 = (instruction >> 5) & 0x7FFFF;
    int32_t offset = sign_extend(imm19 << 2, 21);

    if (CURRENT_STATE.REGS[rt] != 0) {
        NEXT_STATE.PC = CURRENT_STATE.PC + offset;
    } else {
        NEXT_STATE.PC = CURRENT_STATE.PC + 4;
    }
}

void decode_lsl_lsr_imm(uint32_t instruction) {
    // Extraer campos comunes
    uint32_t Rd = instruction & 0x1F;
    uint32_t Rn = (instruction >> 5) & 0x1F;
    uint32_t immr = (instruction >> 16) & 0x3F;
    uint32_t imms = (instruction >> 10) & 0x3F;
    
    uint64_t source = CURRENT_STATE.REGS[Rn];
    uint64_t result;
    
    // Determinar si es LSL o LSR
    if (imms != 63) {
        // LSL: imms = 63 - shift
        uint32_t shift = 63 - imms;
        result = (shift >= 64) ? 0 : (source << shift);
    } else {
        // LSR: imms siempre es 63
        uint32_t shift = immr;
        result = (shift >= 64) ? 0 : (source >> shift);
    }
    
    NEXT_STATE.REGS[Rd] = result;
    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
}

void decode_instruction(){

    printf("Decoding instruction\n");
    uint32_t instruction = mem_read_32(CURRENT_STATE.PC);
    printf("Instruction: 0x%X\n", instruction);

    // Posibles opcodes con diferentes tamaños según el formato de instrucción
    uint32_t opcode_11 = (instruction >> 21) & 0x7FF;  // 11 bits (R, I, D, IW)
    uint32_t opcode_8  = (instruction >> 24) & 0xFF;   // 8 bits (CB)
    uint32_t opcode_6  = (instruction >> 26) & 0x3F;   // 6 bits (B)
    uint32_t opcode_9 = (instruction >> 23) & 0x1FF;

    printf("Opcodes: 11-bit: 0x%X, 8-bit: 0x%X, 6-bit: 0x%X\n", opcode_11, opcode_8, opcode_6);

    // Buscar el opcode en el conjunto de instrucciones
    for (int i = 0; i < INSTRUCTION_SET_SIZE; i++) {
        if (INSTRUCTION_SET[i].opcode == opcode_11 || 
            INSTRUCTION_SET[i].opcode == opcode_8  || 
            INSTRUCTION_SET[i].opcode == opcode_6  ||
            INSTRUCTION_SET[i].opcode == opcode_9){

            printf("Match found\n");
            void (*decode_function)(uint32_t) = INSTRUCTION_SET[i].function;
            decode_function(instruction);
            NEXT_STATE.REGS[31] = 0;
            return;
        }
    }
    }
