#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#define MAX_PROCESSES 5 // o input deve ter o dobro

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
Process waiting_queue[2*MAX_PROCESSES];
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
void addProc(int creation_time, int duration, int priority); // adiciona processos
void removeProc(); // remove processos
void srtBatch(Process processos[], int n);// Escalonador Batch: Shortest Remaining-Time Next
void multilevelFeedback(Process processos[], int n);// Escalonador Interativo: Múltiplas filas com realimentação
void kernelInit(void); // inicializador do kernel
void kernelAddProc(void);
void kernelLoop(void);
int open_and_read_file(FILE *f);
void print_process_list(Process *vet);


// Main
int main(int argc, char **argv)
{
    kernelInit();
    kernelAddProc();
    srtBatch(processes,MAX_PROCESSES);
    print_process_list(processes);
    //kernelLoop(srtBatch);
    //kernelLoop(multilevelFeedback);
}

int open_and_read_file(FILE *f)
{
    f = fopen("input.txt", "r");
    
    if(f == NULL)
        return ERROR_READ;

    int v;
    for(int i = 0; i < 2*MAX_PROCESSES; i++)
    {
        // Colocar os valores do arquivo nas variaveis do Processo
        v = fscanf(f, "%d %d %d", &waiting_queue[i].creation_time, &waiting_queue[i].duration,
            &waiting_queue[i].priority);
        waiting_queue[i].completed = false;
        waiting_queue[i].last_execution = waiting_queue[i].creation_time;
        waiting_queue[i].waiting_time = 0;
    }

    if(f) fclose(f);
    return OK;
}

void kernelInit(void)
{
    //inicializacao do buffer circular
    start = 0;
    end = 0;
    //inicializacao da estrutura de controle de execucao
    clock_tick = 5;

    error_list result = OK;
    FILE *file;

    // Ler o arquivo input.txt
    open_and_read_file(file);
    //if(result != 0) return result;
    printf("lista de espera");
    print_process_list(waiting_queue);


}

void kernelAddProc()
{
    //adiciona a quantidade maxima de processos
    for (int i = 0; i < MAX_PROCESSES; i++)
    {
        addProc(waiting_queue[i].creation_time,waiting_queue[i].duration,waiting_queue[i].priority);
    }
}

void kernelLoop(){
    
    for(int i=0;i < 2* MAX_PROCESSES;i++){
        srtBatch(processes,MAX_PROCESSES);
        
    }
    
}

void print_process_list(Process *vet)
{
    printf("========== LIST OF PROCESSES ==========\n");
    printf("Creation Time    duration  Priority\n");
    for(int i = 0; i < MAX_PROCESSES; i++)
        printf("      %2d           %2d       %2d\n", vet[i].creation_time, vet[i].duration,
            vet[i].priority);
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

void srtBatch(Process processos[], int n) {
    int current_time = 0;
    int completed_processes = 0;

    while (completed_processes < n) {
        int shortest_remaining_time = -1;
        int selected_process = -1;

        for (int i = 0; i < n; i++) {
            if (!processos[i].completed && processos[i].creation_time <= current_time) {
                int remaining_time = processos[i].duration - (current_time - processos[i].last_execution);

                if (shortest_remaining_time == -1 || remaining_time < shortest_remaining_time) {
                    shortest_remaining_time = remaining_time;
                    selected_process = i;
                }
            }
        }

        if (selected_process != -1) {
            Process *process = &processos[selected_process];

            // Executa o processo pelo tempo de clock_tick (quantum)
            int execution_time = shortest_remaining_time < clock_tick ? shortest_remaining_time : clock_tick;
            current_time += execution_time;
            process->duration -= execution_time;

            if (process->duration == 0) {
                process->completed = true;
                completed_processes++;
            } else {
                process->last_execution = current_time;
                process->waiting_time += current_time - process->last_execution;
            }
        } else {
            current_time++;
        }
    }
}

void multilevelFeedback(Process processos[], int n) {
    int current_time = 0;
    int completed_processes = 0;

    while (completed_processes < n) {
        int selected_process = -1;

        for (int i = 0; i < n; i++) {
            if (!processos[i].completed && processos[i].creation_time <= current_time) {
                selected_process = i;
                break;
            }
        }

        if (selected_process != -1) {
            Process *process = &processos[selected_process];
            int priority = process->priority;

            if (process->duration <= clock_tick) {
                // Executa o processo até o fim
                current_time += process->duration;
                process->duration = 0;
                process->completed = true;
                completed_processes++;
            } else {
                // Executa o processo por um quantum (clock_tick)
                current_time += clock_tick;
                process->duration -= clock_tick;
            }

            // Ajusta a prioridade do processo de acordo com o nível de feedback
            if (priority > 1 && process->duration > 0) {
                process->priority--;
            }

            // Atualiza o tempo de espera dos outros processos
            for (int i = 0; i < n; i++) {
                if (!processos[i].completed && i != selected_process) {
                    processos[i].waiting_time += clock_tick;
                }
            }
        } else {
            current_time++;
        }
    }
}
