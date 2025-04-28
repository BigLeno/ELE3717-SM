.include "m328pdef.inc"

.def regCounter = r16
.def regAuxiliar = r20
.def centena = r22
.def dezena = r23
.def unidade = r24
.def divisor = r25

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
    sbi DDRC, 4    ; PC4 como saída (Trigger)
    cbi DDRC, 5    ; PC5 como entrada (Echo)

state_verifica_eeprom:
    ; vai verificar o conteudo da eeprom
    ; se for 0x00, vai carregar o valor padrão  
    ; se for diferente disso, vai carregar o valor da eeprom

    ldi r31, 0x00

; ----------------------------------------------------
; Loop Principal
; ----------------------------------------------------
main_loop:
    rcall measure_distance    ; Mede a distância (resultado em r16)
    mov regAuxiliar, r31
    sbis PINC, 1
    rcall s1_pressionado     ; Verifica se o botão foi pressionado 
    rcall exibe_display      ; Exibe no display
    rjmp main_loop           ; Repete

s1_pressionado:
    mov r31, r16     ; Copia o valor para regAuxiliar
    ret

exibe_display:
    rcall break_digits       ; Divide em centenas, dezenas, unidades
    rcall mostrar_digitos    ; Exibe no display
    ret

; ----------------------------------------------------
; Sub-rotina: Gera pulso de Trigger para o HC-SR04
; Entradas:
;   - Nenhuma
; Saídas:
;   - Gera pulso de Trigger no pino PC4
; ----------------------------------------------------
trigger_pulse:
    cbi PORTC, 4     ; Trigger LOW
    rcall delay_2us  ; Delay de 2 μs
    sbi PORTC, 4     ; Trigger HIGH
    rcall delay_10us ; Delay de 10 μs
    cbi PORTC, 4     ; Trigger LOW
    ret

; ----------------------------------------------------
; Sub-rotina: Mede a distância usando o HC-SR04
; Entradas:
;   - Nenhuma
; Saídas:
;   - r16: Duração do pulso em μs
;   - r17: Distância em cm (opcional)
; ----------------------------------------------------
measure_distance:
    rcall trigger_pulse
    ldi r26, HIGH(40000)  ; Timeout ~7m
    ldi r27, LOW(40000)
    
    ; Espera ativa com verificação de timeout
echo_wait:
    sbic PINC, 5
    rjmp pulse_start
    sbiw X, 1
    brne echo_wait
    ret                 ; Retorna 0 se timeout

pulse_start:
    clr r26
    clr r27
pulse_measure:
    adiw X, 1
    sbis PINC, 5
    rjmp pulse_end
    cpi r27, LOW(40000)
    brlo pulse_measure

pulse_end:
    ; Divisão rápida por 58 usando multiplicação
    movw r16, X
    ldi r18, 58
    rcall divide_16by8
    ret

divide_16by8:
    clr r19
    ldi r20, 16
div_loop:
    lsl r16
    rol r17
    rol r19
    cp r19, r18
    brlo div_next
    sub r19, r18
    inc r16
div_next:
    dec r20
    brne div_loop
    ret

delay_2us:
    nop
    nop
    ret

delay_10us:
    ldi r19, 4
delay_loop:
    dec r19
    brne delay_loop
    ret

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

; ----------------------------------------------------
; Sub-rotina: Verifica o valor de regAuxiliar e
;             carrega o valor correspondente em regAuxiliar
; Entradas:
;   - regAuxiliar: Número de 0 a 9
; Saídas:
;   - regAuxiliar: Valor correspondente para o display
; ----------------------------------------------------
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
    ldi r21, 100   ; Usando registradores não críticos
    ldi r20, 0
delay_loop_exibir:
    dec r20
    brne delay_loop_exibir
    dec r21
    brne delay_loop_exibir
    ret