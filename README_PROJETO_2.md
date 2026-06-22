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

Por fim, a entrega final. Nela, implementamos, finalmente, a lógica de interrupção, que salva uma grande quantidade de ciclos de máquina. Nos checkpoints anteriores já era possível identificar que havia uma lógica de "fallback" (quando um rótulo é acionado e ao finalizar acaba caindo em outra subrotina), especialmente no caso Zerada e display, que sempre tinham que ser acionadas juntas e, portanto, não fazia sentido gastar 2 ciclos de máquina para acionar a subrotina de display que era a próxima no contador de programa (Program Counter) e depois ainda gastar mais 2 cilcos extras usando um segundo retorno (RET). Nesse caso, usamos essa mesma lógica na subrotina Display, que agora faz parte, da rotina de interrupção.\
Nesse caso, a primeira coisa que fazemos é selecionar a Origem do nosso código em no endereço 0033h ao invés do 0000h que utilizavamos anteriormente. Fazemos isso justamente porque dos valores de 0000h a 00032h são os locais escolhidos previamente pelo fabricante para a rotinas de interrupçãoi pularem quando ativas. Assim para a interrupção do Timer 1, o endereço de memória é o 01Bh, então é nele que escrevemos nosso código. Primeiramente pegamos o valor armazenado no endereço de memória 055h (endereço arbitrário que escolhemos), Aumentamos em 1 o valor dele e depois usamos a instrução CJNE (*Compare and Jump if Not Equal*), que é uma instrução de 2 ciclos de máquina e 3 bytes de tamanho, que compara o valor de um registrador (nesse caso escolhemos o Acc), com um valor de memória ou valor arbitrário (nesse caso escolhemos o 051h, que armazena o valor máximo do contador) e se ele não for igual ele vai direto para a função Display. Nesse caso, Ele só não vai ser igual se o valor ainda for menor que 10, se ele estiver nisso, de fato a soma já foi feita e basta apresentar no display (0 funciona no primeiro ciclo pois tanto as funções horárias quanto antihorárias começam zerando o valor de 055h e depois chamam a subrotina de display para imprimir ele no 7 segmentos). Agora, se for 10, significa que o ciclo acabou e tudo que basta ser feito para retornar a normalidade é colocar o valor de 055h para reiniciar a contagem.\
De resto a lógica toda é quase igual a anterior, salvo o fato de agora sim estarmos utilizando a função de reload do Modo 2 do contador interno do 8051. Assim, depois de colocar o TMOD como o modo 2 (01100000), basta carregar o valor que deve ser reabastecido no Byte inferior (TL1), com o valor do Byte superior (TH1). Como queremos que a cada flag de interrupção interna o contador atualize, colocamos esse valor de TH1 para ser #0FFh que é o último valor que o contador assume antes de levantar a flag e acionar a interrupção (que nesse caso está ativa no código, como podemos ver com o MOV IE, #88h). Fora isso o código continua igual ao anterior, exceto por uma pequena adição que não foi pedida no roteiro da prática, mas que é bem importante: Um botão de parada forçada do motor.\
A utilidade para isso é variada, desde segurança até mesmo pausa para manutenção sem a necessidade de desligar todo o circuito. e para implementar essa subrotina é bem simples, se o operador ativar a SW0 ele coloca o motor em um estado de parada (representado pelo P no display de sete segmentos),para isso utilizando a instrução CPL no pino P3.1. Quando ele finalizar ele ele simplesmente desativa ela, dando outra instrução CPL no pino P3.1 e retornando ao estado habitual, na sequência analisando o bit do P3.1 do motor para saber de onde ele deverá voltar, zerando o contador no processo. 

```assembly

SJMP Inicializacao

org 01Bh
INC 055h
MOV A, 055h
CJNE A, 051h, Display
MOV 055h, #0
Display:
MOV A, 55h
MOVC A, @A+DPTR
MOV P1, A    
RETI

ORG 0033h 

Inicializacao:
MOV 051h,#10 
MOV IE,#88h
CLR P3.4
CLR P3.3
MOV TMOD, #060H
MOV TH1, #0FFh
MOV TL1, TH1
SETB TR1
CLR P3.1

CycHorario:
MOV DPTR, #TAB_H
MOV 055h, #0
ACALL Display 
Loop_Horario:
JNB P2.7, Panic
JNB P2.0,Reverso
SJMP Loop_Horario

CycAHorario:
MOV DPTR, #TAB_AH
MOV 055h, #0
ACALL Display
Loop_AHorario:
JNB P2.7, Panic
JB P2.0, Reverso
SJMP Loop_AHorario

Reverso:
JNB P2.7, Panic
CPL P3.1
CPL P3.0
Retorno:
JNB P3.1, CycHorario
SJMP CycAHorario

Panic:
CPL P3.1
MOV P1, #10001100b
Loop_Panic:
JB P2.7, Panic_Finish
SJMP Loop_Panic
Panic_Finish:
MOV P1, #0FFh
CPL P3.1
SJMP Retorno

TAB_H:
DB 0C0h, 0F9h, 0A4h, 0B0h, 099h, 092h, 082h, 0F8h, 080h, 098h

TAB_AH: 
DB 040h, 079h, 024h, 030h, 019h, 012h, 002h, 078h, 000h, 018h
```
## Autores
| Nome | NUSP |
| --- | --- |
|Raphael Franco de Oliveira	| 13862393|
|Giulliano Olivato da Silva	| 9944204|


