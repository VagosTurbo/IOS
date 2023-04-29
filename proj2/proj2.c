/*
* @author: xseman06
* @date: 2023-04-20
* @project: Projekt 2
*/

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
sem_t *queue_done;
sem_t *queue2_done;
sem_t *queue3_done;
sem_t *customer_done;
sem_t *employee_done;
sem_t *queue;
sem_t *queue2;
sem_t *queue3;
sem_t *queue1;
FILE *file;
int *action_counter;
int *post_office;
int *people_inside;
int *queue_size;
int *queue_size2;
int *queue_size3;

typedef struct{
    int CustomerID;
    int CustomerWaitTime;
    int CustomerDemand;
} Customer; 

// prints help how to use the program
void help(){
    printf("Usage: ./proj2 NZ NU TZ TU F\n");
    printf("NZ: počet zákazníků\n");
    printf("NU: počet úředníků\n");
    printf("TZ: Maximální čas v milisekundách, po který zákazník po svém vytvoření čeká, než vejde na poštu. 0<=TZ<=10000\n");
    printf("TU: Maximální délka přestávky úředníka v milisekundách. 0<=TU<=100\n");
    printf("F: Maximální čas v milisekundách, po kterém je uzavřena pošta pro nově příchozí. 0<=F<=10000\n");
}

// checks if the number of arguments is correct
int argcheck(int argc, char *argv[]) {

    int NZ = atoi(argv[1]);
    int NU = atoi(argv[2]);
    int TZ = atoi(argv[3]);
    int TU = atoi(argv[4]);
    int F = atoi(argv[5]);

    if (argc != 6) {
        help();
        fprintf(stderr, "Incorrect number of arguments \n");
        return EXIT_FAILURE;
    }

    // checks if NZ or NU is in range
    if (NZ < 0 || NU < 0){
        fprintf(stderr, "Incorrect format of arguments (NZ/NU)\n");
        return EXIT_FAILURE;
    }

    // checks if TZ or F is in range 0<=TZ/F<=10000
    if ( TZ < 0 || TZ > 10000 || F < 0 || F > 10000 ){
        fprintf(stderr, "Incorrect format of arguments (TZ/F)\n");
        return EXIT_FAILURE;
    }
    
    // checks if TU is in range 0<=TZ/F<=100
    if ( TU < 0 || TU > 100){
        fprintf(stderr, "Incorrect format of arguments (TU)\n");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

void startup(){

    // open file
    file = fopen("proj2.out", "w");
    if (file == NULL){
        fprintf(stderr, "Error opening file %s\n", "proj2.out");
        exit(EXIT_FAILURE);
    }

    // initialize shared variables
    mutex = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    customer_done = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    employee_done = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    action_counter = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    post_office = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    queue = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    queue2 = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    queue3 = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    queue_done = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    queue2_done = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    queue3_done = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    queue_size = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    queue_size2 = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    queue_size3 = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    queue1 = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    //TODO DOPLNIT
    if (mutex == MAP_FAILED ||  customer_done == MAP_FAILED || employee_done == MAP_FAILED || action_counter == MAP_FAILED || post_office == MAP_FAILED || queue == MAP_FAILED || queue2 == MAP_FAILED || queue3 == MAP_FAILED || queue_done == MAP_FAILED || queue2_done == MAP_FAILED || queue3_done == MAP_FAILED || queue_size == MAP_FAILED || queue_size2 == MAP_FAILED || queue_size3 == MAP_FAILED || queue1 == MAP_FAILED){
        fprintf(stderr, "Error creating shared memory\n");
        exit(EXIT_FAILURE);
    }


    // initialize semaphores
    if ( 
           sem_init(mutex, 1, 1) == -1 
        || sem_init(queue_done, 1, 0) == -1
        || sem_init(queue2_done, 1, 0) == -1
        || sem_init(queue3_done, 1, 0) == -1
        || sem_init(customer_done, 1, 0) == -1 
        || sem_init(employee_done, 1, 0) == -1 
        || sem_init(queue, 1, 0) == -1 
        || sem_init(queue2, 1, 0) == -1 
        || sem_init(queue, 1, 0) == -1
        ){
        fprintf(stderr, "Error creating semaphore\n");
        exit(EXIT_FAILURE);
    }
    setbuf(file, NULL);
}

void cleanup(){

    // close semaphores
    sem_destroy(mutex);
    sem_destroy(queue_done);
    sem_destroy(queue2_done);
    sem_destroy(queue3_done);
    sem_destroy(customer_done);
    sem_destroy(employee_done);
    sem_destroy(queue);
    sem_destroy(queue2);
    sem_destroy(queue3);

    // close shared memory
    munmap(mutex, sizeof(sem_t));
    munmap(queue_done, sizeof(sem_t));
    munmap(queue2_done, sizeof(sem_t));
    munmap(queue3_done, sizeof(sem_t));
    munmap(customer_done, sizeof(sem_t));
    munmap(employee_done, sizeof(sem_t));
    munmap(action_counter, sizeof(int));
    munmap(post_office, sizeof(int));
    munmap(queue, sizeof(sem_t));
    munmap(queue2, sizeof(sem_t));
    munmap(queue3, sizeof(sem_t));

    // close file
    if(file != NULL){
        fclose(file);
    }
}


void customer_process(Customer customer){
    srand(getpid());

    sem_wait(mutex);
    customer.CustomerDemand = (rand() % 3)+1;
    fprintf(file, "%d: Z %d: started\n", ++(*action_counter), customer.CustomerID);
    sem_post(mutex);

    // waits infront of post office
    sleep(rand() % customer.CustomerWaitTime);
    
    // enters post office
    sem_wait(mutex);
    if (*post_office == 1){
        fprintf(file, "%d: Z %d: going home\n", ++(*action_counter), customer.CustomerID);
        sem_post(mutex);
        exit(EXIT_SUCCESS);
    }
    fprintf(file, "%d: Z %d: entering office for a service %d\n", ++(*action_counter), customer.CustomerID, customer.CustomerDemand);
    sem_post(mutex);

    if (customer.CustomerDemand == 1){
        (*queue_size)++;
        sem_wait(queue);
        sem_wait(mutex);
        fprintf(file, "%d: Z %d: called by office worker\n", ++(*action_counter), customer.CustomerID);
        sem_post(queue_done);
        sem_post(mutex);
    }
    else if (customer.CustomerDemand == 2){
        (*queue_size2)++;
        sem_wait(queue2);
        sem_wait(mutex);
        fprintf(file, "%d: Z %d: called by office worker\n", ++(*action_counter), customer.CustomerID);
        sem_post(queue2_done);
        sem_post(mutex);

    }
    else if (customer.CustomerDemand == 3){
        (*queue_size3)++;
        sem_wait(queue3);
        sem_wait(mutex);
        fprintf(file, "%d: Z %d: called by office worker\n", ++(*action_counter), customer.CustomerID);
        sem_post(queue3_done);
        sem_post(mutex);
    }
    else{
        exit(EXIT_FAILURE);
    }
    sem_wait(employee_done);
    sem_wait(mutex);
    fprintf(file, "%d: Z %d: going home\n", ++(*action_counter), customer.CustomerID);
    sem_post(mutex);
    exit(EXIT_SUCCESS);

}

void employee_process(int EmployeeID, int breaktime){
    srand(getpid());

    sem_wait(mutex);
    fprintf(file, "%d: U %d: started\n", ++(*action_counter), EmployeeID);
    sem_post(mutex);

    while(true){
        int random = (rand() % 3) + 1;
        printf("U %d: posta %d, queue %d, queue2 %d, queue3 %d\n", EmployeeID, *post_office, *queue_size, *queue_size2, *queue_size3);
        if(random == 1 && *queue_size > 0){
            sem_post(queue);
            (*queue_size)--;
            sem_wait(queue_done);
            sem_wait(mutex);
            fprintf(file, "%d: U %d: serving service 1\n", ++(*action_counter), EmployeeID);
            sem_post(mutex);
            sleep(rand() % 10);
            sem_wait(mutex);
            fprintf(file, "%d: U %d: service finished\n", ++(*action_counter), EmployeeID);
            sem_post(employee_done);
            sem_post(mutex);
        }
        else if(random == 2 && *queue_size2 > 0){
            sem_post(queue2);
            (*queue_size2)--;
            sem_wait(queue2_done);
            sem_wait(mutex);
            fprintf(file, "%d: U %d: serving service 2\n", ++(*action_counter), EmployeeID);
            sem_post(mutex);
            sleep(rand() % 10);
            sem_wait(mutex);
            fprintf(file, "%d: U %d: service finished\n", ++(*action_counter), EmployeeID);
            sem_post(employee_done);
            sem_post(mutex);
        }
        else if(random == 3 && *queue_size3 > 0){
            sem_post(queue3);
            (*queue_size3)--;
            sem_wait(queue3_done);
            sem_wait(mutex);
            fprintf(file, "%d: U %d: serving service 3\n", ++(*action_counter), EmployeeID);
            sem_post(mutex);
            sleep(rand() % 10);
            sem_wait(mutex);
            fprintf(file, "%d: U %d: service finished\n", ++(*action_counter), EmployeeID);
            sem_post(employee_done);
            sem_post(mutex);
        }
        else if( *post_office == 1 && *queue_size == 0 && *queue_size2 == 0 && *queue_size3 == 0){
            sem_wait(mutex);
            fprintf(file, "%d: U %d: going home\n", ++(*action_counter), EmployeeID);
            sem_post(mutex);
            exit(EXIT_SUCCESS);
        }
        else if ( *post_office == 0 && *queue_size == 0 && *queue_size2 == 0 && *queue_size3 == 0){
            sem_wait(mutex);
            fprintf(file, "%d: U %d: taking break\n", ++(*action_counter), EmployeeID);
            sem_post(mutex);
            sleep(rand() % breaktime);
            fprintf(file, "%d: U %d: break finished\n", ++(*action_counter), EmployeeID);
        }
    }
}



int main(int argc, char *argv[]) {

    
    // checks if arguments are in correct format
    if (argcheck(argc, argv) == EXIT_FAILURE){
        return EXIT_FAILURE;
    }
    int CustomerCount = atoi(argv[1]);
    int EmployeeCount = atoi(argv[2]);
    int CustomerMaxWait = atoi(argv[3]);
    int EmployeeMaxBreak = atoi(argv[4]);
    int MaxTime = atoi(argv[5]);

    startup();


    pid_t pid_post_office = fork();
    if (pid_post_office == 0){
        int kokot = rand() % MaxTime;
        sleep(kokot);
        fprintf(file, "%d: closing\n", ++(*action_counter));
        (*post_office) = 1;
        exit(EXIT_SUCCESS);
    }

    for (int i = 0; i < EmployeeCount; i++){
        pid_t pid_e_id = fork();
        if (pid_e_id == 0){
            int breaktime = rand() % EmployeeMaxBreak;
            employee_process(i+1, breaktime);
            exit(EXIT_SUCCESS);
        }
    }

    for (int i = 0; i < CustomerCount; i++){
        pid_t pid_c_id = fork();
        if (pid_c_id == 0){
            Customer customer;
            customer.CustomerID = i+1;
            customer.CustomerWaitTime = CustomerMaxWait;
            customer.CustomerDemand = rand() % 3;
            customer_process(customer);
            exit(EXIT_SUCCESS);
        }
    }
    cleanup();
    while( wait(NULL) > 0);
    return EXIT_SUCCESS;
}