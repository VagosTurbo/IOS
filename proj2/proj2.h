// @author: xseman06
// @date: 2023-04-20
// @project: Projekt 2

#ifndef PROJ2_H
#define PROJ2_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <unistd.h>
#include <semaphore.h>
#include <string.h>
#include <fcntl.h>
#include <stdarg.h>
#include <limits.h>
#include <stdbool.h>
#include <time.h>

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

sem_t *mutex;
sem_t *customer_done;
sem_t *employee_done;
sem_t *queue;
sem_t *queue2;
sem_t *queue3;
sem_t *queue_done;
sem_t *queue2_done;
sem_t *queue3_done;

FILE *file;
int *action_counter;
int *post_office;
int *queue_size;
int *queue_size2;
int *queue_size3;

typedef struct{
    int CustomerID;
    int CustomerWaitTime;
    int CustomerDemand;
} Customer; 

// Functions
void help();
int argcheck(int argc, char *argv[]);
void startup();
void cleanup();
void customer_process(Customer customer);
void employee_process(int EmployeeID, int breaktime);


#endif // PROJ2_H
