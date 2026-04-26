# Sistema de Dosagem Rotativa (8051 Assembly)

## Sobre o projeto

  O projeto visa implementar um sistema de dosagem rotativa utilizando um microcontrolador da família 8051. Esse sistema simula o controle de um mecanismo industrial que dispensa uma quantidade fixa de parafusos para serem ensacodos atravez da manipulação de um motor DC. Os requisitos para o projeto são que o dosador deve girar exatamente 10 voltas e deve conter uma medida no caso de travamento, fazendo com que o motor gire no sentido contrário para destravar a linha de produção.



## Objetivos

Os principais objetivos para esse projeto são:

* Compreender a arquitetura interna de um microcontrolador 8051
* Desenvolvimento low level 
* Simular o comportamento de hardware no ambiente EdSim51 



### <ins>Checkpoint 1 – Leitura dos botões, acionamento dos LEDs e Display de 7 Segmentos <ins>


 ```assembly
ORG 0000H

MAIN:
    MOV DPTR, #TAB

LOOP:
    MOV A, P0  ; Leitura das chaves

; Checando a cada bit para saber qual chave foi pressionada
; Nesse caso, a de menor indice tem maior prioridade
    JNB P2.0, CH0  
    JNB P2.1, CH1
    JNB P2.2, CH2
    JNB P2.3, CH3
    JNB P2.4, CH4
    JNB P2.5, CH5
    JNB P2.6, CH6
    JNB P2.7, CH7

	SJMP Apagar;Se nenhuma chave for selecionada apaga o display para economizar energia.	
    SJMP LOOP  ; Loop para continuar olhando as chaves

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
    MOVC A, @A+DPTR   ; Busca na tabela
    MOV P1, A         ; Envia para o display
    SJMP LOOP

TAB:
DB 0C0h, 0F9h, 0A4h, 0B0h, 099h, 092h, 082h, 0F8h, 080h, 098h, 0FFh

END
```

### <ins>Checkpoint 2 – Controle da direção do motor <ins>

```assembly
Org 0000h ;Joga a origem do codigo para 0000h
clr P3.1 ;Seta o motor para rodar em sentido horario por padrao
 
CycHorario:  
JNB P2.0,Pivo 
SJMP CycHorario

CycAHorario:
JB P2.0, Pivo
SJMP CycAHorario

;Pivo: Interliga a inversao do motor que acontece quando o SW0 e pressionado com o 
;reencaminhamento para a subrotina correta, utilizando para isso o nivel logico no 
;pino P3.1 ( Quando estiver com valor alto (1) ele vai estar no sentido horario, se nao entao estara no sentido antihorario). 
;Em outras palavras, permite que o motor continue girando caso o usuario pare de clicar no botao. 

Pivo:
Acall Reverso
JNB P3.1, CycHorario
SJMP CycAHorario

;Reverso: Essa subrotina, sempre que acionada,Inverte o valor dos bits da P3.1 e da P3.0 (isso por conta da operacao de complementar) 
Reverso:
CPL P3.1
CPL P3.0
RET
end
```







### <ins>Checkpoint 3 – Contagem de voltas com o timer <ins>

```assembly
;-=-=-=-=-=-=-=-=-=--=-=-=-=-=-=-=-=-=-=-=-=-=-=---=-=-=-=-=-=-=-=-=-=-
;Bloco 1: Setup
;-=-=-=-=-=-=-=-=-=--=-=-=-=-=-=-=-=-=-=-=-=-=-=---=-=-=-=-=-=-=-=-=-=-
ORG 0000H ;Inicia o codigo em #0000h, uma vez que nao utilizamos interrupcoes nesse caso. 

;Seta o mux do 7 segmentos para o 1o display (direita para esquerda)
CLR P3.4 
CLR P3.3

;Seta o contador 1 como ativo,com o overflow ajustado caso o byte inferior(TL1) exceda #0FFh. (nesse caso M1 = 1; M0 = 0)
;(TMOD fica: (0110 0000)
MOV TMOD, #060H  
MOV R0, #0F6H ;Armazena valor que vai ser usado como limite da contagem no Registrador 0 do banco 0. Para decada(0-9): #0F6h.
MOV TL1, R0;condicao inicial para o contador contar so ate 9 (reiniciando no 10)
SETB TR1; Inicia o contador 1

CLR P3.1 ; Inicia a rotacao do motor em sentido horario
;-=-=-=-=-=-=-=-=-=--=-=-=-=-=-=-=-=-=-=-=-=-=-=---=-=-=-=-=-=-=-=-=-=-
;Bloco 2: Definindo as funcoes de ciclo Horario e Anti_Horario 
;-=-=-=-=-=-=-=-=-=--=-=-=-=-=-=-=-=-=-=-=-=-=-=---=-=-=-=-=-=-=-=-=-=-

;O comportamento do sentido de rotacao do motor eh dado pelo estado do bit em P3.1, de modo que
;as seguintes funcoes CycHorario e CycAHorario sao enderecadas na checagem da funcao Retorno definida
;mais abaixo. Cada funcao possui uma tabela correspontente armazenando os valores que serao indexados ao display,
;seguindo a orientacao de sinalizar o sentido de rotacao antihorario com ponto inferior ligado.
;Ambas funcoes operam no mesmo principio, primeiro utilizando o ponteiro de dados para enderecar diretamente a tabela 
;correspondente ao sentido de rotacao e entao criando um loop que garante o ciclo de contagem no display, checa para caso o sentido de 
;rotacao tenha sido alterado em meio a contagem e entao atualiza o valor no display. 

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

;-=-=-=-=-=-=-=-=-=--=-=-=-=-=-=-=-=-=-=-=-=-=-=---=-=-=-=-=-=-=-=-=-=-
;Bloco 3: Overflow 
;-=-=-=-=-=-=-=-=-=--=-=-=-=-=-=-=-=-=-=-=-=-=-=---=-=-=-=-=-=-=-=-=-=-

;A subrotina Overflow eh acionada quando o valor do registrador TF1 atinge 1.
;como ajustamos o contador 1, la no inicio do codigo para que ele operasse em modo
;8 bits, isso significa que toda vez que ele ultrapassar o valor #0FFh
;ele vai chamar a flag TF1 e reiniciar em #000h. Assim, esse bloco limpa a
;flag, zera o display e tambem reajusta o valor do contador para #0F6h
;reiniciando o ciclo de operacao do contador

Overflow: 
CLR TF1
ACALL Zerada
SJMP Retorno

;-=-=-=-=-=-=-=-=-=--=-=-=-=-=-=-=-=-=-=-=-=-=-=---=-=-=-=-=-=-=-=-=-=-
;Bloco 4: "Quarteto fantastico"
;-=-=-=-=-=-=-=-=-=--=-=-=-=-=-=-=-=-=-=-=-=-=-=---=-=-=-=-=-=-=-=-=-=-
;Esse bloco e o coracao do programa, basicamente, sempre que reverso e chamado ele chama zerada, que, naturalmente executa a subrotina display.
;Display, por sua vez, sempre que chamado encerra com um RET, que devolve o ultimo valor registrado pela funcao ACALL no stackpointer.
;Dessa forma, o posicionamento das subrotinas foi pensado para otimizar o codigo e sua memoria de programa:
;toda vez que zerada e chamada como subrotina ela precisa so reiniciar o valor maximo do contador e depois atualizar o display, logo, o ret do display retorna
;para a funcao original, economizando 4 ciclos de maquina que seriam necessarios para executar o zerada e o display depois! Ao mesmo tempo, usamos um rotulo
;para o retorno, que e um pequeno bloco utilizado tanto pela subrotina overflow quanto pela rotina reverso. Basicamente, quando usada pelo reverso, apos 
;inverter a rotacao do motor na ponte h, ele ve se era para retornar para o loop de rotacao do sentido horario ou antihorario.
;Assim, separando em 2 rotulos, se torna possivel economizar memoria do programa repetindo a estrutura de retorno que teria que ser escrita 2 vezes
; com um retorno.
;De quebra, no caso do reverso, Ainda economizamos 2 ciclos de maquina ja que naturalmente o PC vai mandar o codigo continuar do ret do display
;para a logica de sentido do motor.
;Em resumo: a subrotina retorno, impede o codigo de atingir seu final e ainda envia o codigo para o loop correto. 

Reverso:
CPL P3.1
CPL P3.0
ACALL Zerada

Retorno:
JNB P3.1, CycHorario
SJMP CycAHorario

;Subrotina Zerada esta protegida pela ramificacao acima, ao mesmo tempo, se for chamada executa zerada e display (que sempre e necessario)
Zerada:
MOV TL1, R0 

;Display: Subrotina que serve meramente para atualizar o display a partir do valor do byte inferior do Temporizador 1 (nesse caso: contador de eventos)
DISPLAY:
MOV A,TL1
SUBB A, R0
MOVC A, @A+DPTR   ; Busca na tabela
MOV P1, A         ; Envia para o display 
RET

TAB_H:
DB 0C0h, 0F9h, 0A4h, 0B0h, 099h, 092h, 082h, 0F8h, 080h, 098h  ;Prog 7 Segmentos do 0 ate o 9

;Sacrificamos Alguns bytes de memoria para economizar ciclos de maquina. Caso escolhessemos adicionar o ponto decimal manualmente na tabela TAB_H, 
;Teriamos que perder 1 ciclo de maquina por Iteracao para adicionar o ponto decimal no caso Anti horario.
TAB_AH: 
DB 040h, 079h, 024h, 030h, 019h, 012h, 002h, 078h, 000h, 018h  ;Prog 7 segmentos 0 ate o 9 com ponto decimal (rotacao antihoraria)

END
```


