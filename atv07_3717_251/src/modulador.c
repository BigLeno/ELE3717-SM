#include <avr/io.h>
#include <util/delay.h>
#include "modulador.h"
#include <stdint.h> // Adicionado para tipos de dados explícitos

// --- MELHORIA: Constantes para "números mágicos" ---
#define SAMPLES_PER_CYCLE 32
#define PHASE_ACC_SHIFT 11 // (16 - log2(32)) = 11, para usar os 5 bits mais significativos do acumulador
#define SAMPLING_FREQ 20000UL // Frequência de amostragem em Hz (exemplo)
#define PHASE_ACC_MAX 65536UL // Valor máximo de um acumulador de 16 bits

const uint8_t COS_TABLE[SAMPLES_PER_CYCLE] = {
    255, 253, 246, 234, 218, 199, 177, 153,
    128, 103,  79,  57,  38,  22,  10,   3,
      0,   3,  10,  22,  38,  57,  79, 103,
    128, 153, 177, 199, 218, 234, 246, 253
};

// CORRIGIDO: A função agora usa 'msg' para modular a amplitude.
// Assumindo que 'msg' é um valor de 8 bits (0-255).
uint8_t modula_am(uint16_t freq_portadora, uint8_t msg)
{
    static uint16_t contador_portadora = 0;
    
    // Calcula o incremento de fase para a frequência da portadora desejada
    uint16_t step_portadora = (uint32_t)freq_portadora * PHASE_ACC_MAX / SAMPLING_FREQ;
    
    // Gera a portadora
    uint8_t idx_portadora = (contador_portadora >> PHASE_ACC_SHIFT) & (SAMPLES_PER_CYCLE - 1);
    uint8_t valor_portadora = COS_TABLE[idx_portadora];
    
    // Aplica a modulação usando o sinal de entrada 'msg'
    // A modulação varia a amplitude com base no valor de 'msg'
    // msg = 0 -> ~50% amplitude; msg = 255 -> 100% amplitude
    uint16_t fator_modulacao = 128 + (msg >> 1); // Fator vai de 128 a 255
    
    // Aplica modulação e normaliza o resultado para 8 bits
    uint16_t resultado = ((uint16_t)valor_portadora * fator_modulacao) >> 8;
    
    // Atualiza o acumulador de fase da portadora
    contador_portadora += step_portadora;
    
    return (uint8_t)resultado;
}

// CORRIGIDO: A função agora usa 'msg' para modular a frequência.
// Assumindo que 'msg' é um valor de 8 bits (0-255).
uint8_t modula_fm(uint16_t freq_portadora, uint8_t msg)
{
    static uint16_t contador_fase = 0;
    
    // Converte 'msg' (0-255) para um desvio de frequência assinado.
    // Ex: -32 a +31, resultando em um desvio de frequência.
    int8_t desvio = ((int16_t)msg - 128) >> 2; 
    
    // Frequência instantânea = Frequência Central + (fator de desvio * sinal modulante)
    uint16_t freq_instantanea = freq_portadora + (desvio * 2); // Ex: desvio de +/- 64 Hz
    
    // Calcula o incremento de fase para a frequência instantânea
    uint16_t step = (uint32_t)freq_instantanea * PHASE_ACC_MAX / SAMPLING_FREQ;
    
    // Gera o sinal FM
    uint8_t indice = (contador_fase >> PHASE_ACC_SHIFT) & (SAMPLES_PER_CYCLE - 1);
    uint8_t resultado = COS_TABLE[indice];
    
    // Atualiza o acumulador de fase
    contador_fase += step;
    
    return resultado;
}

// CORRIGIDO: O parâmetro foi renomeado e a função usa 'msg' como o byte de dados a ser transmitido.
uint8_t modula_ask(uint16_t taxa_bits, uint8_t msg)
{
    static uint16_t contador_portadora = 0;
    static uint16_t contador_bits_tempo = 0;
    
    // Período de cada bit em número de amostras
    uint16_t bit_period = SAMPLING_FREQ / taxa_bits;
    if (bit_period == 0) bit_period = 1; // Evita divisão por zero
    
    // Determina o índice do bit a ser transmitido (0 a 7)
    uint8_t bit_index = (contador_bits_tempo / bit_period) % 8;
    
    // Pega o bit atual do byte de dados 'msg'
    uint8_t bit_atual = (msg >> bit_index) & 0x01;
    
    // Gera a portadora em uma frequência fixa (ex: 2.5 kHz)
    uint16_t step_portadora = (uint32_t)2500 * PHASE_ACC_MAX / SAMPLING_FREQ;
    uint8_t indice = (contador_portadora >> PHASE_ACC_SHIFT) & (SAMPLES_PER_CYCLE - 1);
    uint8_t portadora = COS_TABLE[indice];
    
    // ASK: Se o bit for 1, transmite a portadora. Se for 0, transmite uma amplitude baixa.
    uint8_t resultado = bit_atual ? portadora : 0; 
    
    // Atualiza contadores
    contador_portadora += step_portadora;
    contador_bits_tempo++;
    if(contador_bits_tempo >= (bit_period * 8)) {
        contador_bits_tempo = 0; // Reinicia após transmitir os 8 bits
    }
    
    return resultado;
}

// CORRIGIDO: O parâmetro foi renomeado e a função usa 'msg' como o byte de dados.
uint8_t modula_fsk(uint16_t taxa_bits, uint8_t msg)
{
    static uint16_t contador_fase = 0;
    static uint16_t contador_bits_tempo = 0;

    // Frequências para bit 0 e bit 1
    const uint16_t freq_bit0 = 1200;
    const uint16_t freq_bit1 = 2200;
    
    // Período de cada bit em número de amostras
    uint16_t bit_period = SAMPLING_FREQ / taxa_bits;
    if (bit_period == 0) bit_period = 1;

    // Determina o bit atual a ser transmitido
    uint8_t bit_index = (contador_bits_tempo / bit_period) % 8;
    uint8_t bit_atual = (msg >> bit_index) & 0x01;
    
    // Seleciona a frequência com base no bit atual
    uint16_t freq_atual = bit_atual ? freq_bit1 : freq_bit0;
    
    // Calcula o passo de fase para a frequência atual
    uint16_t step = (uint32_t)freq_atual * PHASE_ACC_MAX / SAMPLING_FREQ;
        
    // Gera o sinal na frequência apropriada
    uint8_t indice = (contador_fase >> PHASE_ACC_SHIFT) & (SAMPLES_PER_CYCLE - 1);
    uint8_t resultado = COS_TABLE[indice];
    
    // Atualiza contadores
    contador_fase += step;
    contador_bits_tempo++;
    if(contador_bits_tempo >= (bit_period * 8)) {
        contador_bits_tempo = 0; // Reinicia
    }
    
    return resultado;
}

// --- CORRIGIDO E MELHORADO: Função de envio de dados ---
void envia_dados(uint8_t y)
{
    // Saída para malha R2R de 8 bits usando PC4, PC5, PB0-PB5
    // Bit 7 (MSB) -> PB5
    // ...
    // Bit 2       -> PB0
    // Bit 1       -> PC5
    // Bit 0 (LSB) -> PC4
    
    // Bits 7-2 (os 6 bits mais significativos) são enviados para PORTB pins PB5-PB0.
    // PORTB pode ser sobrescrito diretamente se os pinos PB6 e PB7 não forem usados.
    PORTB = (y >> 2);
    
    // Bits 1-0 (os 2 bits menos significativos) são enviados para PORTC pins PC5 e PC4.
    // Usa operação Read-Modify-Write para preservar os outros pinos de PORTC (ex: PC0).
    // 1. Prepara a máscara para os bits que serão alterados (PC5 e PC4).
    uint8_t mask_c = (1 << PC5) | (1 << PC4);
    
    // 2. Prepara os novos valores para os bits PC5 e PC4.
    uint8_t bits_baixos_mapeados = ((y & 0x01) << PC4) | ((y & 0x02) << (PC5 - 1));

    // 3. Lê o valor atual de PORTC, limpa os bits que vamos alterar, e então aplica os novos valores.
    PORTC = (PORTC & ~mask_c) | bits_baixos_mapeados;
}

// --- NOVO: Implementação da função que faltava ---
uint8_t gera_senoide_pura(uint16_t freq_hz)
{
    static uint16_t contador_fase = 0;
    
    // Calcula o passo de fase para a frequência desejada
    uint16_t step = (uint32_t)freq_hz * PHASE_ACC_MAX / SAMPLING_FREQ;
    
    // Gera o sinal
    uint8_t indice = (contador_fase >> PHASE_ACC_SHIFT) & (SAMPLES_PER_CYCLE - 1);
    uint8_t resultado = COS_TABLE[indice];
    
    // Atualiza o acumulador de fase
    contador_fase += step;
    
    return resultado;
}