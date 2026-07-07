# Controle PWM e Comunicação (ESP32)

## Sobre o projeto

O projeto visa explorar os recursos de controle PWM e comunicação disponíveis na plataforma ESP32 DevKit, por meio do desenvolvimento de aplicações voltadas ao acionamento de dispositivos eletrônicos e monitoramento do sistema. Ao longo da atividade são utilizados periféricos como LED RGB, servomotor, display OLED, comunicação serial UART e a biblioteca MCPWM, permitindo o estudo de técnicas de controle aplicadas a sistemas embarcados modernos.
Os principais objetivos para esse projeto são:

* Compreender o funcionamento do controle PWM na plataforma ESP32 utilizando as bibliotecas LEDC e MCPWM.
* Desenvolver aplicações que integrem diferentes periféricos da ESP32, como ADC, UART, display OLED e GPIOs.
* Explorar técnicas de controle de dispositivos eletrônicos utilizando modulação por largura de pulso.
* Aplicar conceitos de programação embarcada para desenvolvimento de sistemas modulares e escaláveis.
* Validar o funcionamento do sistema por meio de simulações no ambiente Wokwi.

### <ins>Checkpoint 1 – Controle PWM de LED RGB<ins>

Para o primeiro *Checkpoint*, foi desenvolvido um sistema de controle de um LED RGB de cátodo comum utilizando a biblioteca LEDC, responsável pela geração dos sinais PWM da ESP32. Cada componente de cor (vermelho, verde e azul) foi conectado a um canal PWM independente, configurado com frequência de 5 kHz e resolução de 8 bits, permitindo o controle individual da intensidade luminosa.

O firmware foi estruturado para variar continuamente o duty cycle de cada canal entre 0% e 100%, utilizando incrementos distintos para cada cor. Dessa forma, diferentes combinações de intensidade são produzidas automaticamente ao longo da execução, demonstrando o funcionamento da modulação PWM aplicada ao controle de iluminação.

Durante toda a operação, os valores de duty cycle e os incrementos aplicados são enviados pela interface UART, configurada para 115200 baud, permitindo o monitoramento do comportamento do sistema através do terminal serial. Essa etapa permitiu compreender o funcionamento da biblioteca LEDC, a configuração de múltiplos canais PWM e a utilização da comunicação serial como ferramenta de depuração.

```c
#include <Arduino.h>

#if __has_include(<esp_arduino_version.h>)
#include <esp_arduino_version.h>
#endif

const int PIN_RED = 25;
const int PIN_GREEN = 26;
const int PIN_BLUE = 27;

const int PWM_FREQUENCY = 5000;
const int PWM_RESOLUTION = 8;
const int PWM_MAX_DUTY = 255;

const int RED_STEP = 15;
const int GREEN_STEP = 5;
const int BLUE_STEP = 10;

const int CHANNEL_RED = 0;
const int CHANNEL_GREEN = 1;
const int CHANNEL_BLUE = 2;

int redPercent = 0;
int greenPercent = 0;
int bluePercent = 0;

int percentToDuty(int percent)
{
  if (percent < 0) {
    percent = 0;
  }

  if (percent > 100) {
    percent = 100;
  }

  return (percent * PWM_MAX_DUTY) / 100;
}

int advancePercent(int currentValue, int stepValue)
{
  if (currentValue >= 100) {
    return 0;
  }

  int nextValue = currentValue + stepValue;

  if (nextValue > 100) {
    nextValue = 100;
  }

  return nextValue;
}


bool configurePWM()
{
#if defined(ESP_ARDUINO_VERSION_MAJOR) && ESP_ARDUINO_VERSION_MAJOR >= 3

  // Arduino ESP32 versao 3.x
  bool redConfigured = ledcAttach(
    PIN_RED,
    PWM_FREQUENCY,
    PWM_RESOLUTION
  );

  bool greenConfigured = ledcAttach(
    PIN_GREEN,
    PWM_FREQUENCY,
    PWM_RESOLUTION
  );

  bool blueConfigured = ledcAttach(
    PIN_BLUE,
    PWM_FREQUENCY,
    PWM_RESOLUTION
  );

  return redConfigured &&
         greenConfigured &&
         blueConfigured;

#else

  double redFrequency = ledcSetup(
    CHANNEL_RED,
    PWM_FREQUENCY,
    PWM_RESOLUTION
  );

  double greenFrequency = ledcSetup(
    CHANNEL_GREEN,
    PWM_FREQUENCY,
    PWM_RESOLUTION
  );

  double blueFrequency = ledcSetup(
    CHANNEL_BLUE,
    PWM_FREQUENCY,
    PWM_RESOLUTION
  );

  ledcAttachPin(PIN_RED, CHANNEL_RED);
  ledcAttachPin(PIN_GREEN, CHANNEL_GREEN);
  ledcAttachPin(PIN_BLUE, CHANNEL_BLUE);

  return redFrequency > 0 &&
         greenFrequency > 0 &&
         blueFrequency > 0;

#endif
}

void setRGB(int red, int green, int blue)
{
  int redDuty = percentToDuty(red);
  int greenDuty = percentToDuty(green);
  int blueDuty = percentToDuty(blue);

#if defined(ESP_ARDUINO_VERSION_MAJOR) && ESP_ARDUINO_VERSION_MAJOR >= 3

  ledcWrite(PIN_RED, redDuty);
  ledcWrite(PIN_GREEN, greenDuty);
  ledcWrite(PIN_BLUE, blueDuty);

#else

  
  ledcWrite(CHANNEL_RED, redDuty);
  ledcWrite(CHANNEL_GREEN, greenDuty);
  ledcWrite(CHANNEL_BLUE, blueDuty);

#endif
}

void setup()
{
  Serial.begin(115200);

  delay(500);

  Serial.println();
  Serial.println("SEL0433 - Projeto 3 - Parte 1");
  Serial.println("Controle PWM de LED RGB");
  Serial.println("Frequencia PWM: 5 kHz");
  Serial.println("Resolucao PWM: 8 bits");

  Serial.print("Incremento vermelho: ");
  Serial.print(RED_STEP);
  Serial.println("%");

  Serial.print("Incremento verde: ");
  Serial.print(GREEN_STEP);
  Serial.println("%");

  Serial.print("Incremento azul: ");
  Serial.print(BLUE_STEP);
  Serial.println("%");

  if (configurePWM() == false) {
    Serial.println("Erro ao configurar os canais PWM.");

    while (true) {
      delay(1000);
    }
  }

  
  setRGB(0, 0, 0);

  Serial.println();
  Serial.println("PWM configurado corretamente.");
  Serial.println();
}

void loop()
{
 
  setRGB(
    redPercent,
    greenPercent,
    bluePercent
  );

  Serial.print("Vermelho: ");
  Serial.print(redPercent);
  Serial.print("%");
  Serial.print(" | Duty: ");
  Serial.print(percentToDuty(redPercent));

  Serial.print("   Verde: ");
  Serial.print(greenPercent);
  Serial.print("%");
  Serial.print(" | Duty: ");
  Serial.print(percentToDuty(greenPercent));

  Serial.print("   Azul: ");
  Serial.print(bluePercent);
  Serial.print("%");
  Serial.print(" | Duty: ");
  Serial.println(percentToDuty(bluePercent));

  redPercent = advancePercent(
    redPercent,
    RED_STEP
  );

  greenPercent = advancePercent(
    greenPercent,
    GREEN_STEP
  );

  bluePercent = advancePercent(
    bluePercent,
    BLUE_STEP
  );

  delay(250);
}
```

<img width="423" height="364" alt="Circuito - Checkpoint 1" src="https://github.com/Franc0liv/SEL0433/blob/main/Projeto3_Misc/gif_esp32_proj3.gif" />




### <ins>Checkpoint 2 – Controle de Servomotor utilizando PWM <ins>

Para o segundo *Checkpoint*, desenvolvemos um sistema de controle manual de posição de um servomotor utilizando um potenciômetro como dispositivo de entrada. A tensão analógica fornecida pelo potenciômetro é adquirida pelo conversor ADC da ESP32, sendo posteriormente convertida para um valor correspondente ao ângulo de rotação do servomotor.

Para o acionamento do servo foi utilizada a biblioteca ESP32Servo, responsável por gerar automaticamente os pulsos PWM necessários para o posicionamento do eixo. Dessa forma, a movimentação do potenciômetro resulta diretamente na alteração do ângulo do servomotor, proporcionando uma interface simples para o estudo da conversão analógico-digital integrada ao controle PWM.

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
<img width="618" height="440" alt="Circuito - Checkpoint 2" src="https://github.com/Franc0liv/SEL0433/blob/main/Projeto3_Misc/gif_esp32_2.gif" />

## <ins>Entrega Final – Aplicação utilizando MCPWM <ins>



```c

```


## Autores
| Nome | NUSP |
| --- | --- |
|Raphael Franco de Oliveira	| 13862393|
|Giulliano Olivato da Silva	| 9944204|
|Guilherme Pereira Loredo   | 11885190|


