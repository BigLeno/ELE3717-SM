.include "m328pdef.inc"

.def regCounter = r16
.def regAuxiliar = r20
.def centena = r22
.def dezena = r23
.def unidade = r24
.def divisor = r25
.def regCounterSecond = r27

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

    ; Configura pinos do botão
    cbi DDRC, 1;
    cbi DDRC, 2;
    cbi DDRC, 3;

    ; Configura pinos do HC-SR04
    ; PC4 -> Trigger e PC5 -> Echo
    sbi DDRC, 4;
    sbi DDRC, 5;


; ----------------------------------------------------
; Loop Principal
; ----------------------------------------------------
main_loop:
    
    rjmp main_loop        ; Loop infinito para manter exibição

; ----------------------------------------------------
; Verificação de casos de dígitos, inversão de bits
; ----------------------------------------------------
verifica:
	cpi regAuxiliar, 0
	breq case_0
	cpi regAuxiliar, 1
	breq case_1
	cpi regAuxiliar, 2
	breq case_2
	cpi regAuxiliar, 3
	breq case_3
	cpi regAuxiliar, 4
	breq case_4
	cpi regAuxiliar, 5
	breq case_5
	cpi regAuxiliar, 6
	breq case_6
	cpi regAuxiliar, 7
	breq case_7
	cpi regAuxiliar, 8
	breq case_8
	cpi regAuxiliar, 9
	breq case_9

case_0:
	ldi regAuxiliar, 0x00
	rjmp fim_case
case_1:
	ldi regAuxiliar, 0x80
	rjmp fim_case
case_2:
	ldi regAuxiliar, 0x40
	rjmp fim_case
case_3:
	ldi regAuxiliar, 0xC0
	rjmp fim_case
case_4:
	ldi regAuxiliar, 0x20
	rjmp fim_case
case_5:
	ldi regAuxiliar, 0xA0
	rjmp fim_case
case_6:
	ldi regAuxiliar, 0x60
	rjmp fim_case
case_7:
	ldi regAuxiliar, 0xe0
	rjmp fim_case
case_8:
	ldi regAuxiliar, 0x10
	rjmp fim_case
case_9:
	ldi regAuxiliar, 0x90
	rjmp fim_case

fim_case:
	ret

; ----------------------------------------------------
; Sub-rotina: Divide regAuxiliar por 100, 10 e obtém os dígitos
;             de centena, dezena e unidade.
; Entradas:
;   - regAuxiliar: Número de 8 bits (0-255)
; Saídas:
;   - centena: Dígito das centenas
;   - dezena: Dígito das dezenas
;   - unidade: Dígito das unidades
; ----------------------------------------------------
break_digits:
    ; Inicializa os registradores de saída
    clr centena  ; Centenas
    clr dezena  ; Dezenas
    clr unidade  ; Unidades

    ; Calcula o dígito das centenas
    ldi divisor, 100
centena_loop:
    cp regAuxiliar, divisor
    brlo calcula_dezena
    sub regAuxiliar, divisor
    inc centena
    rjmp centena_loop

calcula_dezena:
    ; Calcula o dígito das dezenas
    ldi divisor, 10
dezena_loop:
    cp regAuxiliar, divisor
    brlo calcula_unidade
    sub regAuxiliar, divisor
    inc dezena
    rjmp dezena_loop

calcula_unidade:
    ; O valor restante em regAuxiliar é o dígito das unidades
    mov unidade, regAuxiliar
    ret

mostrar_digitos:
	ldi regCounter, 0

counter:
	inc regCounter
    ; Mostra CENTENA
    mov regAuxiliar, centena        
	rcall verifica
    out PORTD, regAuxiliar      ; valor direto nos bits baixos, assume PORTD4-7 conectados corretamente
    sbi PORTB, 0      ; ativa TJ1 (centena)
	cbi PORTD, 3
	cbi PORTD, 2
    rcall espera_curta
    cbi PORTB, 0        ; desliga TJ1

    ; Mostra DEZENA
    mov regAuxiliar, dezena        
	rcall verifica
    out PORTD, regAuxiliar
    sbi PORTD, 3        ; ativa TJ2 (dezena)
	cbi PORTB, 0
	cbi PORTD, 2
    rcall espera_curta
    cbi PORTD, 3        ; desliga TJ2

    ; Mostra UNIDADE
    mov regAuxiliar, unidade        
	rcall verifica
    out PORTD, regAuxiliar
    sbi PORTD, 2        ; ativa TJ3 (unidade)
	cbi PORTB, 0
	cbi PORTD, 3 
    rcall espera_curta
    cbi PORTD, 2        ; desliga TJ3
	cpi regCounter, 35	; branch para counter:
	brlo counter
    ret

espera_curta:
	ldi regCounterSecond, 100
	ldi r28, 0
delay_loop:
	dec r28
	cpi r28, 0
	brne delay_loop

	dec regCounterSecond
	cpi regCounterSecond, 0
	brne delay_loop

	ret