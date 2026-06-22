# Aferidor de Temperatura de Forno Industrial (PIC18)

## Sobre o projeto

O projeto visa projetar e implementar um dispositivo de aferição de temperatura e tempo para um forno industrial, utilizando um microcontrolador da família PIC18 (especificamente o modelo PIC18F4550). Esse sistema tem a finalidade de monitorar a temperatura interna de um forno em intervalos de tempo, uma funcionalidade aplicada em processos de fabricação metálica, pintura eletrostática e manipulação de componentes químicos.
O funcionamento do sistema consiste em realizar a leitura da temperatura na faixa de 0°C a 100°C por meio de um sensor LM35 e exibir esse valor continuamente no display LCD. O dispositivo conta com botões para selecionar tempos predeterminados de medição, entre aferição de curta ou longa duração, exibindo a contagem regressiva correspondente. Além disso, o projeto faz uso de sinalização luminosa via LEDs para simular o estado da resistência do forno em temperaturas elevadas e medidas de prevenção contra o efeito bouncing dos botões mecânicos. A implementação foi realizada via para a simulação no ambiente SimulIDE e execução na placa de desenvolvimento Kit EasyPIC v7.
## Objetivos

Os principais objetivos para esse projeto são:

* Compreender a interação entre os elementos e os circuitos internos dos microcontroladores da família PIC18.
* Simular o comportamento de hardware no ambiente SimulIDE integrando periféricos externos como displays LCD 16x2 em modo de 4 bits, botões e LEDs.
* Desenvolver firmware em linguagem C modularizado aplicando técnicas como tratamento de debounce e cálculo de bases de tempo de timers.


### <ins>Checkpoint 1 – Tratamento do efeito bouncing e acionamento do display LCD<ins>

Para o primeiro *Checkpoint*, o foco principal foi o acionamento do display LCD e o tratamento via software do efeito bouncing (repique). O bouncing é um problema comum em chaves mecânicas, fazendo com que o microcontrolador interprete erroneamente múltiplos acionamentos elétricos ao invés de apenas um.

O desenvolvimento consistiu na criação de um firmware que exibe a frase **"HelloWrld"** na primeira linha de um display LCD operando como uma interface de comunicação de 4 bits. Na segunda linha, foi estruturado um contador de ciclo único de 0 a 9 que é incrementado toda vez que o botão é pressionado. Utilizamos uma flag auxiliar de software e detecção por borda de subida para garantir que o repique dos contatos metálicos do botão fosse totalmente filtrado pelo código, proporcionando uma leitura limpa.

```c
sbit LCD_RS at RB4_bit;
sbit LCD_EN at RB5_bit;
sbit LCD_D4 at RB0_bit;
sbit LCD_D5 at RB1_bit;
sbit LCD_D6 at RB2_bit;
sbit LCD_D7 at RB3_bit;

sbit LCD_RS_Direction at TRISB4_bit;
sbit LCD_EN_Direction at TRISB5_bit;
sbit LCD_D4_Direction at TRISB0_bit;
sbit LCD_D5_Direction at TRISB1_bit;
sbit LCD_D6_Direction at TRISB2_bit;
sbit LCD_D7_Direction at TRISB3_bit;

sbit BOTAO at RD0_bit;
sbit BOTAO_Direction at TRISD0_bit;

unsigned short contador = 0;
unsigned short botao_flag = 0;
char digito[2];

void atualiza_lcd() {
    digito[0] = contador + '0';
    digito[1] = '\0';

    Lcd_Out(1, 1, "HelloWrld");
    Lcd_Out(2, 1, "Contador: ");
    Lcd_Out(2, 11, digito);
}

void main() {
    ADCON1 = 0x0F;
    CMCON = 0x07;

    TRISB = 0x00;
    BOTAO_Direction = 1;

    Lcd_Init();
    Lcd_Cmd(_LCD_CLEAR);
    Lcd_Cmd(_LCD_CURSOR_OFF);

    atualiza_lcd();

    while(1) {
        if (BOTAO == 1 && botao_flag == 0) {
            Delay_ms(30);

            if (BOTAO == 1) {
                botao_flag = 1;

                contador++;

                if (contador > 9) {
                    contador = 0;
                }

                atualiza_lcd();
            }
        }

        if (BOTAO == 0) {
            botao_flag = 0;
        }
    }
}
```

<img width="423" height="364" alt="Circuito - Checkpoint 1" src="https://github.com/user-attachments/assets/f6abc80e-d822-4b97-918a-bcb5dd7eb76d" />




### <ins>Checkpoint 2 – Contagem de tempo utilizado Timers e interrupções no PIC <ins>

Para o segundo *Checkpoint*, desenvolvemos a base de temporização para o contador regressivo. Nesta etapa, implementamos a manipulação de dois timers em conjunto com interrupções externas acionadas por dois botões push buttons distintos.

A lógica aplicada integra o microcontrolador operando com um cristal oscilador de 8 MHz. Utilizamos o temporizador TMR0 configurado com interrupção externa para ser a base de tempo do intervalo de contagem de longa duração (que dura 60 segundos com precisão na casa de 1 segundo). Simultaneamente, o temporizador TMR1 foi configurado para gerenciar o intervalo de curta duração (10 segundos, com base de 250 milissegundos). Todos os dados processados na contagem regressiva passaram a ser enviados para exibição em uma das linhas do display LCD inicializado no checkpoint anterior, consolidando a rotina principal focada em eventos.

```c
sbit LCD_RS at RB4_bit;
sbit LCD_EN at RB5_bit;
sbit LCD_D4 at RB0_bit;
sbit LCD_D5 at RB1_bit;
sbit LCD_D6 at RB2_bit;
sbit LCD_D7 at RB3_bit;

sbit LCD_RS_Direction at TRISB4_bit;
sbit LCD_EN_Direction at TRISB5_bit;
sbit LCD_D4_Direction at TRISB0_bit;
sbit LCD_D5_Direction at TRISB1_bit;
sbit LCD_D6_Direction at TRISB2_bit;
sbit LCD_D7_Direction at TRISB3_bit;

sbit BOTAO_60 at RD0_bit;
sbit BOTAO_10 at RD1_bit;

unsigned short tempo = 0;
unsigned short modo = 0; 
unsigned short rodando = 0;
unsigned short atualiza = 1;

unsigned short ticks_tmr1 = 0; 

char estado_b0 = 0;
char estado_b1 = 0;

void mostra_lcd() {
    char txt[4];

    txt[0] = (tempo / 10) + '0';
    txt[1] = (tempo % 10) + '0';
    txt[2] = '\0';
    
    if (modo == 1) 
    {
        Lcd_Out(1, 1, "Modo: 60s       ");
        Lcd_Out(2, 1, "Tempo: ");
        Lcd_Out(2, 8, txt);
        Lcd_Out(2, 10, "s       "); 
    } 
    
    else if (modo == 2)
    {
        Lcd_Out(1, 1, "Modo: 10s       ");
        Lcd_Out(2, 1, "Tempo: ");
        Lcd_Out(2, 8, txt);
        Lcd_Out(2, 10, "s       "); 
    } 
    
    else
    {
        Lcd_Out(1, 1, "Aperte um botao");
        Lcd_Out(2, 1, "para iniciar...");
    }


}

void inicia_60s() {
    tempo = 60;
    modo = 1;
    rodando = 1;
    T1CON.TMR1ON = 0; 
    TMR0H = 0xC2;
    TMR0L = 0xF7;
    INTCON.TMR0IF = 0;
    T0CON.TMR0ON = 1;
    atualiza = 1;
}

void inicia_10s() {
    tempo = 10;
    modo = 2;
    rodando = 1;
    ticks_tmr1 = 0;

    T0CON.TMR0ON = 0; 
    TMR1H = 0x0B;
    TMR1L = 0xDC;

    PIR1.TMR1IF = 0;
    T1CON.TMR1ON = 1;

    atualiza = 1;
}

void interrupt() {
    if (INTCON.TMR0IF) {
        TMR0H = 0xC2;
        TMR0L = 0xF7;
        INTCON.TMR0IF = 0;

        if (rodando && tempo > 0) {
            tempo--;
            atualiza = 1;

            if (tempo == 0) {
                rodando = 0;
                T0CON.TMR0ON = 0;
            }
        }
    }

    if (PIR1.TMR1IF) {
        TMR1H = 0x0B;
        TMR1L = 0xDC;
        PIR1.TMR1IF = 0;

        if (rodando && tempo > 0) {
            ticks_tmr1++;

            if (ticks_tmr1 >= 4) {
                ticks_tmr1 = 0;
                tempo--;
                atualiza = 1;

                if (tempo == 0) {
                    rodando = 0;
                    T1CON.TMR1ON = 0;
                }
            }
        }
    }
}

void main() {
    ADCON1 = 0x0F; 
    CMCON = 0x07;  

    TRISB = 0x00; 

    TRISD0_bit = 1; 
    TRISD1_bit = 1; 

    Lcd_Init();
    Lcd_Cmd(_LCD_CLEAR);
    Lcd_Cmd(_LCD_CURSOR_OFF);
    
  
    
    T0CON = 0b00000110;
    INTCON.TMR0IE = 1;
    INTCON.TMR0IF = 0;
    
  
    
    T1CON = 0b10110000;
    PIE1.TMR1IE = 1;
    PIR1.TMR1IF = 0;

    INTCON.PEIE = 1; 
    INTCON.GIE = 1; 

    mostra_lcd();

    while (1) {
        if (BOTAO_60 == 1 && estado_b0 == 0) {
            Delay_ms(30);
            if (BOTAO_60 == 1) {
                estado_b0 = 1;
                inicia_60s();
            }
        } else if (BOTAO_60 == 0) {
            estado_b0 = 0;
        }

        if (BOTAO_10 == 1 && estado_b1 == 0) {
            Delay_ms(30);
            if (BOTAO_10 == 1) {
                estado_b1 = 1;
                inicia_10s();
            }
        } else if (BOTAO_10 == 0) {
            estado_b1 = 0;
        }

        if (atualiza) {
            atualiza = 0;
            mostra_lcd();
        }
    }
}
```


## <ins>Entrega Final – Seleção da Temperatura <ins>

Por fim, na entrega final, integramos todos os módulos anteriores ao subsistema de conversão Analógico-Digital (ADC) para criar a lógica de aferição da temperatura. A medição no projeto simula as saídas do sensor LM35 na faixa de 0°C a 100°C usando um potenciômetro ligado aos pinos analógicos do PIC18F4550.

O destaque para esta fase está na configuração dedicada do registrador ADCON1. Como o ADC do PIC tem resolução de 10 bits e o sensor LM35 fornece pequenas variações de tensão, utilizamos uma alimentação de referência externa de 1V ao invés dos convencionais 5V, refinando expressivamente a precisão de conversão. Após o ADC fazer a leitura, o valor processado é formatado para exibir 3 algarismos com ponto flutuante ("XX.X °C") no display sem de fato declarar variáveis float, buscando manter uma alta eficiência da memória do microcontrolador.

Além disso, adicionamos um terceiro botão que atua como acionador geral de todo o processo simultâneo: ao ser disparado, ele liga a leitura contínua da temperatura e a contagem do tempo pré-selecionado (curta ou longa) pelos timers. Se a temperatura do sistema ultrapassar os 50°C, um LED foi configurado para acender em resposta, indicando que a resistência interna do forno está ativa.

```c
Entrega Final Code

```
## Autores
| Nome | NUSP |
| --- | --- |
|Raphael Franco de Oliveira	| 13862393|
|Giulliano Olivato da Silva	| 9944204|
|Guilherme Pereira Loredo   | 11885190|


