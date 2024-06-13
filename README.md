# Escalonador de Processos em C

Este projeto consiste em um escalonador de processos implementado em linguagem C. Ele oferece suporte a dois algoritmos de escalonamento: SRTN (Shortest Remaining-Time Next) e Multilevel Feedback Queue. Abaixo estão detalhes sobre como o projeto está estruturado e como executá-lo.

## Funcionalidades

- **SRTN (Shortest Remaining-Time Next)**:
  - Implementação do algoritmo SRTN para escalonamento de processos.
  - Diagrama de tempo de execução gerado como saída.

- **Multilevel Feedback Queue**:
  - Implementação de um esquema de filas múltiplas com realimentação para escalonamento de processos.
  - Geração de listas de processos e processos em espera como saída.


## Como Executar

1. **Compilação**:
   - Certifique-se de ter um compilador C instalado, como GCC.
   - Abra o terminal na pasta do projeto e compile o código-fonte:

     ```bash
     gcc main.c -o escalonador
     ```

2. **Execução**:
   - Após compilar, execute o programa:

     ```bash
     ./escalonador
     ```

3. **Saída**:
   - O programa irá gerar um arquivo `output.txt` com os resultados do escalonamento.

## Exemplo de Uso

Para usar o programa, certifique-se de fornecer um arquivo `input.txt` com os processos a serem escalonados. O formato do arquivo deve ser:
creation_time duration priority


