// Conexões do Módulo LCD
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

// Definição de Botões e LED
sbit BOTAO_SEL at RD0_bit;    // Seleciona o tempo (Curto ou Longo)
sbit BOTAO_START at RD1_bit;  // Inicia a contagem
sbit LED_AQUEC at RD2_bit;    // LED de Resistência (> 50 °C)

// Variáveis Globais
unsigned short tempo_sel = 10; // Modo selecionado (10 ou 60)
unsigned short tempo = 0;      // Tempo atual da contagem regressiva
unsigned short rodando = 0;    // Flag de status (1 = contando)
unsigned short atualiza = 1;   // Flag para atualizar o display
unsigned short ticks_tmr1 = 0;

char estado_b_sel = 0;
char estado_b_start = 0;

unsigned int adc_value = 0;
unsigned int temp_int = 0;
unsigned int temp_int_old = 0xFFFF; // Força primeira atualização do LCD

void inicia_60s() {
    tempo = 60;
    rodando = 1;
    T1CON.TMR1ON = 0;

    // Configura Timer0 para 1 segundo exato (Prescaler 1:128)
    TMR0H = 0xC2;
    TMR0L = 0xF7;
    INTCON.TMR0IF = 0;
    T0CON.TMR0ON = 1;
    atualiza = 1;
}

void inicia_10s() {
    tempo = 10;
    rodando = 1;
    ticks_tmr1 = 0;
    T0CON.TMR0ON = 0;

    // Configura Timer1 para 0.25 segundos (Prescaler 1:8, precisará de 4 ticks)
    TMR1H = 0x0B;
    TMR1L = 0xDC;
    PIR1.TMR1IF = 0;
    T1CON.TMR1ON = 1;
    atualiza = 1;
}

void interrupt() {
    // Interrupção Timer0 (Usado para 60s)
    if (INTCON.TMR0IF) {
        TMR0H = 0xC2;
        TMR0L = 0xF7;
        INTCON.TMR0IF = 0;

        if (rodando && tempo > 0) {
            tempo--;
            atualiza = 1;
            if (tempo == 0) {
                rodando = 0;
                T0CON.TMR0ON = 0; // Desliga Timer0 ao fim
            }
        }
    }

    // Interrupção Timer1 (Usado para 10s)
    if (PIR1.TMR1IF) {
        TMR1H = 0x0B;
        TMR1L = 0xDC;
        PIR1.TMR1IF = 0;

        if (rodando && tempo > 0) {
            ticks_tmr1++;
            if (ticks_tmr1 >= 4) { // 4 x 0.25s = 1s
                ticks_tmr1 = 0;
                tempo--;
                atualiza = 1;

                if (tempo == 0) {
                    rodando = 0;
                    T1CON.TMR1ON = 0; // Desliga Timer1 ao fim
                }
            }
        }
    }
}

void mostra_lcd() {
    char txt_tempo[4];
    char txt_temp[8];

    // Formatação do tempo
    txt_tempo[0] = (tempo / 10) + '0';
    txt_tempo[1] = (tempo % 10) + '0';
    txt_tempo[2] = '\0';

    // Formatação da temperatura em formato "XX.X" usando apenas inteiros
    // temp_int varia de 0 a 1000
    if (temp_int >= 1000) {
        txt_temp[0] = '1'; txt_temp[1] = '0'; txt_temp[2] = '0';
        txt_temp[3] = '.'; txt_temp[4] = '0'; txt_temp[5] = '\0';
    } else {
        txt_temp[0] = (temp_int / 100) ? ((temp_int / 100) + '0') : ' '; // Esconde zero à esquerda
        txt_temp[1] = ((temp_int / 10) % 10) + '0';
        txt_temp[2] = '.';
        txt_temp[3] = (temp_int % 10) + '0';
        txt_temp[4] = '\0';
    }

    // Linha 1: Exibe a Temperatura
    Lcd_Out(1, 1, "Temp: ");
    Lcd_Out(1, 7, txt_temp);
    Lcd_Chr(1, 12, 223); // Imprime o símbolo de grau (°)
    Lcd_Out(1, 13, "C   ");

    // Linha 2: Exibe estado do Temporizador
    if (rodando) {
        Lcd_Out(2, 1, "Restam: ");
        Lcd_Out(2, 9, txt_tempo);
        Lcd_Out(2, 11, "s     ");
    } else {
        if (tempo_sel == 10) {
            Lcd_Out(2, 1, "Modo: 10s (Curt)");
        } else {
            Lcd_Out(2, 1, "Modo: 60s (Long)");
        }
    }
}


void main() {
     // Inicializamos o módulo ADC do MikroC primeiro para não dar problema
    // Isso é: Evita que a biblioteca sobrescreva o ADCON1 depois
    ADC_Init();

    // Daí sim configuramos o ADCON1 apenas após a biblioteca ter sido iniciada
    // VCFG1 (bit 5): 0 (VSS como Vref-)
    // VCFG0 (bit 4): 1 (AN3/RA3 como Vref+ de 1V)
    // PCFG (bits 3 a 0): 1011 (AN0, AN1, AN2 e AN3 como analógicos)
    ADCON1 = 0x1B;

    CMCON = 0x07; // Desliga comparadores

    // Configuração dos pinos (TRIS)
    TRISB = 0x00;     // Porta B como saída (LCD)
    TRISD0_bit = 1;   // RD0: Botão de Seleção (Entrada)
    TRISD1_bit = 1;   // RD1: Botão Start (Entrada)
    TRISD2_bit = 0;   // RD2: LED de Aquecimento (Saída)
    LED_AQUEC = 0;

    TRISA0_bit = 1;   // RA0/AN0: Entrada analógica LM35
    TRISA3_bit = 1;   // RA3/AN3: Entrada tensão Vref+ (1V)

    // Inicializa o LCD
    Lcd_Init();
    Lcd_Cmd(_LCD_CLEAR);
    Lcd_Cmd(_LCD_CURSOR_OFF);
    // Configurações dos Timers e Interrupções
    T0CON = 0b00000110;
    INTCON.TMR0IE = 1;
    INTCON.TMR0IF = 0;

    T1CON = 0b10110000;
    PIE1.TMR1IE = 1;
    PIR1.TMR1IF = 0;

    INTCON.PEIE = 1;
    INTCON.GIE = 1;

    tempo = tempo_sel;
    mostra_lcd();

    while (1) {
        // Leitura do ADC
        // ATENÇÃO: É vital usar ADC_Get_Sample(0) em vez de ADC_Read(0).
        // ADC_Read() força internamente a Vref para os padrões do microcontrolador (5V),
        // anulando nossa configuração do ADCON1, enquanto ADC_Get_Sample() respeita o nosso setup!
        adc_value = ADC_Get_Sample(0);

        // Converte valor ADC (0 a 1023) para "0 a 1000" para ter 1 casa decimal sem usar float.
        // 1023 -> 100.0 graus -> representado em int como 1000.
        temp_int = (unsigned int)(((unsigned long)adc_value * 1000) / 1023);

        // Controle da resistência/forno: Acende LED se > 50.0 °C
        if (temp_int > 500) {
            LED_AQUEC = 1;
        } else {
            LED_AQUEC = 0;
        }

        // Apenas pede para atualizar o LCD se a temperatura mudou
        if (temp_int != temp_int_old) {
            temp_int_old = temp_int;
            atualiza = 1;
        }

        // Lógica: Botão de Seleção de Modo (Atua apenas se parado)
        if (BOTAO_SEL == 1 && estado_b_sel == 0) {
            Delay_ms(30); // Debounce
            if (BOTAO_SEL == 1) {
                estado_b_sel = 1;
                if (!rodando) {
                    if (tempo_sel == 10) tempo_sel = 60;
                    else tempo_sel = 10;
                    tempo = tempo_sel;
                    atualiza = 1;
                }
            }
        } else if (BOTAO_SEL == 0) {
            estado_b_sel = 0;
        }

        // Lógica: Botão Start
        if (BOTAO_START == 1 && estado_b_start == 0) {
            Delay_ms(30); // Debounce
            if (BOTAO_START == 1) {
                estado_b_start = 1;
                if (!rodando) {
                    if (tempo_sel == 60) inicia_60s();
                    else inicia_10s();
                }
            }
        } else if (BOTAO_START == 0) {
            estado_b_start = 0;
        }

        // Rotina de impressão no display (Evita piscar o LCD em loop infinito)
        if (atualiza) {
            atualiza = 0;
            mostra_lcd();
        }

        Delay_ms(20); // Pequeno atraso para manter estabilidade do loop
    }
}