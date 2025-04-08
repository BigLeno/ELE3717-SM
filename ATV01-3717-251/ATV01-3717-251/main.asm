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
    ld r20, Z+            ; Lê próximo número da SRAM
    rcall break_digits    ; Calcula r22 (centena), r23 (dezena), r24 (unidade)
    rcall mostrar_digitos ; Exibe no display

    inc r21               ; Incrementa índice
    cpi r21, 10
    brlo main_loop        ; Continua se ainda não leu todos
	rcall reset_ponteiro
    rjmp main_loop        ; Loop infinito para manter exibição

verifica:
	cpi r20, 0
	breq case_0
	cpi r20, 1
	breq case_1
	cpi r20, 2
	breq case_2
	cpi r20, 3
	breq case_3
	cpi r20, 4
	breq case_4
	cpi r20, 5
	breq case_5
	cpi r20, 6
	breq case_6
	cpi r20, 7
	breq case_7
	cpi r20, 8
	breq case_8
	cpi r20, 9
	breq case_9


case_0:
	ldi r20, 0x00
	rjmp fim_case
case_1:
	ldi r20, 0x80
	rjmp fim_case
case_2:
	ldi r20, 0x40
	rjmp fim_case
case_3:
	ldi r20, 0xC0
	rjmp fim_case
case_4:
	ldi r20, 0x20
	rjmp fim_case
case_5:
	ldi r20, 0xA0
	rjmp fim_case
case_6:
	ldi r20, 0x60
	rjmp fim_case
case_7:
	ldi r20, 0xe0
	rjmp fim_case
case_8:
	ldi r20, 0x10
	rjmp fim_case
case_9:
	ldi r20, 0x90
	rjmp fim_case

fim_case:
	ret
	
; ----------------------------------------------------
; Sub-rotina: Armazena 10 números fixos em SRAM
; ----------------------------------------------------
salvar_numeros:
    ldi ZH, high(numeros)
    ldi ZL, low(numeros)

    ldi r16, 1
    st Z+, r16
    ldi r16, 10
    st Z+, r16
    ldi r16, 100
    st Z+, r16
    ldi r16, 89
    st Z+, r16
    ldi r16, 101
    st Z+, r16
    ldi r16, 3
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

counter:
	inc r16
    ; Mostra CENTENA
    mov r20, r22        ; r22 = centena
	rcall verifica
	cpi r20, 0
	brne f_port_centena
	ldi r20, 0xF0
f_port_centena:
    out PORTD, r20      ; valor direto nos bits baixos, assume PORTD4-7 conectados corretamente
    sbi PORTB, 0      ; ativa TJ1 (centena)
	cbi PORTD, 3
	cbi PORTD, 2
    rcall espera_curta
    cbi PORTB, 0        ; desliga TJ1

    ; Mostra DEZENA
    mov r20, r23        ; r23 = dezena
	rcall verifica
	cpi r20, 0x00
	brne f_port_dezena
	cpi r22, 0x00
	brne f_port_dezena
	ldi r20, 0xF0
f_port_dezena:
    out PORTD, r20
    sbi PORTD, 3        ; ativa TJ2 (dezena)
	cbi PORTB, 0
	cbi PORTD, 2
    rcall espera_curta
    cbi PORTD, 3        ; desliga TJ2

    ; Mostra UNIDADE
    mov r20, r24        ; r24 = unidade
	rcall verifica
    out PORTD, r20
    sbi PORTD, 2        ; ativa TJ3 (unidade)
	cbi PORTB, 0
	cbi PORTD, 3 
    rcall espera_curta
    cbi PORTD, 2        ; desliga TJ3
	cpi r16, 35	; branch para counter:
	brlo counter
    ret

espera_curta:
	ldi r27, 100
	ldi r28, 0
delay_loop:
	dec r28
	cpi r28, 0
	brne delay_loop

	dec r27
	cpi r27, 0
	brne delay_loop

	ret