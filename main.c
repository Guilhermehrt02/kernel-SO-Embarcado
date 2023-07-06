#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#define MAX_PROCESSES 10 // o input deve ter o dobro

// Enum para erros no retorno de funcoes
typedef enum
{
    OK,
    ERROR_READ,
    ERROR_WRITE
} error_list;

typedef void (*ptrFunc)(void);

typedef struct
{
    int creation_time;
    int duration;
    int priority;
    int waiting_time;
    int last_execution;
    bool completed;
} Process;

Process processes[MAX_PROCESSES]; // buffer circular
int start, end;                   // parametros de controle
int clock_tick;                   // quantum(controle de execucao dos processos)

// Linhas para escrever no arquivo de saida
char l[20][130] = {
    "##  --  --  --  --  --  --  --  --  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---",
    "--  ##  --  --  --  --  --  --  --  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---",
    "--  --  ##  --  --  --  --  --  --  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---",
    "--  --  --  ##  --  --  --  --  --  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---",
    "--  --  --  --  ##  --  --  --  --  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---",
    "--  --  --  --  --  ##  --  --  --  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---",
    "--  --  --  --  --  --  ##  --  --  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---",
    "--  --  --  --  --  --  --  ##  --  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---",
    "--  --  --  --  --  --  --  --  ##  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---",
    "--  --  --  --  --  --  --  --  --  ###  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---",
    "--  --  --  --  --  --  --  --  --  ---  ###  ---  ---  ---  ---  ---  ---  ---  ---  ---",
    "--  --  --  --  --  --  --  --  --  ---  ---  ###  ---  ---  ---  ---  ---  ---  ---  ---",
    "--  --  --  --  --  --  --  --  --  ---  ---  ---  ###  ---  ---  ---  ---  ---  ---  ---",
    "--  --  --  --  --  --  --  --  --  ---  ---  ---  ---  ###  ---  ---  ---  ---  ---  ---",
    "--  --  --  --  --  --  --  --  --  ---  ---  ---  ---  ---  ###  ---  ---  ---  ---  ---",
    "--  --  --  --  --  --  --  --  --  ---  ---  ---  ---  ---  ---  ###  ---  ---  ---  ---",
    "--  --  --  --  --  --  --  --  --  ---  ---  ---  ---  ---  ---  ---  ###  ---  ---  ---",
    "--  --  --  --  --  --  --  --  --  ---  ---  ---  ---  ---  ---  ---  ---  ###  ---  ---",
    "--  --  --  --  --  --  --  --  --  ---  ---  ---  ---  ---  ---  ---  ---  ---  ###  ---",
    "--  --  --  --  --  --  --  --  --  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ###"};


// functions
void AddProc(int creation_time, int duration, int priority); // adiciona processos
void removeProc(); // remove processos
void srtBatch(Process processos[], int n);// Escalonador Batch: Shortest Remaining-Time Next
void multilevelFeedback(Process processos[], int n, int quantum[], int num_filas);// Escalonador Interativo: Múltiplas filas com realimentação
void kernelInit(void); // inicializador do kernel

// Main
int main(int argc, char **argv)
{
    // Definindo variaveis
    error_list result = OK;
    FILE *file;

    // Ler o arquivo input.txt
    if (result != 0)
        return result;

    return result;

    kernelInit();
    kernelAddProc();
}

void addProc(int creation_time, int duration, int priority)
{
    if (((end + 1) % MAX_PROCESSES) != start) // verifica se esta cheio
    {
        processes[end].creation_time = creation_time;
        processes[end].duration = duration;
        processes[end].priority = priority;

        end = (end + 1) % MAX_PROCESSES; // atualiza a ultima posicao de elementos
    }
}

void removeProc()
{
    if (start != end)
    {                                        // verifica se esta vazio
        start = (start + 1) % MAX_PROCESSES; // atualiza a posicacao inicial
    }
}

void srtBatch(Process processos[], int n)
{
    int tempo_total = 0;
    int i, j;

    // Ordenar os processos por tempo de chegada
    for (i = 0; i < n - 1; i++)
    {
        for (j = 0; j < n - i - 1; j++)
        {
            if (processos[j].creation_time > processos[j + 1].creation_time)
            {
                Process temp = processos[j];
                processos[j] = processos[j + 1];
                processos[j + 1] = temp;
            }
        }
    }

    printf("Escalonamento SRTN (Batch):\n");

    // Executar os processos
    for (i = 0; i < n; i++)
    {
        processos[i].waiting_time = 0;
        processos[i].last_execution = 0;
        processos[i].completed = false;
        tempo_total += processos[i].duration;
    }

    int tempo_atual = 0;
    int processo_atual = -1;
    int concluidos = 0;

    while (concluidos < n)
    {
        int menor_tempo_restante = tempo_total + 1;
        int proximo_processo = -1;

        // Encontrar o próximo processo a ser executado
        for (i = 0; i < n; i++)
        {
            if (!processos[i].completed && processos[i].creation_time <= tempo_atual &&
                (processos[i].duration - processos[i].last_execution) < menor_tempo_restante)
            {
                menor_tempo_restante = processos[i].duration - processos[i].last_execution;
                proximo_processo = i;
            }
        }

        if (proximo_processo == -1)
        {
            tempo_atual++;
            continue;
        }

        // Executar o próximo processo por 1 unidade de tempo
        processos[proximo_processo].last_execution++;
        tempo_atual++;

        if (processos[proximo_processo].last_execution == processos[proximo_processo].duration)
        {
            printf("Tempo %d: Processo %d concluído\n", tempo_atual, proximo_processo);
            processos[proximo_processo].completed = true;
            concluidos++;
        }
    }

    printf("Tempo total de execução: %d unidades de tempo\n", tempo_total);
}

void multilevelFeedback(Process processos[], int n, int quantum[], int num_filas)
{
    int tempo_total = 0;
    int i, j;

    // Ordenar os processos por tempo de chegada
    for (i = 0; i < n - 1; i++)
    {
        for (j = 0; j < n - i - 1; j++)
        {
            if (processos[j].creation_time > processos[j + 1].creation_time)
            {
                Process temp = processos[j];
                processos[j] = processos[j + 1];
                processos[j + 1] = temp;
            }
        }
    }

    printf("Escalonamento Multilevel Feedback:\n");

    // Inicializar as filas com os processos
    int fila_atual[num_filas];
    for (i = 0; i < num_filas; i++)
    {
        fila_atual[i] = -1;
    }

    // Executar os processos
    for (i = 0; i < n; i++)
    {
        processos[i].waiting_time = 0;
        processos[i].last_execution = 0;
        processos[i].completed = false;
        tempo_total += processos[i].duration;
    }

    int tempo_atual = 0;
    int concluidos = 0;

    while (concluidos < n)
    {
        // Verificar se há algum processo na fila atual
        int processo_atual = -1;
        for (i = 0; i < num_filas; i++)
        {
            if (fila_atual[i] != -1)
            {
                processo_atual = fila_atual[i];
                break;
            }
        }

        if (processo_atual == -1)
        {
            tempo_atual++;
            continue;
        }

        // Executar o processo atual por um quantum
        int fila_processo_atual = i;
        int quantum_atual = quantum[fila_processo_atual];
        int tempo_executado = 0;

        while (tempo_executado < quantum_atual)
        {
            processos[processo_atual].last_execution++;
            tempo_atual++;
            tempo_executado++;

            if (processos[processo_atual].last_execution == processos[processo_atual].duration)
            {
                printf("Tempo %d: Processo %d concluído\n", tempo_atual, processo_atual);
                processos[processo_atual].completed = true;
                concluidos++;
                break;
            }
        }

        if (tempo_executado == quantum_atual)
        {
            processos[processo_atual].waiting_time += tempo_atual - processos[processo_atual].last_execution;
            fila_atual[fila_processo_atual] = processo_atual;
        }
        else
        {
            fila_atual[fila_processo_atual] = -1;
        }

        // Realimentação: mover o processo para a próxima fila com prioridade mais baixa
        if (fila_processo_atual < num_filas - 1 && fila_atual[fila_processo_atual] != -1)
        {
            fila_atual[fila_processo_atual + 1] = fila_atual[fila_processo_atual];
            fila_atual[fila_processo_atual] = -1;
        }
    }

    printf("Tempo total de execução: %d unidades de tempo\n", tempo_total);
}

void kernelInit(void)
{
    start = 0;
    end = 0;
    clock_tick = 0;
}

void kernelAddProc(ptrFunc newFunc)
{
    
}


