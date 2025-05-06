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

    ; Configura pinos do HC-SR04
    ; PC4 -> Trigger e PC5 -> Echo
    sbi DDRC, 4    ; PC4 como saída (Trigger)
    cbi DDRC, 5    ; PC5 como entrada (Echo)

    ; Habilita interrupção por mudança no pino PC1 (PCINT9)
    ldi r26, (1 << PCIE1)   ; Habilita PCI1 (Port C) no PCICR
    sts PCICR, r26
    ldi r26, (1 << PCINT9) | (1 << PCINT10) | (1 << PCINT11)  ; Habilita PCINT9, PCINT10, PCINT11 (PC1, PC2, PC3)
    sts PCMSK1, r26

    sei                     ; Habilita interrupções globais

    clr r30                 ; Inicializa flag de interrupção (bit 0)

state_verifica_eeprom:
    ; vai verificar o conteudo da eeprom
    ; se for 0x00, vai carregar o valor padrão  
    ; se for diferente disso, vai carregar o valor da eeprom

    ldi r31, 0x00

; ----------------------------------------------------
; Loop Principal
; ----------------------------------------------------
main_loop:
    sbrs r30, 0              ; Checa se PC1 (medir distância) foi pressionado
    rjmp checa_botao_pc2
    rcall measure_distance    ; Mede distância (resultado em r17)
    mov r31, r17             ; Atualiza regAuxiliar
    rjmp atualiza_display

checa_botao_pc2:
    sbrs r30, 1              ; Checa se PC2 foi pressionado
    rjmp checa_botao_pc3
    ldi r31, 2               ; Mostra 2 no display
    rjmp atualiza_display

checa_botao_pc3:
    sbrs r30, 2              ; Checa se PC3 foi pressionado
    rjmp exibe_default
    ldi r31, 3               ; Mostra 3 no display
    rjmp atualiza_display

atualiza_display:
    clr r30                  ; Limpa todos os bits de flag
    mov regAuxiliar, r31
    rcall exibe_display
    rjmp main_loop

exibe_default:
    mov regAuxiliar, r31
    rcall exibe_display
    rjmp main_loop

exibe_display:
    rcall break_digits       ; Divide em centenas, dezenas, unidades
    rcall mostrar_digitos    ; Exibe no display
    ret

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
    sbr r30, (1 << 1)    ; seta bit 1

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
    clr r17
    rcall divide_16by8
    ret

divide_16by8:
    cpi XH, 0x00
    breq verifica_xl
    rcall divide_loop

verifica_xl:
    cpi XL, 58
    brlo divide_done

divide_loop:
    sbiw XH:XL, 58
    inc r17
    rcall divide_16by8  

divide_done:
    ; O resultado da divisão está em r17 (distância em cm)
    ; O valor de X é o r26o em μs
    ;mov regAuxiliar, r17 ; Armazena o resultado em regAuxiliar
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

; ----------------------------------------------------
; Sub-rotina: Escrita na EEPROM
; Entradas:
;   - r30 = endereço
;   - r31 = dado a ser salvo
; ----------------------------------------------------
eeprom_write:
    sbic EECR, EEPE          ; Espera se EEPROM estiver ocupada
    rjmp eeprom_write
    out EEARL, r30           ; Define endereço
    out EEDR, r31            ; Define dado
    sbi EECR, EEMPE          ; Habilita gravação
    sbi EECR, EEPE           ; Inicia gravação
    ret

; ----------------------------------------------------
; Sub-rotina: Leitura da EEPROM
; Entradas:
;   - r30 = endereço
; Saídas:
;   - r31 = valor lido
; ----------------------------------------------------
eeprom_read:
    sbic EECR, EEPE          ; Espera se EEPROM estiver ocupada
    rjmp eeprom_read
    out EEARL, r30
    sbi EECR, EERE           ; Dispara leitura
    in r31, EEDR             ; Lê valor
    ret