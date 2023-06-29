#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#define MAX_PROCESSES 10 //o input deve ter o dobro

// Enum para erros no retorno de funcoes
typedef enum
{
    OK,
    ERROR_READ,
    ERROR_WRITE
} error_list;

typedef void(*ptrFunc)(void);

typedef struct {
    int creation_time;
    int duration;
    int priority;
    int waiting_time;
    int last_execution;
    bool completed; 
    //ptrFunc function; nao sei se precisa
} Process;

Process processes[MAX_PROCESSES];//buffer circular
int start=0, end=0;

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

//functions
void addProc(int creation_time, int duration, int priority);// adiciona processos
void removeProc (); //remove processos
// Escalonador Batch: Shortest Remaining-Time Next
// Escalonador Interativo: Múltiplas filas com realimentação

// Main
int main(int argc, char **argv)
{
    // Definindo variaveis
    error_list result = OK;
    FILE *file;

    // Ler o arquivo input.txt
    if(result != 0) return result;

    
    return result;
}

void addProc(int creation_time, int duration, int priority){
    if( ((end+1)%MAX_PROCESSES) != start )
    {
        processes[end].creation_time = creation_time;
        processes[end].duration = duration;
        processes[end].priority = priority;

        end = (end+1)%MAX_PROCESSES;
    }
}

void removeProc (){
    if(start != end){
        start = (start+1)%MAX_PROCESSES;
    }
}