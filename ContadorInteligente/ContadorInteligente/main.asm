.include "m328pdef.inc"

.def r16 = timerCounter
.def r20 = displayTemp
.def r22 = centena
.def r23 = dezena
.def r24 = unidade


.org 0x000
    rjmp setup             ; Ponto de entrada

init_digits:
    clr centena
    clr dezena
    clr unidade

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
    mov displayTemp, r24        ; r24 = unidade
	rcall verifica
    out PORTD, displayTemp
    sbi PORTD, 2        ; ativa TJ3 (unidade)
	cbi PORTB, 0
	cbi PORTD, 3 
    rcall espera_curta
    cbi PORTD, 2        ; desliga TJ3
	cpi timerCounter, 35	; branch para counter:
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
