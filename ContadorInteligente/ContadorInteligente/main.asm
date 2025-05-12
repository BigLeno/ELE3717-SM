.include "m328pdef.inc"

; Nomeação de registradores
.def centena = r0
.def dezena = r1
.def unidade = r2
.def regAdc = r3
.def regPot = r4
.def divisor_100 = r5
.def divisor_10 = r6
.def regCounter = r16
.def regMinMaxStep = r17
.def step = r18
.def min = r19
.def max = r20
.def regAuxiliar = r21
.def contador_do_contador = r23
.def contador = r24
.def inc_ou_dec = r25

; registradores disponíveis
; r25, r23, r24, r27


.org 0x000
    rjmp setup             ; Ponto de entrada

; ----------------------------------------------------
; Vetor de Interrupção PCI1 (Pin Change Interrupt 1 - Port C)
; ----------------------------------------------------
.org PCI1addr            ; Endereço do vetor de interrupção PCI1
    rjmp PCINT1_ISR      ; Salta para a rotina de serviço

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

    ; Configura pinos do led rgb
    sbi DDRB, 1 ; LED Azul
    sbi DDRB, 2 ; LED Verde
    sbi DDRB, 3 ; LED Vermelho

    ; Configura pino do potenciometro
    cbi DDRC, 0 ; Potenciômetro

    ldi     r26, 0x60
	sts     ADMUX, r26   ; defina AVcc como referencia e ajuste a esquerda             
	ldi     r26, 0x00
	sts     ADCSRB, r26  ; defina o modo de trigger free running		
	ldi     r26, 0x87    
	sts     ADCSRA, r26  ; habilite o adc e utilize uma pre-escala de 128

    ; Habilita interrupção por mudança no pino PC1 (PCINT9)
    ldi r26, (1 << PCIE1)   ; Habilita PCI1 (Port C) no PCICR
    sts PCICR, r26
    ldi r26, (1 << PCINT9) | (1 << PCINT10) | (1 << PCINT11)  ; Habilita PCINT9, PCINT10, PCINT11 (PC1, PC2, PC3)
    sts PCMSK1, r26

    sei                     ; Habilita interrupções globais

    ldi r26, 0xC7
    mov regAdc, r26         ; Inicializa o registrador ADC

    ldi r26, 100
    mov divisor_100, r26   ; Inicializa o divisor de 100

    ldi r26, 10
    mov divisor_10, r26    ; Inicializa o divisor de 10

    clr r26



    clr r30                 ; Inicializa flag de interrupção (bit 0)
    ldi r31, 0x00
    ldi regMinMaxStep, 0x00 ; Inicializa o registrador de controle
    ldi min, 0x00
    ; ldi max, 0xFF
    ldi max, 0x0A
    ; ldi step, 0x01
    ldi step, 0x03
    ldi inc_ou_dec, 0x00 ; Inicializa o estado do contador (crescente)
    ldi contador_do_contador, 0  ; r23 ← 0
    ldi contador,              0  ; r24 ← 0

    


main_loop:
    sbrc r30, 0            ; Verifica se a flag de interrupção está setada
    rcall incrementador ; Se sim, chama a rotina de mudança de direção
    sbrc r30, 1            ; Se flag de interrupção não estiver setada, continua
    rjmp set_min_max_step
    sbrc r30, 2            ; Verifica se a flag de interrupção está setada
    rcall decrementador ; Se sim, chama a rotina de mudança de direção

    rcall cont_count ; Chama a rotina de inicialização do contador

    mov r31, contador ; Move o valor do contador para o registrador auxiliar
	
    rjmp atualiza_display



cont_count:

    cpi inc_ou_dec, 0x00         ; Verifica se o estado é crescente (0) 
    breq cont_crescente
    cpi inc_ou_dec, 0x01         ; Verifica se o estado é decrescente (1)
    breq cont_decrescente
    breq stop_contador

cont_crescente:
    add contador, step ; Incrementa o contador
    cp contador, max ; Verifica se o contador chegou ao máximo
    brge overflow
    ret

overflow:
    mov contador, max ; Reseta o contador para o máximo
    rjmp stop_contador

cont_decrescente:
    sub contador, step ; Decrementa o contador
    cp contador, min ; Verifica se o contador chegou ao mínimo
    brlt if_lower_than
    ret

if_lower_than:
    mov contador, min

stop_contador:
    nop
    rjmp atualiza_display ; Retorna para o loop principal


set_min_max_step:

    cpi regMinMaxStep, 0x01 ; Modo: Ajustar Min
    breq set_add_min
    cpi regMinMaxStep, 0x02 ; Modo: Ajustar Max
    breq set_add_max
    cpi regMinMaxStep, 0x03 ; Modo: Ajustar Step
    breq set_add_step
    cpi regMinMaxStep, 0x04 ; Modo: Resetar
    breq reset_regMinMaxStep
    

incrementador:
    ldi inc_ou_dec, 0x00
    
    ret

decrementador:
    ldi inc_ou_dec, 0x01
    ret


    
atualiza_display:
    clr r30                  ; Limpa todos os bits de flag
    mov regAuxiliar, r31
    rcall exibe_display

    cpi regMinMaxStep, 0x00 ; Verifica se o modo é de ajuste
    brne set_min_max_step ; Se não for, continua no loop principal

    rjmp main_loop

exibe_default:
    mov regAuxiliar, r31
    rcall exibe_display
    rjmp main_loop

exibe_display:
    rcall break_digits       ; Divide em centenas, dezenas, unidades
    rcall mostrar_digitos    ; Exibe no display
    ret

; ; ----------------------------------------------------
; ; Rotina de verificação de Min, Max e Step
; ; ----------------------------------------------------

reset_regMinMaxStep:
    ldi regMinMaxStep, 0x00 ; Reseta o registrador de controle
    cbi PORTB, 1 ; Desliga o LED azul
    cbi PORTB, 2 ; Desliga o LED verde
    cbi PORTB, 3 ; Desliga o LED vermelho

    mov contador, min

    rjmp atualiza_display

set_add_min:
    cbi PORTB, 2 ; Desliga o LED verde
    cbi PORTB, 1 ; Desliga o LED azul
    sbi PORTB, 3 ; Liga o LED vermelho
    call Radc ; Chama a rotina de leitura do ADC
    mov r31, regPot ; Armazena o valor lido no registrador auxiliar
    mov min, regPot ; Armazena o valor lido no registrador auxiliar

    rjmp atualiza_display

set_add_max:
    cbi PORTB, 3 ; Desliga o LED vermelho
    cbi PORTB, 1 ; Desliga o LED azul
    sbi PORTB, 2 ; Liga o LED verde

    call Radc ; Chama a rotina de leitura do ADC
    mov r31, regPot ; Armazena o valor lido no registrador auxiliar
    mov max, regPot ; Armazena o valor lido no registrador auxiliar

    rjmp atualiza_display

set_add_step:
    cbi PORTB, 2 ; Desliga o LED verde
    cbi PORTB, 3 ; Desliga o LED vermelho
    sbi PORTB, 1 ; Liga o LED azul

    call Radc ; Chama a rotina de leitura do ADC
    lsr regPot ; Divide o valor lido por 2
    lsr regPot ; Divide o valor lido por 2
    lsr regPot ; Divide o valor lido por 2
    lsr regPot ; Divide o valor lido por 2
    mov r31, regPot ; Armazena o valor lido no registrador auxiliar
    mov step, regPot ; Armazena o valor lido no registrador auxiliar

    rjmp atualiza_display

; ----------------------------------------------------
; Rotina de Serviço de Interrupção (ISR) para PCINT1
; ----------------------------------------------------
PCINT1_ISR:
    push r26
    in r26, SREG
    push r26

    ; Verifica PC1 (botão de medir)
    sbis PINC, PC1
    sbr r30, (1 << 0)    ; seta bit 0

    ; Verifica PC2
    sbis PINC, PC2
    call pc2_interrupt

    ; Verifica PC3
    sbis PINC, PC3
    sbr r30, (1 << 2)    ; seta bit 2

    pop r26
    out SREG, r26
    pop r26
    reti
fim_isr:
    pop r26
    out SREG, r26
    pop r26
    reti

pc2_interrupt:
    sbr r30, (1 << 1)    ; seta bit 1
    inc regMinMaxStep ; Incrementa o modo de ajuste
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
centena_loop:
    cp regAuxiliar, divisor_100
    brlo calcula_dezena
    sub regAuxiliar, divisor_100
    inc centena
    rjmp centena_loop

calcula_dezena:
    ; Calcula o dígito das dezenas
dezena_loop:
    cp regAuxiliar, divisor_10
    brlo calcula_unidade
    sub regAuxiliar, divisor_10
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
    ldi XL, 100
    ldi XH, 0
delay_loop:
    dec XH
    cpi XH, 0
    brne delay_loop

    dec XL
    cpi XL, 0
    brne delay_loop

    ret

Radc:   
	sts     ADCSRA,regAdc   ; inicie a conversao 

loop_adc:
	lds     regpot, ADCSRA  ; carregue no registrador regpot o valor de ADCSRA
	sbrs    regpot, ADIF    ; verifique se o processo de conversao finalizou
	rjmp    loop_adc 
	lds     regpot, ADCH    ; carregue o valor (H) do conversor ad no registrador 22
	ret                  ; retorne