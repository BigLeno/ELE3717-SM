.include "m328pdef.inc"

; Nomeação de registradores
.def r16 = timerCounter
.def r20 = displayTemp
.def r22 = centena
.def r23 = dezena
.def r24 = unidade
.def r25 = divisor
.def r27 = delayHigh
.def r28 = delayLow

init_system:
    clr centena
    clr dezena
    clr unidade
    

.org 0x000
    rjmp setup             ; Ponto de entrada

; ----------------------------------------------------
; Setup: Configuração inicial dos pinos e memória
; ----------------------------------------------------
setup:
    ; Configura pinos dos transistores (controle dos dígitos)
    sbi DDRB, 0 ; TJ1 - Centena
    sbi DDRD, 3 ; TJ2 - Dezena
    sbi DDRD, 2 ; TJ3 - Unidade

    ; Configura pinos dos segmentos do display
    sbi DDRD, 4 ; HX0
    sbi DDRD, 5 ; HX1
    sbi DDRD, 6 ; HX2
    sbi DDRD, 7 ; HX3

main_loop:

verifica:
    cpi displayTemp, 0
    breq case_0
    cpi displayTemp, 1
    breq case_1
    cpi displayTemp, 2
    breq case_2
    cpi displayTemp, 3
    breq case_3
    cpi displayTemp, 4
    breq case_4
    cpi displayTemp, 5
    breq case_5
    cpi displayTemp, 6
    breq case_6
    cpi displayTemp, 7
    breq case_7
    cpi displayTemp, 8
    breq case_8
    cpi displayTemp, 9
    breq case_9

case_0:
    ldi displayTemp, 0x00
    rjmp fim_case
case_1:
    ldi displayTemp, 0x80
    rjmp fim_case
case_2:
    ldi displayTemp, 0x40
    rjmp fim_case
case_3:
    ldi displayTemp, 0xC0
    rjmp fim_case
case_4:
    ldi displayTemp, 0x20
    rjmp fim_case
case_5:
    ldi displayTemp, 0xA0
    rjmp fim_case
case_6:
    ldi displayTemp, 0x60
    rjmp fim_case
case_7:
    ldi displayTemp, 0xe0
    rjmp fim_case
case_8:
    ldi displayTemp, 0x10
    rjmp fim_case
case_9:
    ldi displayTemp, 0x90
    rjmp fim_case

fim_case:
    ret

; ----------------------------------------------------
; Sub-rotina: Divide r20 por 100, 10 e obtém os dígitos
;             de centena, dezena e unidade.
; Entradas:
;   - r20: Número de 8 bits (0-255)
; Saídas:
;   - r22: Dígito das centenas
;   - r23: Dígito das dezenas
;   - r24: Dígito das unidades
; ----------------------------------------------------
break_digits:
    ; Calcula o dígito das centenas
    ldi divisor, 100
centena_loop:
    cp r20, divisor
    brlo calcula_dezena
    sub r20, divisor
    inc centena
    rjmp centena_loop

calcula_dezena:
    ; Calcula o dígito das dezenas
    ldi divisor, 10
dezena_loop:
    cp r20, divisor
    brlo calcula_unidade
    sub r20, divisor
    inc dezena
    rjmp dezena_loop

calcula_unidade:
    ; O valor restante em r20 é o dígito das unidades
    mov unidade, r20
    ret

mostrar_digitos:
    ldi timerCounter, 0

counter:
    inc timerCounter
    ; Mostra CENTENA
    mov displayTemp, centena        ; centena = centena
    rcall verifica
    out PORTD, displayTemp      ; valor direto nos bits baixos, assume PORTD4-7 conectados corretamente
    sbi PORTB, 0      ; ativa TJ1 (centena)
    cbi PORTD, 3
    cbi PORTD, 2
    rcall espera_curta
    cbi PORTB, 0        ; desliga TJ1

    ; Mostra DEZENA
    mov displayTemp, dezena        ; dezena = dezena
    rcall verifica
    out PORTD, displayTemp
    sbi PORTD, 3        ; ativa TJ2 (dezena)
    cbi PORTB, 0
    cbi PORTD, 2
    rcall espera_curta
    cbi PORTD, 3        ; desliga TJ2

    ; Mostra UNIDADE
    mov displayTemp, unidade        ; unidade = unidade
    rcall verifica
    out PORTD, displayTemp
    sbi PORTD, 2        ; ativa TJ3 (unidade)
    cbi PORTB, 0
    cbi PORTD, 3 
    rcall espera_curta
    cbi PORTD, 2        ; desliga TJ3
    cpi timerCounter, 35    ; branch para counter:
    brlo counter
    ret

espera_curta:
    ldi delayHigh, 100
    ldi delayLow, 0
delay_loop:
    dec delayLow
    cpi delayLow, 0
    brne delay_loop

    dec delayHigh
    cpi delayHigh, 0
    brne delay_loop

    ret
