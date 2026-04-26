# Sistema de Dosagem Rotativa (8051 Assembly)

## Sobre o projeto

  O projeto visa implementar um sistema de dosagem rotativa utilizando um microcontrolador da família 8051. Esse sistema simula o controle de um mecanismo industrial que dispensa uma quantidade fixa de parafusos para serem ensacodos atravez da manipulação de um motor DC.\
  Os requisitos para o projeto são que o dosador deve ter um ciclo de 10 voltas, exibindo no display de 7 segmentos o número de rotações realizadas e indicando o sentido da rotação pelo ponto decimal no display. O sistema deve conter uma medida no caso de travamento, fazendo com que o motor gire no sentido contrário para destravar a linha de produção, além da implementação de parada imediata e zeragem do display ao mudar o sentido de rotação em meio a contagem.


## Objetivos

Os principais objetivos para esse projeto são:

* Compreender a arquitetura interna de um microcontrolador 8051
* Desenvolvimento low level 
* Simular o comportamento de hardware no ambiente EdSim51
* Aplicar o conhecimento de recursos como pilhas, detecção de evento, subrotinas, integração interfaces externas e registradores GPR e SFR



### <ins>Checkpoint 1 – Leitura dos botões, acionamento dos LEDs e Display de 7 Segmentos <ins>


 ```assembly
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

```assembly
SJMP Inicio

org 01Bh

PUSH Acc
INC 055h 
MOV A, 055h
CJNE A, 051h, Ciclo_Normal

ACALL Zerada   
POP Acc
RETI

Ciclo_Normal:
ACALL Display
POP Acc
RETI


ORG 0033h 
Inicio:
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
ACALL Zerada 
Loop_Horario:
JNB P2.0,reverso
SJMP Loop_Horario

CycAHorario:
MOV DPTR, #TAB_AH
ACALL Zerada
Loop_AHorario:
JB P2.0, reverso
SJMP Loop_AHorario

Reverso:
CPL P3.1
CPL P3.0
JNB P3.1, CycHorario
SJMP CycAHorario

Zerada:
MOV 55h, #0 

Display:
MOV A, 55h
MOVC A, @A+DPTR
MOV P1, A    
RET

TAB_H:
DB 0C0h, 0F9h, 0A4h, 0B0h, 099h, 092h, 082h, 0F8h, 080h, 098h

TAB_AH: 
DB 040h, 079h, 024h, 030h, 019h, 012h, 002h, 078h, 000h, 018h

END

```
## Autores
| Nome | NUSP |
| --- | --- |
|Raphael Franco de Oliveira	| 13862393|
|Giulliano Olivato da Silva	| 9944204|

