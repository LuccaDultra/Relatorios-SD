# **LAB 7: Análise e Implementação de Práticas em Sistemas Digitais**

## **I. Introdução Teórica (Acréscimo para o LAB 7\)**

Enquanto as ferramentas de simulação em nível elétrico (como o Spice, abordado no LAB 5\) e de desenho esquemático/layout (abordados no LAB 6\) focam nas características físicas e geométricas do circuito, a complexidade dos sistemas digitais modernos exige níveis mais altos de abstração. Nesse cenário, as Linguagens de Descrição de Hardware (HDL), como o Verilog, desempenham um papel fundamental ao permitir a modelagem e síntese de circuitos complexos de forma independente da tecnologia física final.  
A descrição em nível estrutural funciona de maneira análoga a um diagrama de conexões elétricas, onde o projetista interliga primitivas lógicas lógicas fundamentais (and, or, xor, not) ou módulos preexistentes por meio de canais de comunicação internos denominados *wires*. O Somador Completo (*Full Adder* \- FA) de 1 bit representa a unidade aritmética básica capaz de computar a soma de dois bits de entrada (A e B) considerando o transporte proveniente de um estágio anterior (Cin). Suas saídas de Soma (S) e Transporte de Saída (Cout) obedecem às equações booleanas obtidas na análise do circuito:  
S \= A ⊕ B ⊕ Cin  
Cout \= (A ⊕ B) · Cin \+ A · B  
Para a expansão de largura de palavra para 4 bits, duas arquiteturas clássicas se destacam:

1. **Ripple Carry Adder (RCA):** Uma estrutura linear onde quatro instâncias do *Full Adder* são cascateadas, conectando o Cout de um estágio diretamente ao Cin do estágio subsequente. Embora possua uma pegada de área reduzida, seu desempenho é severamente limitado pelo tempo de propagação do transporte através de toda a cadeia (caminho crítico).  
2. **Carry Lookahead Adder (CLA):** Uma arquitetura otimizada para alto desempenho que elimina a dependência do transporte em cadeia. Ela antecipa os sinais de *carry* calculando variáveis de geração (Gi \= Ai · Bi) e propagação (Pi \= Ai ⊕ Bi) de forma paralela. O CLA troca simplicidade estrutural e área de silício por uma redução drástica no atraso de propagação.

## **II. Materiais Utilizados (Acréscimo)**

* **Ambiente de Prototipação:** Emulador Digital Pitanga (InPlace), Versão Student.  
* **Linguagem de Descrição:** Verilog HDL (padrão estrutural e hierárquico).

## **III. Descrição das Atividades**

As atividades do LAB 7 consistiram no desenvolvimento e validação de módulos aritméticos binários utilizando uma abordagem de projeto hierárquica e modular:

1. **Modelagem do Somador Completo de 1 bit:** Inicialmente, foi gerada a tabela-verdade teórica do circuito com base nas expressões booleanas extraídas da Figura 1\. No editor de texto do emulador Pitanga, foi codificado o módulo full\_adder mapeando estritamente a topologia lógica fornecida através das primitivas lógicas nativas da linguagem. Utilizando a ferramenta gráfica de pinagem do emulador, as portas de entrada (A, B, Cin) foram vinculadas às chaves físicas (sw2, sw1, sw0), enquanto as saídas (S, Cout) foram associadas aos indicadores visuais (led0, led1). O circuito foi sintetizado e testado para todas as 8 combinações de entrada.  
2. **Expansão Hierárquica de 4 bits (Ripple Carry):** Utilizando o módulo de 1 bit como um bloco do tipo "caixa-preta", construiu-se um somador de 4 bits. Declarou-se três barramentos de fios internos (wires) para interligar os transportes intermediários em cascata. O barramento de entrada e saída foi expandido no editor de pinagem para realizar testes de somas complexas e a síntese foi executada para a extração do log de consumo de hardware.  
3. **Implementação do Somador com Antecipação de Transporte (Carry Lookahead):** Desenvolveu-se uma arquitetura alternativa de 4 bits implementando diretamente a lógica de geração e propagação do CLA em Verilog. O circuito foi sintetizado sob as mesmas condições do RCA para viabilizar um confronto direto de métricas de área física emulada.

## **IV. Resultados e Discussões**

A emulação inicial do *Full Adder* de 1 bit validou perfeitamente a tabela-verdade do circuito, apresentando saídas idênticas às previstas teoricamente. A análise do log de síntese gerado pela ferramenta Pitanga indicou o consumo exato de **42 transistores**, representando **0,42%** da matriz lógica total disponível na placa didática emulada. Esse valor é perfeitamente justificado pela decomposição do circuito em células de tecnologia CMOS estática: as duas portas XOR de 2 entradas demandam 12 transistores cada (24 no total), enquanto as duas portas AND e a porta OR de 2 entradas consomem 6 transistores cada uma (18 no total), perfazendo os 42 transistores reportados pelo sintetizador.  
Ao expandir o projeto para o *Ripple Carry Adder* (RCA) de 4 bits através da instanciação direta do módulo base, o log de síntese apontou um consumo de **168 transistores** (**1,68%** da capacidade da placa). Observou-se uma relação estritamente linear (4 × 42 \= 168), o que comprova que o sintetizador obedeceu de forma rigorosa a hierarquia estrutural imposta pelo código Verilog, replicando os blocos de hardware sem aplicar algoritmos de otimização cross-boundary ou minimização lógica global.

Tabela I: Comparativo de Recursos de Síntese

| Topologia do Somador (4 bits) | Transistores | Área (%)   |
| :---: | :---: | ----- |
| Ripple Carry Adder (RCA) | 168 | 1,68% |
| Carry Lookahead Adder (CLA) | 286 | 2,86% |

Em contrapartida, a síntese do somador *Carry Lookahead* (CLA) de 4 bits demonstrou o impacto prático do dilema de projeto (*trade-off*) entre área e velocidade em sistemas digitais. O módulo CLA demandou um total de **286 transistores**, elevando a ocupação da placa para **2,86%** (um acréscimo de aproximadamente 70% em relação ao RCA).  
Esse aumento expressivo no número de transistores decorre do hardware adicional necessário para implementar as equações polinomiais de antecipação dos transportes (C1 a C4). Embora o CLA resulte em um circuito significativamente maior em termos de contagem de transistores, ele elimina o atraso acumulado pelas transições em cascata do RCA. Em sistemas de alto desempenho, o custo adicional em silício é amplamente compensado pelo ganho na frequência máxima de operação do circuito, permitindo que a operação de soma seja concluída em uma fração do tempo exigido pela topologia linear.

## **V. Conclusões (Integração dos LABs 5, 6 e 7\)**

A conclusão deste ciclo de práticas permitiu uma compreensão unificada e profunda do fluxo de projeto de sistemas integrados e microeletrônica digital. No LAB 5, a simulação via Spice forneceu a percepção em nível físico-elétrico necessária para compreender o comportamento íntimo dos transistores e os atrasos de chaveamento. No LAB 6, as ferramentas CAD/EDA elevaram o nível de abstração ao introduzir técnicas de desenho esquemático e manipulação de células lógicas fundamentais. Por fim, o LAB 7 consolidou esses conceitos ao demonstrar como a complexidade de sistemas em larga escala é gerenciada na indústria moderna através do uso de Linguagens de Descrição de Hardware (HDL) e FPGAs.  
A experiência prática com o emulador Pitanga evidenciou que a engenharia de computação moderna depende crucialmente do equilíbrio entre restrições físicas e arquitetura lógica. A comparação quantitativa entre as estruturas *Ripple Carry* e *Carry Lookahead* provou empiricamente que otimizações de velocidade (como a redução do caminho crítico no CLA) demandam uma penalidade direta na área física ocupada por transistores. Esse aprendizado consolida a habilidade do projetista em avaliar logs de síntese e tomar decisões de design conscientes, alinhando as restrições de custo de hardware aos requisitos de performance do sistema digital final.