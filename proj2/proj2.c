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
FILE *file;
int *action_counter;


typedef struct{
    int CustomerID;
    int CustomerWaitTime;
    int CustomerDemand;
} Customer; 


bool post_office = true;


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

// writes arguments to file proj2.out
int write_to_file(int CustomerCount, int EmployeeCount, int CustomerMaxWait, int EmployeeMaxBreak, int MaxTime){
    file = fopen("proj2.out", "w");
    if (file == NULL){
        fprintf(stderr, "Error opening file %s\n", "proj2.out");
        return EXIT_FAILURE;
    }
    
    fprintf(file, "NZ: %d\n", CustomerCount);
    fprintf(file, "NU: %d\n", EmployeeCount);
    fprintf(file, "TZ: %d\n", CustomerMaxWait);
    fprintf(file, "TU: %d\n", EmployeeMaxBreak);
    fprintf(file, "F: %d\n", MaxTime);

    fclose(file);
    return EXIT_SUCCESS;
}

void startup(){

    file = fopen("proj2.out", "w");
    if (file == NULL){
        fprintf(stderr, "Error opening file %s\n", "proj2.out");
        exit(EXIT_FAILURE);
    }

    mutex = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    customer_ready = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    employee_ready = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    customer_done = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    employee_done = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    action_counter = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);



    

    if (mutex == MAP_FAILED || customer_ready == MAP_FAILED || employee_ready == MAP_FAILED || customer_done == MAP_FAILED || employee_done == MAP_FAILED || action_counter == MAP_FAILED){
        fprintf(stderr, "Error creating shared memory\n");
        exit(EXIT_FAILURE);
    }

    if ( sem_init(mutex, 1, 1) == -1 || sem_init(customer_ready, 1, 0) == -1 ||  sem_init(employee_ready, 1, 0) == -1 || sem_init(customer_done, 1, 0) == -1 || sem_init(employee_done, 1, 0) == -1){
        fprintf(stderr, "Error creating semaphore\n");
        exit(EXIT_FAILURE);
    }
    setbuf(file, NULL);
}

void cleanup(){

    sem_destroy(mutex);
    sem_destroy(customer_ready);
    sem_destroy(employee_ready);
    sem_destroy(customer_done);
    sem_destroy(employee_done);

    munmap(mutex, sizeof(sem_t));
    munmap(customer_ready, sizeof(sem_t));
    munmap(employee_ready, sizeof(sem_t));
    munmap(customer_done, sizeof(sem_t));
    munmap(employee_done, sizeof(sem_t));
    munmap(action_counter, sizeof(int));


    if(file != NULL){
        fclose(file);
    }
}

void customer_func(Customer customer, int CustomerCount){

    (void) CustomerCount;

    int kokot = rand() % customer.CustomerWaitTime;
    fprintf(file, "%d: Z %d: started with waittme %d\n", ++(*action_counter), customer.CustomerID, kokot);

    if (post_office == false){
        fprintf(file, "%d: Z %d: Odchadza posta je zavreta \n", ++(*action_counter), customer.CustomerID);
        exit(EXIT_SUCCESS);
    }

    sem_post(customer_ready);
    sem_wait(employee_ready);
    sem_wait(mutex);
    fprintf(file, "%d: Z %d: called by office worker\n", ++(*action_counter), customer.CustomerID);
    sem_post(mutex);

    sem_post(customer_done);
    sem_wait(employee_done);
    sem_wait(mutex);
    fprintf(file, "%d: Z %d: Odchadza domov\n", ++(*action_counter), customer.CustomerID);
    sem_post(mutex);
    exit(EXIT_SUCCESS);
}

void employee(int EmployeeID, int breaktime){
    fprintf(file, "%d: U %d: started\n", ++(*action_counter), EmployeeID);

    while(true){
        
        if (post_office == false){
            fprintf(file, "%d: U %d: Konci s robotou\n", ++(*action_counter), EmployeeID);
            break;
        }
        fprintf(file, "%d: U %d: Caka na zakaznika\n", ++(*action_counter), EmployeeID);
        sem_wait(customer_ready);
        
        sem_post(employee_ready);


        sem_wait(customer_done);
        sem_wait(mutex);
        fprintf(file, "%d: U %d: Vybavuje postarske zalezitosti\n", ++(*action_counter), EmployeeID);
        sem_post(mutex);
        sem_post(employee_done);
    }
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
        fprintf(file, "%d: posta sa zatvara o %d\n", ++(*action_counter), kokot);
        sleep(kokot);
        fprintf(file, "%d: posta sa zatvorila\n", ++(*action_counter));
        post_office = false;
        exit(EXIT_SUCCESS);
    }

    for (int i = 0; i < EmployeeCount; i++){
        pid_t pid_e_id = fork();
        if (pid_e_id == 0){
            int breaktime = rand() % EmployeeMaxBreak;
            employee(i+1, breaktime);
        }
    }

    for (int i = 0; i < CustomerCount; i++){
        pid_t pid_c_id = fork();
        if (pid_c_id == 0){
            
            Customer customer;
            customer.CustomerID = i+1;
            customer.CustomerWaitTime = CustomerMaxWait;
            customer.CustomerDemand = rand() % 3;
            customer_func(customer, CustomerCount);
        }
    }

    

    
    
    
    while( wait(NULL) > 0);
    printf("counter je %d\n", (*action_counter));
    cleanup();

    (void) MaxTime;
    return EXIT_SUCCESS;
}