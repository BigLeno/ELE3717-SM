### Parâmetros para configurar os geradores de função (igual ao notebook):

#### Sinal principal (sinal útil)
- **Forma de onda:** Senoidal
- **Frequência:** 1 Hz
- **Amplitude pico-a-pico:** 5 Vpp (para cobrir 0 a 5V do ADC)
- **Offset DC:** +2,5 V (para oscilar de 0 a 5V)

#### Sinal de ruído (senoidal)
- **Forma de onda:** Senoidal
- **Frequência:** 10 Hz
- **Amplitude pico-a-pico:** 0,8 Vpp (corresponde a 80 unidades de pico no ADC, pois 80/1023*5V ≈ 0,39V de pico, ou 0,8Vpp)
- **Offset DC:** 0 V

#### Como conectar:
- Some os dois sinais (soma analógica ou via canal duplo do gerador, se disponível).
- Aplique o sinal somado na entrada analógica do Arduino.

#### Resumo rápido:
- **Sinal 1:** 1 Hz, 5 Vpp, offset +2,5 V
- **Sinal 2:** 10 Hz, 0,8 Vpp, offset 0 V

Assim, você terá exatamente o mesmo cenário do notebook e o filtro FIR irá atuar conforme simulado.
