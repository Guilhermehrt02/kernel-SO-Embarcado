#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#define MAX_PROCESSES 10 // o input deve ter o dobro
#define QUANTUM 1

// Enum para erros no retorno de funcoes
typedef enum
{
    OK,
    ERROR,
    ERROR_READ,
    ERROR_WRITE
} error_list;


typedef struct
{
    int creation_time;
    int duration;
    int priority;
    int waiting_time;
    int last_execution;
    bool completed;
} Process;


typedef void (*ptrFunc)(void);
typedef int (*scheduleFunc)(Process*, int);

Process processes[MAX_PROCESSES]; // buffer circular de processos para o SRTN
Process waiting_list[MAX_PROCESSES]; // buffer circular de espera para o SRTN
Process feedback_proc[MAX_PROCESSES]; // buffer circular de processos para Filas multiplas com realimentacao
Process feedback_waiting[MAX_PROCESSES]; // buffer circular de espera para Filas multiplas com realimentacao
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

typedef struct
{
    int inicio;
    int fim;
    int n;
} OutputProc;

OutputProc outproc[MAX_PROCESSES * 2];
FILE *out_file;
bool first = true;
bool first2 = true;

// functions
void addProc(int creation_time, int duration, int priority); // adiciona processos
void removeProc(); // remove processos
int srtBatch(Process processos[], int n);// Escalonador Batch: Shortest Remaining-Time Next
void kernelInit(void); // inicializador do kernel
void kernel_loop(scheduleFunc func1, scheduleFunc func2);
char kernel_exec(scheduleFunc func);
int multilevelFeedback(Process* processes, int n);
void bubble_sort(Process *vet);

Process getShortestRemainingTimeProcess();
void executeProcess(Process *process);
void scheduleProcesses(int num_processes);

// Funcoes para lidar com arquivo
void print_process_list(Process *vet);
int open_and_read_file(FILE *f, Process *vet, Process *waiting);

// Main
int main(int argc, char **argv)
{
    kernelInit();

    // multilevelFeedback(processes, MAX_PROCESSES);

    kernel_loop(srtBatch, multilevelFeedback);
    // kernel_loop(multilevelFeedback);
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

int srtBatch(Process processos[], int n)
{
    if(first == true)
    {
        start = 0;
        end = 0;
        clock_tick = 0;
    }

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
    int quantum = 3;

    // out_file = fopen("output.txt", "w");

    // // Escrever no arquivo a tabela abaixo
    // fprintf(out_file,"========== LIST OF PROCESSES ==========\n");
    // fprintf(out_file,"ID Creation Time    duration  Priority\n");
    // for(int i = 0; i < MAX_PROCESSES; i++)
    //     fprintf(out_file,"%2d      %2d           %2d       %2d\n", i, processos[i].creation_time, processos[i].duration,
    //         processos[i].priority);

    // Diagrama do tempo de execucao
    // fprintf(out_file,"\n\n========== RUNTIME DIAGRAM ==========");
    // fprintf(out_file,"\ntempo  P0  P1  P2  P3  P4  P5  P6  P7  P8  P9   P10  P11  P12  P13  P14  P15  P16  P17  P18  P19\n");

    while(concluidos < n)
    {
        int menor_tempo_restante = tempo_total + 1;
        int proximo_processo = -1;

        // Encontrar o próximo processo a ser executado
        for (i = 0; i < n; i++)
        {
            if (!processos[i].completed && processos[i].creation_time <= clock_tick &&
                (processos[i].duration - processos[i].last_execution) < menor_tempo_restante)
            {
                menor_tempo_restante = processos[i].duration - processos[i].last_execution;
                proximo_processo = i;
                printf("Proximo processo: %d\n", i);
            }
        }

        if (proximo_processo == -1)
        {
            clock_tick++;
            printf("tempo atual = %d\n", clock_tick);
            continue;
        }

        // Executar o próximo processo por um quantum de tempo
        for (i = 0; i < quantum; i++)
        {
            if (processos[proximo_processo].last_execution < processos[proximo_processo].duration)
            {
                processos[proximo_processo].last_execution++;
                int aux = clock_tick++;
                if(first == true)
                    fprintf(out_file,"%2d-%2d: %s\n", aux, clock_tick, l[proximo_processo]);
                else
                    fprintf(out_file,"%2d-%2d: %s\n", aux, clock_tick, l[proximo_processo + 10]);
            
                if(processes[proximo_processo].last_execution >= processos[proximo_processo].duration)
                {
                    printf("Tempo %d: Processo %d concluído\n", clock_tick, proximo_processo);
                    processos[proximo_processo].completed = true;
                    concluidos++;
                    break;
                }
            }
            else
            {
                break;
            }

            if (processos[proximo_processo].last_execution == processos[proximo_processo].duration)
            {
                printf("Tempo %d: Processo %d concluído\n", clock_tick, proximo_processo);
                processos[proximo_processo].completed = true;
                concluidos++;
            }
        }
    }

    first = !first;
    return OK;

    printf("Tempo total de execução: %d unidades de tempo\n", tempo_total);
}

int multilevelFeedback(Process* processes, int n)
{        
    if(first2 == true)
        fprintf(out_file, "\n\nLista de processos:\n");
    else
        fprintf(out_file, "\n\nLista de espera:\n");

    clock_tick = 0;
    start = 0;
    end = 0;
    Process second_queue[MAX_PROCESSES];

    // for(int i = 0; i < MAX_PROCESSES; i++)
    //     second_queue[i].completed = false;
    
    Process third_queue[MAX_PROCESSES];
    // int start[3] = {start, 0, 0};
    int third_start = 0;
    int third_end = 0;

    for(int i = 0; i < MAX_PROCESSES; i++)
        third_queue[i].completed = false;

    int second_start = 0;
    int second_end = 0;
    int first_quantum = QUANTUM;
    int second_quantum = QUANTUM * 2;
    int current_process=start;

    int current_time = 0;
    int completed_processes = 0;

    int current_state = 0;

    Process *process = &processes[current_process];

    int aux2 = clock_tick;
    while((start+1)%MAX_PROCESSES != end)
    {
        process = &processes[start];
        aux2 = clock_tick;
        if(process->duration >= first_quantum)
        {
            clock_tick += first_quantum;
            process->duration -= first_quantum;
            if(process->duration > 0)
            {
                printf("[FIRST_QUEUE]Processo %d executando[faltam %d ticks] em clock(%d)\n", start, process->duration, clock_tick);
                if (((second_end + 1) % MAX_PROCESSES) != second_start) // verifica se esta cheio
                {
                    printf("[FIRST_QUEUE]Adicionando processo %d na segunda lista na posicao %d\n", start, second_end);
                    second_queue[second_end] = *process;
                    second_end = (second_end + 1) % MAX_PROCESSES; // atualiza a ultima posicao de elementos
                    start = (start + 1) % MAX_PROCESSES;
                }
            }
            else
            {
                process->completed = true;
                fprintf(out_file, "[FIRST_QUEUE]Processo %d completado em clock(%d)\n", start, clock_tick);
                start = (start + 1) % MAX_PROCESSES;
            }
        }
        else
        {
            int dif = first_quantum - process->duration;
            process->duration = 0;
            process->completed = true;
            clock_tick += dif;
            fprintf(out_file, "[FIRST_QUEUE]Processo %d completado em clock(%d)\n", start, clock_tick);
            start = (start + 1) % MAX_PROCESSES;
        }

        printf("\n");
    }

    printf("Comecando SECOND_QUEUE\n\n");
    print_process_list(second_queue);

    current_process = start;
    process = &second_queue[second_start];
    while((second_start+1)%MAX_PROCESSES <= second_end)
    {
        process = &second_queue[second_start];
        aux2 = clock_tick;
        if(process->duration >= second_quantum)
        {
            clock_tick += second_quantum;
            process->duration -= second_quantum;
            if(process->duration > 0)
            {
                printf("[SECOND_QUEUE] Processo %d executando[faltam %d ticks] em clock(%d)\n", second_start, process->duration, clock_tick);
                if (((second_end + 1) % MAX_PROCESSES) != second_start) // verifica se esta cheio
                {
                    printf("[SECOND_QUEUE] Adicionando processo %d na terceira lista na posicao %d\n", second_start, third_end);
                    third_queue[third_end] = *process;
                    third_end = (third_end + 1) % MAX_PROCESSES; // atualiza a ultima posicao de elementos
                    second_start = (second_start + 1) % MAX_PROCESSES; // anda na fila 2
                }
            }
            else
            {
                process->completed = true;
                fprintf(out_file, "[SECOND_QUEUE] Processo %d completado em clock(%d)\n", second_start, clock_tick);
                second_start = (second_start + 1) % MAX_PROCESSES; // anda na fila 2
            }
        }
        else
        {
            // printf("[SECOND_QUEUE] Processo %d (duration = %d)\n", second_start, process->duration);
            int dif = second_quantum - process->duration;
            process->duration = 0;
            process->completed = true;
            clock_tick += dif;
            fprintf(out_file, "[SECOND_QUEUE] Processo %d completado em clock(%d)\n", second_start, clock_tick);
            second_start = (second_start + 1) % MAX_PROCESSES;
        }
    }

    current_process += second_start;
    printf("Comecando THIRD_QUEUE\n\n");
    print_process_list(third_queue);
    printf("third_start = %d e third_end = %d\n", third_start, third_end);
    int aux = third_start;
    while((third_start+1)%MAX_PROCESSES <= third_end)
    {
        aux2 = clock_tick;
        clock_tick += third_queue[third_start].duration;
        third_queue[third_start].duration = 0;
        third_queue[third_start].completed = true;
        fprintf(out_file, "[THIRD_QUEUE] Processo %d completado em clock(%d)\n", third_start, clock_tick);
        third_start = (third_start + 1) % MAX_PROCESSES;
    }

    first2 = !first2;
}

void kernelInit(void)
{
    start = 0;
    end = 0;
    clock_tick = 0;

    error_list result = OK;
    FILE *file;

    // Ler o arquivo input.txt
    open_and_read_file(file, processes, waiting_list);
    
    printf("Lista principal:\n");
    print_process_list(processes);
    printf("\n");

    printf("Lista de espera:\n");
    print_process_list(waiting_list);
}

void kernelAddProc(ptrFunc newFunc)
{
    
}

void kernel_loop(scheduleFunc func1, scheduleFunc func2)
{
    int r1 = ERROR;
    int r2 = ERROR;
    int r3 = ERROR;
    int r4 = ERROR;
    
    // for(;;)
    // {
        if(r1 != OK)
            r1 = func1(processes, MAX_PROCESSES);
        
        if(r2 != OK)
            r2 = func1(waiting_list, MAX_PROCESSES);
        
        if(r3 != OK)
            r3 = func2(feedback_proc, MAX_PROCESSES);
        
        if(r4 != OK)
            r4 = func2(feedback_waiting, MAX_PROCESSES);
        // printf("No more processes.\n");
    // }

    fclose(out_file);
}

// Ordenar processos
void bubble_sort(Process *vet)
{
    Process aux;
    for(int i = 0; i < 20; i++)
    {
        for(int j = 0; j < 20-1; j++)
        {
            if(vet[j].creation_time > vet[j+1].creation_time)
            {
                aux = vet[j];
                vet[j] = vet[j+1];
                vet[j+1] = aux;
            } 
        }
    }
}

int open_and_read_file(FILE *f, Process *vet, Process *waiting)
{
    f = fopen("input.txt", "r");
    
    if(f == NULL)
        return ERROR_READ;

    int v;

    Process aux[20];
    for(int i = 0; i < 20; i++)
    {
        v = fscanf(f, "%d %d %d", &aux[i].creation_time, &aux[i].duration,
            &aux[i].priority);
        aux[i].completed = false;
        aux[i].last_execution = aux[i].creation_time;
        aux[i].waiting_time = 0;
    }

    bubble_sort(aux);

    for(int i = 0; i < MAX_PROCESSES; i++)
    {
        vet[i] = aux[i];
        feedback_proc[i] = aux[i];
    }

    int a = 0;
    for(int i = 10; i < MAX_PROCESSES * 2; i++)
    {
        waiting_list[a] = aux[i];
        feedback_waiting[a++] = aux[i];
    }

    out_file = fopen("output.txt", "w");

    // Escrever no arquivo a tabela abaixo
    fprintf(out_file,"========== LIST OF PROCESSES ==========\n");
    fprintf(out_file,"ID Creation Time    duration  Priority\n");
    for(int i = 0; i < MAX_PROCESSES * 2; i++)
        fprintf(out_file,"%2d      %2d           %2d       %2d\n", i, aux[i].creation_time, aux[i].duration,
            aux[i].priority);

    // Diagrama do tempo de execucao
    fprintf(out_file,"\n\n========== RUNTIME DIAGRAM (SRTN) ==========");
    fprintf(out_file,"\ntempo  P0  P1  P2  P3  P4  P5  P6  P7  P8  P9   P10  P11  P12  P13  P14  P15  P16  P17  P18  P19\n");


    // print_process_list(waiting_list);

    if(f) fclose(f);
    return OK;
}

void print_process_list(Process *vet)
{
    printf("========== LIST OF PROCESSES ==========\n");
    printf("ID Creation Time    duration  Priority\n");
    for(int i = 0; i < MAX_PROCESSES; i++)
        printf("%d      %2d           %2d       %2d\n", i, vet[i].creation_time, vet[i].duration,
            vet[i].priority);
}

bool isBufferFull()
{
    return (end + 1) % MAX_PROCESSES == start;
}

bool isBufferEmpty()
{
    return end == -1;
}

void enqueue(Process process)
{
    if (!isBufferFull())
    {
        end = (end + 1) % MAX_PROCESSES;
        processes[end] = process;
        if (start == -1)
        {
            start = 0;
        }
    }
}

Process dequeue()
{
    Process process;
    if (!isBufferEmpty())
    {
        process = processes[start];
        if (start == end)
        {
            start = -1;
            end = -1;
        }
        else
        {
            start = (start + 1) % MAX_PROCESSES;
        }
    }
    return process;
}

Process getShortestRemainingTimeProcess()
{
    Process shortest_process = processes[start];
    int shortest_time = shortest_process.duration - shortest_process.last_execution;

    int i;
    for (i = 1; i < MAX_PROCESSES; i++)
    {
        int index = (start + i) % MAX_PROCESSES;
        Process current_process = processes[index];
        int remaining_time = current_process.duration - current_process.last_execution;

        if (remaining_time < shortest_time)
        {
            shortest_process = current_process;
            shortest_time = remaining_time;
        }
    }

    return shortest_process;
}

void executeProcess(Process *process)
{
    if (process->last_execution + clock_tick >= process->duration)
    {
        process->completed = true;
        process->waiting_time += process->last_execution - process->creation_time;
        process->last_execution = process->duration;
    }
    else
    {
        process->last_execution += clock_tick;
    }
}

void initializeBuffer()
{
    start = 0;
    end = -1;
}

void scheduleProcesses(int num_processes)
{
    initializeBuffer();

    int current_time = 0;
    int completed_processes = 0;

    while (completed_processes < num_processes)
    {
        // Enqueue newly arrived processes
        int i;
        for (i = 0; i < num_processes; i++)
        {
            if (waiting_list[i].creation_time == current_time)
            {
                enqueue(waiting_list[i]);
            }
        }

        // Get the process with the shortest remaining time
        Process shortest_process = getShortestRemainingTimeProcess();

        // Execute the process
        executeProcess(&shortest_process);

        // Check if the process is completed
        if (shortest_process.completed)
        {
            completed_processes++;
        }
        else
        {
            // Move the process to the end of the buffer
            dequeue();
            enqueue(shortest_process);
        }

        current_time += clock_tick;
    }
}