PROJETO 06 - FILTRO DIGITAL FIR COM INTERFACE HOMEM-MÁQUINA

O Projeto 06 propõe a criação de um filtro digital do tipo FIR (Finite Impulse Response), com ajuste dinâmico de coeficientes em tempo real, por meio de uma Interface Homem-Máquina (IHM). O sistema é baseado no microcontrolador AVR ATMega328P, com firmware desenvolvido em linguagem ANSI C, e tem como objetivo oferecer ao usuário a possibilidade de modificar os parâmetros do filtro de forma prática e intuitiva.

O filtro digital implementado utiliza 16 taps para processamento de sinal, empregando coeficientes em ponto flutuante para garantir maior precisão nos cálculos. O sistema inclui um filtro passa-baixa pré-configurado com frequência de corte de 2Hz e frequência de amostragem de 100Hz, implementando uma linha de atraso circular para otimização do uso de memória. A normalização automática converte sinais de entrada de 10 bits para saída de 8 bits, garantindo compatibilidade com os conversores analógico-digitais e digital-analógicos utilizados.

A Interface Homem-Máquina é composta por um display LCD 16x2 para visualização dos parâmetros do sistema e três botões de controle. O botão S1 permite navegar para o coeficiente anterior, o botão S2 avança para o próximo coeficiente, e o botão S3 confirma a seleção e permite entrar no modo de configuração. O controle da interface é realizado através de uma máquina de estados que alterna entre o estado inicial (STATE_INITIAL), que exibe a tela principal do sistema, e o estado de coeficientes (STATE_COEFFICIENTS), que permite a visualização e navegação pelos dezesseis coeficientes do filtro.

O processamento de sinal é realizado em tempo real utilizando um conversor analógico-digital de 10 bits conectado ao canal A0 para entrada do sinal analógico. A saída do sinal filtrado é fornecida através de um conversor digital-analógico R2R de 8 bits. O sistema implementa proteção contra overflow e underflow na saída, garantindo a estabilidade do processamento mesmo com sinais de entrada que excedam os limites esperados.

Para persistência de dados, o sistema utiliza a EEPROM interna do microcontrolador para armazenamento dos coeficientes personalizados definidos pelo usuário. Os coeficientes padrão são carregados automaticamente na inicialização do sistema, e uma função de restauração permite retornar aos valores de fábrica quando necessário.

As ferramentas de desenvolvimento incluem um Jupyter Notebook localizado em tests/fir.ipynb, que permite o cálculo e simulação dos coeficientes FIR, teste de resposta em frequência, validação do filtro com sinais de teste sintéticos, e conversão dos coeficientes para o formato adequado ao microcontrolador.

Do ponto de vista de hardware, o sistema utiliza o microcontrolador ATMega328P operando a 16MHz, um display LCD 16x2 com protocolo de comunicação paralelo, entrada analógica através de ADC de 10 bits com faixa de 0 a 1023, saída através de DAC R2R de 8 bits com faixa de 0 a 255, três botões de controle com pull-up interno, e armazenamento não-volátil através da EEPROM interna de 1KB.
- Controles: 3 botões com pull-up interno
- Armazenamento: EEPROM interna (1KB)

Software:
- Linguagem: ANSI C
- Compilador: AVR-GCC
- Build System: Makefile
- Bibliotecas: AVR LibC
- Interrupções: PCINT para botões

Filtro Digital:
- Tipo: FIR (Finite Impulse Response)
- Ordem: 15 (16 taps)
- Precisão: Ponto flutuante (32 bits)
- Frequência de amostragem: 100 Hz
- Frequência de corte padrão: 2 Hz
- Atenuação: > 40dB na banda de rejeição

ESTRUTURA DO PROJETO

atv06_3717_251/
├── Makefile              (Build system)
├── README.md            (Este arquivo)
├── atv06_3717_251.pdf   (Documentação do projeto)
├── build/               (Arquivos compilados)
├── include/             (Headers)
│   ├── adc.h           (Interface ADC)
│   ├── btn.h           (Controle de botões)
│   ├── eeprom.h        (Acesso à EEPROM)
│   ├── fir.h           (Filtro FIR)
│   ├── lcd.h           (Display LCD)
│   └── mde.h           (Máquina de estados)
├── src/                (Código fonte)
│   ├── adc.c           (Implementação ADC)
│   ├── btn.c           (Implementação botões)
│   ├── eeprom.c        (Implementação EEPROM)
│   ├── fir.c           (Implementação filtro FIR)
│   ├── lcd.c           (Implementação LCD)
│   ├── main.c          (Programa principal)
│   └── mde.c           (Implementação máquina de estados)
└── tests/              (Ferramentas de teste)
    ├── fir.ipynb       (Notebook para cálculo de coeficientes)
    └── requirements.txt (Dependências Python)

USO DO SISTEMA

Inicialização:
1. O sistema inicia na tela principal mostrando "ELE-3717" e "FILTRO FIR 2Hz"
2. O filtro carrega automaticamente os coeficientes padrão
3. O processamento de sinal inicia imediatamente

Configuração de Coeficientes:
1. Pressione S3 na tela inicial para entrar no modo de configuração
2. Use S1 e S2 para navegar entre os 16 coeficientes (C00 a C15)
3. Visualize cada coeficiente com 3 casas decimais (ex: "0.121")
4. Pressione S3 novamente para avançar ou sair após o último coeficiente

Processamento de Sinal:
- Conecte o sinal de entrada no canal A0 do ADC
- O sinal filtrado estará disponível na saída DAC R2R
- O processamento ocorre continuamente em tempo real

## Compilação e Upload

```bash
# Compilar o projeto
make

# Limpar arquivos compilados
make clean

# Upload para o microcontrolador (ajuste a porta)
make upload
```

## Desenvolvimento e Teste

### Ambiente Python (Jupyter Notebook)

```bash
# Instalar dependências
pip install -r tests/requirements.txt

# Executar notebook
jupyter notebook tests/fir.ipynb
```

### Cálculo de Novos Coeficientes

O notebook `fir.ipynb` permite:

- Calcular coeficientes para diferentes frequências de corte
- Simular a resposta do filtro
- Gerar coeficientes otimizados para o microcontrolador
- Testar com sinais sintéticos

## Pinagem

### LCD (Modo Paralelo)
- **RS**: PD2
- **E**: PD3
- **D4-D7**: PD4-PD7

### Botões
- **S1**: PC1 (PCINT9)
- **S2**: PC2 (PCINT10)
- **S3**: PC3 (PCINT11)

### ADC
- **Entrada**: PC0 (A0)

### DAC R2R (8 bits)
- **Bits 0-1**: PC4, PC5
- **Bits 2-7**: PB0-PB5

## Características do Filtro Padrão

- **Tipo**: Passa-baixa
- **Frequência de amostragem**: 100 Hz
- **Frequência de corte**: 2 Hz
- **Ondulação na banda passante**: < 0.1 dB
- **Atenuação mínima**: 40 dB @ 10 Hz
- **Resposta ao impulso**: Simétrica (Fase linear)

## Autor

Desenvolvido para a disciplina ELE-3717 - Sistemas Microprocessados

## Data

Julho de 2025
