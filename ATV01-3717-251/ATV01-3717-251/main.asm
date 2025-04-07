.include "m328pdef.inc"

; ----------------------------------------------------
; Área de Dados - Reservada na SRAM
; ----------------------------------------------------
.dseg
numeros: .byte 10          ; Vetor com 10 posições para os inteiros

; ----------------------------------------------------
; Código Principal - Início
; ----------------------------------------------------
.cseg
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

    ; Armazena os números em memória
    rcall salvar_numeros

    ; Reseta ponteiro e índice
    rcall reset_ponteiro

; ----------------------------------------------------
; Loop Principal
; ----------------------------------------------------
main_loop:
    ld r20, Z+          ; Lê próximo número para r20

	; Suponha que r20 já contém o número a ser dividido
    rcall break_digits
    ; Agora, r22 = centena, r23 = dezena, r24 = unidade

	rcall mostrar_digitos   ; exibe no display


    inc r21             ; Incrementa índice
    cpi r21, 10
    brlo main_loop      ; Continua se ainda não leu todos

    rcall reset_ponteiro
    rjmp main_loop

; ----------------------------------------------------
; Sub-rotina: Armazena 10 números fixos em SRAM
; ----------------------------------------------------
salvar_numeros:
    ldi ZH, high(numeros)
    ldi ZL, low(numeros)

    ldi r16, 123
    st Z+, r16
    ldi r16, 45
    st Z+, r16
    ldi r16, 67
    st Z+, r16
    ldi r16, 89
    st Z+, r16
    ldi r16, 101
    st Z+, r16
    ldi r16, 202
    st Z+, r16
    ldi r16, 150
    st Z+, r16
    ldi r16, 78
    st Z+, r16
    ldi r16, 34
    st Z+, r16
    ldi r16, 255
    st Z+, r16

    ret

; ----------------------------------------------------
; Sub-rotina: Reseta ponteiro Z para início da SRAM
;             e zera índice (r21)
; ----------------------------------------------------
reset_ponteiro:
    ldi ZH, high(numeros)
    ldi ZL, low(numeros)
    ldi r21, 0
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
    ; Inicializa os registradores de saída
    clr r22  ; Centenas
    clr r23  ; Dezenas
    clr r24  ; Unidades

    ; Calcula o dígito das centenas
    ldi r25, 100
centena_loop:
    cp r20, r25
    brlo calcula_dezena
    sub r20, r25
    inc r22
    rjmp centena_loop

calcula_dezena:
    ; Calcula o dígito das dezenas
    ldi r25, 10
dezena_loop:
    cp r20, r25
    brlo calcula_unidade
    sub r20, r25
    inc r23
    rjmp dezena_loop

calcula_unidade:
    ; O valor restante em r20 é o dígito das unidades
    mov r24, r20

    ret

mostrar_digitos:
	ldi r16, 0

teste:
	inc r16
    ; Mostra CENTENA
    mov r20, r22        ; r22 = centena
    out PORTD, r20      ; valor direto nos bits baixos, assume PORTD4-7 conectados corretamente
    sbi PORTB, 0        ; ativa TJ1 (centena)
	cbi PORTD, 3
	cbi PORTD, 2
    ;rcall espera_curta
    cbi PORTB, 0        ; desliga TJ1

    ; Mostra DEZENA
    mov r20, r23        ; r23 = dezena
    out PORTD, r20
    sbi PORTD, 3        ; ativa TJ2 (dezena)
	cbi PORTB, 0
	cbi PORTD, 2
    ;rcall espera_curta
    cbi PORTD, 3        ; desliga TJ2

    ; Mostra UNIDADE
    mov r20, r24        ; r24 = unidade
    out PORTD, r20
    sbi PORTD, 2        ; ativa TJ3 (unidade)
	cbi PORTB, 0
	cbi PORTD, 3 
   ; rcall espera_curta
    cbi PORTD, 2        ; desliga TJ3
	cpi r16, 14	; branch para teste:
	brlo teste
    ret

espera_curta:
    ldi r18, 100        ; ajustar para ~5ms
loop1:
    ldi r19, 200
loop2:
    nop
    dec r19
    brne loop2
    dec r18
    brne loop1
    ret 