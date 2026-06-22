# Aferidor de Temperatura de Forno Industrial (PIC18)

## Sobre o projeto

O projeto visa projetar e implementar um dispositivo de aferição de temperatura e tempo para um forno industrial, utilizando um microcontrolador da família PIC18 (especificamente o modelo PIC18F4550). Esse sistema tem a finalidade de monitorar a temperatura interna de um forno em intervalos de tempo, uma funcionalidade aplicada em processos de fabricação metálica, pintura eletrostática e manipulação de componentes químicos.
O funcionamento do sistema consiste em realizar a leitura da temperatura na faixa de 0°C a 100°C por meio de um sensor LM35 (emulado via potenciômetro) e exibir esse valor continuamente no display LCD. O dispositivo conta com botões para selecionar dinamicamente tempos predeterminados de medição (aferição de curta ou longa duração), exibindo a contagem regressiva correspondente. Além disso, o sistema dispõe de sinalização luminosa (LED) para simular o estado da resistência do forno em temperaturas elevadas e medidas de prevenção contra o efeito bouncing (repique) dos botões mecânicos. A implementação foi direcionada para a simulação no SimulIDE e execução na placa de desenvolvimento Kit EasyPIC v7.
## Objetivos

Os principais objetivos para esse projeto são:

* Compreender a interação entre os elementos e os circuitos internos dos microcontroladores da família PIC18.
* Simular o comportamento de hardware no ambiente SimulIDE integrando periféricos externos como displays LCD 16x2 em modo de 4 bits, botões e LEDs.
* Desenvolver firmware em linguagem C modularizado aplicando técnicas como tratamento de debounce e cálculo de bases de tempo de timers.


### <ins>Checkpoint 1 – Tratamento do efeito bouncing e acionamento do display LCD<ins>

Para o primeiro Checkpoint, o foco principal foi o acionamento do display LCD e o tratamento via software do efeito bouncing (repique). O bouncing é um problema comum em chaves mecânicas, fazendo com que o microcontrolador interprete erroneamente múltiplos acionamentos elétricos ao invés de apenas um.

O desenvolvimento consistiu na criação de um firmware que exibe a frase "HelloWrld" na primeira linha de um display LCD operando em interface de comunicação de 4 bits. Na segunda linha, foi estruturado um contador de ciclo único de 0 a 9 que é incrementado toda vez que o botão é pressionado. Utilizamos uma flag auxiliar de software e detecção por borda de subida para garantir que o repique dos contatos metálicos do botão fosse totalmente filtrado pelo código, proporcionando uma leitura limpa.

ORG 0000H

MAIN:
    MOV DPTR, #TAB

LOOP:
    MOV A, P0  
    JNB P2.0, CH0  
    JNB P2.1, CH1
    JNB P2.2, CH2
    JNB P2.3, CH3
    JNB P2.4, CH4
    JNB P2.5, CH5
    JNB P2.6, CH6
    JNB P2.7, CH7
	SJMP Apagar
    SJMP LOOP  

CH0:
    MOV A, #0
    SJMP DISPLAY

CH1:
    MOV A, #1
    SJMP DISPLAY

CH2:
    MOV A, #2
    SJMP DISPLAY
CH3:
    MOV A, #3
    SJMP DISPLAY

CH4:
    MOV A, #4
    SJMP DISPLAY

CH5:
    MOV A, #5
    SJMP DISPLAY
CH6:
    MOV A, #6
    SJMP DISPLAY

CH7:
    MOV A, #7
	SJMP DISPLAY

Apagar:
	MOV A, #10

DISPLAY:
    MOVC A, @A+DPTR   
    MOV P1, A         
    SJMP LOOP

TAB:
DB 0C0h, 0F9h, 0A4h, 0B0h, 099h, 092h, 082h, 0F8h, 080h, 098h, 0FFh

END
```


### <ins>Checkpoint 2 – Controle da direção do motor <ins>

Para o segundo *Checkpoint* desenvolvemos a lógica do controle de direção do motor DC através de uma chave, nela temos que controle do sentido de rotação é dado pela porta P2.0. Foi feita uma subrotina *Pivo* (em referẽncia aos jogadores pivô do basquete, peça central em grande parte das estratégias) que verifica constantemente a mundaça de direção do giro através do loop das funções de ciclo horário e ciclo anti-horário.\
Aqui, vale ressaltar a principal diferença entre o que o material original da disciplina pedia e o que foi implementado deste cgeckpoint até o código final: A manipulação e checagem direta do pino P3.1 para determinar o sentido de rotação do motor. Durante a criação do código observamos que a criação de uma subrotina que servia estritamente para comparar o valor salvo na Flag 0 (F0) do registrador PSW era desnecessário. Isso porque a informação apresentada no Edsim 51 sobre o funcionamento da ponte H que controla a direção de rotação do motor: Se ele está em P3.1 = 0 e P3.0 = 0 o motor está parado. Em P3.1 = 0 P3.0 = 1 ele está rotacionando em sentido horário. Em P3.1 = 1 P3.0 = 0 ele está rotacionando em sentido antihorário. Em P3.1 = 1 P3.0 = 1  ele também está parado.\
Assim, note que no Edsim51 o estado inicial dos motores é sempre 1 e 1, então o que fazemos é iniciar o motor simplesmente escolhendo qual deles vamor querer que ele inicie. Como o estado normal é dado pela rotação no sentido horário iniciamos ele dando uma instrução de Clear na porta P3.1, e depois basta fazer pooling no estado da chave SW0 (P2.0) para ver se ele mudou de direção. Se ele mudou de direção Chamamos a função Reverso que simplesmente joga uma instrução de Complementary (CPL) nas duas portas. Como ela não faz a movimentação nas duas portas ao mesmo tempo, ela para o motor e só depois, quase instântaneamente, inverte a rotação, já que os dois estados de rotação da ponte H são complementares (invertidos). 

```assembly
Org 0000h
clr P3.1 
 
CycHorario:  
JNB P2.0,Pivo 
SJMP CycHorario

CycAHorario:
JB P2.0, Pivo
SJMP CycAHorario

Pivo:
Acall Reverso
JNB P3.1, CycHorario
SJMP CycAHorario

Reverso:
CPL P3.1
CPL P3.0
RET
end
```

### <ins>Checkpoint 3 – Contagem de voltas com o timer <ins>

Para o terceiro *Checkpoint* desenvolvemos a lógica para contagem de eventos e atualização contínua do display utilizando o registrador TMOD e o temporizador TR1. O código amplia o que foi realizado nos ultimos checkpoints, expandindo as funções de ciclo horário e ciclo anti-horário com rotinas de atualização do display de 7 segmentos e integrando tabelas distintas para os valores do display considerando o uso do ponto decimal para sinalizar o sentido de rotação.\
O que vale ressaltar desse código abaixo é que ele funciona inteiramente sem utilizar interrupções e é uma versão menos otimizada do código final. Isto é, a flag T1 está sendo utilizada, mas ela ainda não está forçando a interrupção no sistema, o que obriga ele a fazer pooling constante no Display para atualizar o valor do contador.\
No caso, mesmo aqui, já são implementadas lógicas análogas a do código final, como por exemplo um registrador (ou um endereço de memória) ser o responsável por controlar o valor máximo do contador, e a tabela TAB_AH. A tabela, foi craida após um teste simples, diminuindo a velocidade do clock emulado no Edsim51 para 1 MHz (Aproximadamente 10% do valor normal de 12 MHz). Quando este teste foi feito e a frequẽncia de atualização foi colocada em 1 e foi possível começar a ver o LED do ponto decimal piscar, uma vez que para toda vez que a função display era chamada no caso Anti Horário era necessário gastar 1 ciclo de máquina adicional só para adicionar o ponto.\
Como o código estava enxuto e havia espaço de memória em abundância disponível, optamos por adicionar a descrição da segunda tabela com o ponto decimal adicional previamente, que permitia que mesmo em casos extremos de baixíssima frequência de clock no oscilador o ponto decimal pudesse ser visto de maneira constante. De quebra, por ele não ficar apagando e acendendo o segmento do ponto decimal isso deve aumentar o tempo de vida útil de um display 7 segmentos da vida real além de economizar uma quantia de ciclos de máquina igual ao número de vezes que o contador era acionado para adicionar o display (isto é, para 10 atualizações, 10 ciclos de máquina seriam poupados só por ter colocado os valores previamente em uma tabela alternativa).   

```assembly

ORG 0000H 

CLR P3.4 
CLR P3.3

MOV TMOD, #060H  
MOV R0, #0F6H
MOV TL1, R0
SETB TR1

CLR P3.1

CycHorario:
MOV DPTR, #TAB_H
Loop_Horario:
JB TF1, Overflow
JNB P2.0,reverso
ACALL Display
SJMP Loop_Horario

CycAHorario:
MOV DPTR, #TAB_AH
Loop_AHorario:
JB TF1, Overflow
JB P2.0, reverso
ACALL Display
SJMP Loop_AHorario

Overflow: 
CLR TF1
ACALL Zerada
SJMP Retorno

Reverso:
CPL P3.1
CPL P3.0
ACALL Zerada

Retorno:
JNB P3.1, CycHorario
SJMP CycAHorario

Zerada:
MOV TL1, R0 

DISPLAY:
MOV A,TL1
SUBB A, R0
MOVC A, @A+DPTR   
MOV P1, A         
RET

TAB_H:
DB 0C0h, 0F9h, 0A4h, 0B0h, 099h, 092h, 082h, 0F8h, 080h, 098h  

TAB_AH: 
DB 040h, 079h, 024h, 030h, 019h, 012h, 002h, 078h, 000h, 018h  

END

```


## <ins>Entrega Final – Integração da mudança de direção <ins>

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


