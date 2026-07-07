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

Na etapa final do projeto foi desenvolvida uma aplicação própria utilizando a biblioteca MCPWM (Motor Control PWM) da ESP32, explorando seus recursos avançados para o controle de atuadores. Diferentemente da biblioteca LEDC utilizada na primeira etapa, a MCPWM foi projetada especificamente para aplicações envolvendo motores, oferecendo maior flexibilidade na geração de sinais PWM e permitindo a implementação de estratégias de controle mais sofisticadas.

Usando como base a segunda etapa do projeto, foi incorporado um display OLED conectado via barramento I²C para fornecer uma interface de monitoramento em tempo real do sistema de controle do servomotor. Em uma aplicação prática, como um sistema de posicionamento de um **braço robótico** ou de uma **câmera móvel**, saber apenas o sinal PWM enviado ao servo não é suficiente, tal que é igualmente importante que o operador visualize o ângulo de posicionamento do atuador durante seu funcionamento.

O display OLED foi utilizado para exibir continuamente o ângulo atual do servomotor, calculado a partir da leitura do potenciômetro e convertido para a faixa de operação entre 0° e 180°. Além disso, a interface pode apresentar o modo de operação do sistema manual ou automático, facilitando o acompanhamento do comportamento do controlador durante os testes.

A utilização do display torna a interação com o sistema mais intuitiva, eliminando a necessidade de monitorar exclusivamente a comunicação serial. Em aplicações industriais e de automação, uma interface local desse tipo permite que operadores acompanhem rapidamente a posição do atuador, realizem calibrações e validem o funcionamento do sistema diretamente no dispositivo. Dessa forma, o OLED agrega funcionalidade ao projeto, oferecendo uma interface homem-máquina simples, de baixo consumo de energia e adequada para sistemas embarcados baseados na ESP32.

```c
#include <Arduino.h>
#include <Wire.h>

#if __has_include(<esp_arduino_version.h>)
#include <esp_arduino_version.h>
#endif


const int POT_PIN = 34;
const int SERVO_PIN = 18;

const int OLED_SDA = 21;
const int OLED_SCL = 22;

const uint8_t OLED_ADDR = 0x3C;

const int ADC_MIN = 0;
const int ADC_MAX = 4095;

//-
const int ANGLE_MIN = 0;
const int ANGLE_MAX = 180;

const int SERVO_MIN_US = 500;
const int SERVO_MAX_US = 2500;

const int SERVO_FREQUENCY = 50;
const int SERVO_RESOLUTION = 16;
const int SERVO_PERIOD_US = 20000;

const uint32_t PWM_MAX_DUTY = 65535;

// Canal usado apenas em Arduino ESP32 2.x
const int SERVO_CHANNEL = 0;


int adcValue = 0;
int servoAngle = 90;
int pulseWidthUs = 1500;
int positionPercent = 50;
uint32_t dutyDigital = 0;
float dutyPercent = 7.5;

int previousAngle = -1;

unsigned long lastDisplayUpdate = 0;


const uint8_t fontDigits[10][5] = {
  {0x3E, 0x51, 0x49, 0x45, 0x3E},
  {0x00, 0x42, 0x7F, 0x40, 0x00},
  {0x42, 0x61, 0x51, 0x49, 0x46},
  {0x21, 0x41, 0x45, 0x4B, 0x31},
  {0x18, 0x14, 0x12, 0x7F, 0x10},
  {0x27, 0x45, 0x45, 0x45, 0x39},
  {0x3C, 0x4A, 0x49, 0x49, 0x30},
  {0x01, 0x71, 0x09, 0x05, 0x03},
  {0x36, 0x49, 0x49, 0x49, 0x36},
  {0x06, 0x49, 0x49, 0x29, 0x1E}
};

const uint8_t fontUpper[26][5] = {
  {0x7E, 0x11, 0x11, 0x11, 0x7E},
  {0x7F, 0x49, 0x49, 0x49, 0x36},
  {0x3E, 0x41, 0x41, 0x41, 0x22},
  {0x7F, 0x41, 0x41, 0x22, 0x1C},
  {0x7F, 0x49, 0x49, 0x49, 0x41},
  {0x7F, 0x09, 0x09, 0x09, 0x01},
  {0x3E, 0x41, 0x49, 0x49, 0x7A},
  {0x7F, 0x08, 0x08, 0x08, 0x7F},
  {0x00, 0x41, 0x7F, 0x41, 0x00},
  {0x20, 0x40, 0x41, 0x3F, 0x01},
  {0x7F, 0x08, 0x14, 0x22, 0x41},
  {0x7F, 0x40, 0x40, 0x40, 0x40},
  {0x7F, 0x02, 0x0C, 0x02, 0x7F},
  {0x7F, 0x04, 0x08, 0x10, 0x7F},
  {0x3E, 0x41, 0x41, 0x41, 0x3E},
  {0x7F, 0x09, 0x09, 0x09, 0x06},
  {0x3E, 0x41, 0x51, 0x21, 0x5E},
  {0x7F, 0x09, 0x19, 0x29, 0x46},
  {0x46, 0x49, 0x49, 0x49, 0x31},
  {0x01, 0x01, 0x7F, 0x01, 0x01},
  {0x3F, 0x40, 0x40, 0x40, 0x3F},
  {0x1F, 0x20, 0x40, 0x20, 0x1F},
  {0x3F, 0x40, 0x38, 0x40, 0x3F},
  {0x63, 0x14, 0x08, 0x14, 0x63},
  {0x07, 0x08, 0x70, 0x08, 0x07},
  {0x61, 0x51, 0x49, 0x45, 0x43}
};

const uint8_t glyphSpace[5] = {0x00, 0x00, 0x00, 0x00, 0x00};
const uint8_t glyphColon[5] = {0x00, 0x36, 0x36, 0x00, 0x00};
const uint8_t glyphPercent[5] = {0x63, 0x13, 0x08, 0x64, 0x63};

// -----------------------------------------------------------------------------
// Funcoes do OLED
// -----------------------------------------------------------------------------
void oledCommand(uint8_t command)
{
  Wire.beginTransmission(OLED_ADDR);
  Wire.write(0x00);
  Wire.write(command);
  Wire.endTransmission();
}

void oledData(uint8_t data)
{
  Wire.beginTransmission(OLED_ADDR);
  Wire.write(0x40);
  Wire.write(data);
  Wire.endTransmission();
}

void oledSetCursor(uint8_t column, uint8_t page)
{
  oledCommand(0xB0 + page);
  oledCommand(0x00 + (column & 0x0F));
  oledCommand(0x10 + ((column >> 4) & 0x0F));
}

void oledClear()
{
  for (int page = 0; page < 8; page++) {
    oledSetCursor(0, page);

    for (int column = 0; column < 128; column++) {
      oledData(0x00);
    }
  }
}

const uint8_t* getGlyph(char c)
{
  if (c >= '0' && c <= '9') {
    return fontDigits[c - '0'];
  }

  if (c >= 'A' && c <= 'Z') {
    return fontUpper[c - 'A'];
  }

  if (c == ':') {
    return glyphColon;
  }

  if (c == '%') {
    return glyphPercent;
  }

  return glyphSpace;
}

void oledPrintChar(char c)
{
  const uint8_t* glyph = getGlyph(c);

  for (int i = 0; i < 5; i++) {
    oledData(glyph[i]);
  }

  oledData(0x00);
}

void oledPrintText(uint8_t column, uint8_t page, const char* text)
{
  oledSetCursor(column, page);

  while (*text) {
    oledPrintChar(*text);
    text++;
  }
}

void oledInit()
{
  delay(100);

  oledCommand(0xAE);
  oledCommand(0xD5);
  oledCommand(0x80);
  oledCommand(0xA8);
  oledCommand(0x3F);
  oledCommand(0xD3);
  oledCommand(0x00);
  oledCommand(0x40);
  oledCommand(0x8D);
  oledCommand(0x14);
  oledCommand(0x20);
  oledCommand(0x02);
  oledCommand(0xA1);
  oledCommand(0xC8);
  oledCommand(0xDA);
  oledCommand(0x12);
  oledCommand(0x81);
  oledCommand(0xCF);
  oledCommand(0xD9);
  oledCommand(0xF1);
  oledCommand(0xDB);
  oledCommand(0x40);
  oledCommand(0xA4);
  oledCommand(0xA6);
  oledCommand(0xAF);

  oledClear();
}

void updateOLED()
{
  char line[20];

  oledClear();

  oledPrintText(0, 0, "PWM SERVO");

  sprintf(line, "ADC:%4d", adcValue);
  oledPrintText(0, 2, line);

  sprintf(line, "ANG:%3d", servoAngle);
  oledPrintText(0, 3, line);

  sprintf(line, "POS:%3d%%", positionPercent);
  oledPrintText(0, 4, line);

  sprintf(line, "PULSO:%4d", pulseWidthUs);
  oledPrintText(0, 5, line);

  oledPrintText(0, 7, "ESTADO:OK");
}


int readPotentiometer()
{
  long sum = 0;
  const int samples = 16;

  for (int i = 0; i < samples; i++) {
    sum += analogRead(POT_PIN);
    delayMicroseconds(150);
  }

  return sum / samples;
}


uint32_t pulseToDuty(int pulseWidth)
{
  uint64_t calculation;

  calculation =
    ((uint64_t)pulseWidth * PWM_MAX_DUTY) /
    SERVO_PERIOD_US;

  return (uint32_t)calculation;
}


bool configureServoPWM()
{
#if defined(ESP_ARDUINO_VERSION_MAJOR) && ESP_ARDUINO_VERSION_MAJOR >= 3

  return ledcAttach(
    SERVO_PIN,
    SERVO_FREQUENCY,
    SERVO_RESOLUTION
  );

#else

  double configuredFrequency;

  configuredFrequency = ledcSetup(
    SERVO_CHANNEL,
    SERVO_FREQUENCY,
    SERVO_RESOLUTION
  );

  ledcAttachPin(
    SERVO_PIN,
    SERVO_CHANNEL
  );

  return configuredFrequency > 0;

#endif
}


void writeServoAngle(int angle)
{
  angle = constrain(
    angle,
    ANGLE_MIN,
    ANGLE_MAX
  );

  pulseWidthUs = map(
    angle,
    ANGLE_MIN,
    ANGLE_MAX,
    SERVO_MIN_US,
    SERVO_MAX_US
  );

  dutyDigital = pulseToDuty(pulseWidthUs);

  dutyPercent =
    ((float)pulseWidthUs /
    (float)SERVO_PERIOD_US) * 100.0;

#if defined(ESP_ARDUINO_VERSION_MAJOR) && ESP_ARDUINO_VERSION_MAJOR >= 3

  ledcWrite(
    SERVO_PIN,
    dutyDigital
  );

#else

  ledcWrite(
    SERVO_CHANNEL,
    dutyDigital
  );

#endif
}


void setup()
{
  Serial.begin(115200);
  delay(500);

  analogReadResolution(12);
  pinMode(POT_PIN, INPUT);

  Wire.begin(OLED_SDA, OLED_SCL);
  Wire.setClock(400000);

  oledInit();

  if (configureServoPWM() == false) {
    Serial.println("Erro ao configurar PWM do servo.");

    while (true) {
      delay(1000);
    }
  }

  writeServoAngle(90);

  Serial.println();
  Serial.println("SEL0433 - Projeto 3 - Parte 2 - Exercicio 2");
  Serial.println("Controle de servo por potenciometro com OLED");
  Serial.println("--------------------------------------------");
  Serial.println("Potenciometro: GPIO34");
  Serial.println("Servo PWM: GPIO18");
  Serial.println("OLED SDA: GPIO21");
  Serial.println("OLED SCL: GPIO22");
  Serial.println("UART: 115200 baud");
  Serial.println();

  delay(500);
}


void loop()
{
  adcValue = readPotentiometer();

  adcValue = constrain(
    adcValue,
    ADC_MIN,
    ADC_MAX
  );

  servoAngle = map(
    adcValue,
    ADC_MIN,
    ADC_MAX,
    ANGLE_MIN,
    ANGLE_MAX
  );

  positionPercent = map(
    adcValue,
    ADC_MIN,
    ADC_MAX,
    0,
    100
  );

  if (servoAngle != previousAngle) {
    writeServoAngle(servoAngle);
    previousAngle = servoAngle;
  }

  Serial.print("ADC: ");
  Serial.print(adcValue);

  Serial.print(" | Posicao: ");
  Serial.print(positionPercent);
  Serial.print("%");

  Serial.print(" | Angulo: ");
  Serial.print(servoAngle);

  Serial.print(" | Pulso: ");
  Serial.print(pulseWidthUs);
  Serial.print(" us");

  Serial.print(" | Duty digital: ");
  Serial.print(dutyDigital);

  Serial.print(" | Duty percentual: ");
  Serial.print(dutyPercent, 2);
  Serial.println("%");

  if (millis() - lastDisplayUpdate > 300) {
    updateOLED();
    lastDisplayUpdate = millis();
  }

  delay(100);
}
```
<img width="423" height="364" alt="Circuito - Entrega Final" src="https://github.com/Franc0liv/SEL0433/blob/main/Projeto3_Misc/gif_esp_final.gif" />

## Autores
| Nome | NUSP |
| --- | --- |
|Raphael Franco de Oliveira	| 13862393|
|Giulliano Olivato da Silva	| 9944204|
|Guilherme Pereira Loredo   | 11885190|


