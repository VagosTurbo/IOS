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
sem_t *customer_ready;
sem_t *employee_ready;
sem_t *customer_done;
sem_t *employee_done;
sem_t *semafor;
sem_t *semafor2;
sem_t *semafor3;
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

    // initialize semaphores
    mutex = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    customer_ready = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    employee_ready = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    customer_done = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    employee_done = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    action_counter = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    post_office = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    semafor = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    semafor2 = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    semafor3 = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    queue_size = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    queue_size2 = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    queue_size3 = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    if (mutex == MAP_FAILED || customer_ready == MAP_FAILED || employee_ready == MAP_FAILED || customer_done == MAP_FAILED || employee_done == MAP_FAILED || action_counter == MAP_FAILED || post_office == MAP_FAILED || semafor == MAP_FAILED || semafor2 == MAP_FAILED || semafor3 == MAP_FAILED || people_inside == MAP_FAILED){
        fprintf(stderr, "Error creating shared memory\n");
        exit(EXIT_FAILURE);
    }

    if ( sem_init(mutex, 1, 1) == -1 || sem_init(customer_ready, 1, 0) == -1 ||  sem_init(employee_ready, 1, 0) == -1 || sem_init(customer_done, 1, 0) == -1 || sem_init(employee_done, 1, 0) == -1 || sem_init(semafor, 1, 0) == -1 || sem_init(semafor2, 1, 0) == -1 || sem_init(semafor, 1, 0) == -1){
        fprintf(stderr, "Error creating semaphore\n");
        exit(EXIT_FAILURE);
    }
    setbuf(file, NULL);
}

void cleanup(){

    // close semaphores
    sem_destroy(mutex);
    sem_destroy(customer_ready);
    sem_destroy(employee_ready);
    sem_destroy(customer_done);
    sem_destroy(employee_done);
    sem_destroy(semafor);
    sem_destroy(semafor2);
    sem_destroy(semafor3);

    // close shared memory
    munmap(mutex, sizeof(sem_t));
    munmap(customer_ready, sizeof(sem_t));
    munmap(employee_ready, sizeof(sem_t));
    munmap(customer_done, sizeof(sem_t));
    munmap(employee_done, sizeof(sem_t));
    munmap(action_counter, sizeof(int));
    munmap(post_office, sizeof(int));
    munmap(semafor, sizeof(sem_t));
    munmap(semafor2, sizeof(sem_t));
    munmap(semafor3, sizeof(sem_t));

    // close file
    if(file != NULL){
        fclose(file);
    }
}


void customer_process(Customer customer, int CustomerCount){

    (void) CustomerCount;

    sem_wait(mutex);
    customer.CustomerDemand = rand() % 3;
    fprintf(file, "%d: Z %d: started\n", ++(*action_counter), customer.CustomerID);

    if (*post_office){
        fprintf(file, "%d: Z %d: Odchadza posta je zavreta \n", ++(*action_counter), customer.CustomerID);
        exit(EXIT_SUCCESS);
    }
    fprintf(file, "%d: Z %d: entering office for a service %d\n", ++(*action_counter), customer.CustomerID, customer.CustomerDemand);

    sem_post(mutex);
    sem_post(customer_ready);
    if (customer.CustomerDemand == 0){
        (*queue_size)++;
        sem_wait(semafor);
    }
    else if (customer.CustomerDemand == 1){
        (*queue_size2)++;
        sem_wait(semafor2);
    }
    else if (customer.CustomerDemand == 2){
        (*queue_size3)++;
        sem_wait(semafor3);
    }

    sem_wait(mutex);
    fprintf(file, "%d: Z %d: called by office worker\n", ++(*action_counter), customer.CustomerID);
    sem_post(mutex);
    
    sem_post(customer_done);
    sem_wait(employee_done);
    fprintf(file, "%d: Z %d: going home\n", ++(*action_counter), customer.CustomerID);
    exit(EXIT_SUCCESS);
}

void employee_process(int EmployeeID, int breaktime, int EmployeeCount){

    while(true){


        if ( *queue_size == 0 && *queue_size2 == 0 && *queue_size3 == 0 && *action_counter >= EmployeeCount+1){
            fprintf(file, "%d: U %d: taking break\n", ++(*action_counter), EmployeeID);
            sleep(breaktime);
        }

        sem_wait(customer_ready);
        sem_wait(mutex);
        int rand_queue;
        while(true){
            rand_queue = rand() % 3;
            if (rand_queue == 0 && *queue_size > 0){
                (*queue_size)--;
                break;
            }
            else if (rand_queue == 1 && *queue_size2 > 0){
                (*queue_size2)--;
                break;
            }
            else if (rand_queue == 2 && *queue_size3 > 0){
                (*queue_size3)--;
                break;
            }
            if (*queue_size == 0 && *queue_size2 == 0 && *queue_size3 == 0 && *post_office == 0){
                fprintf(file, "%d: U %d: going home\n", ++(*action_counter), EmployeeID);
            }
        }
        sem_post(mutex);
        
        if (rand_queue == 0){
            fprintf(file, "%d: U %d: pracuje na 0\n", ++(*action_counter), EmployeeID);
            sem_post(semafor);
        }
        else if (rand_queue == 1){
            fprintf(file, "%d: U %d: pracuje na 1\n", ++(*action_counter), EmployeeID);
            sem_post(semafor2);
        }
        else if (rand_queue == 2){
            fprintf(file, "%d: U %d: pracuje na 2\n", ++(*action_counter), EmployeeID);
            sem_post(semafor3);
        }
        sem_wait(customer_done);
        sem_post(employee_done);
    }
    fprintf(file, "%d: KOKOOOOOOOOOOOOOOOOOOOOOT\n", ++(*action_counter));
    exit(EXIT_SUCCESS);
    (void) breaktime;
}



int main(int argc, char *argv[]) {

    srand(time(NULL));
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
        for (int i = 0; i < EmployeeCount; i++){
            sem_post(customer_ready);
        }
        exit(EXIT_SUCCESS);
    }

    for (int i = 0; i < EmployeeCount; i++){
        pid_t pid_e_id = fork();
        if (pid_e_id == 0){
            int breaktime = rand() % EmployeeMaxBreak;
            if ((*action_counter) <= EmployeeCount+1 ){
                fprintf(file, "%d: U %d: started\n", ++(*action_counter), i+1);
            }
            employee_process(i+1, breaktime, EmployeeCount);
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
            customer_process(customer, CustomerCount);
            exit(EXIT_SUCCESS);
        }
    }

    cleanup();
    while( wait(NULL) > 0);
    return EXIT_SUCCESS;
}