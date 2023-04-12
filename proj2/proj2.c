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
#include <pthread.h>
#include <limits.h>

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

sem_t *mutex;
FILE *file;

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

void semaphore_init (){
    mutex = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED , -1, 0);
    if (mutex == MAP_FAILED){
        fprintf(stderr, "Error creating semaphore\n");
        exit(EXIT_FAILURE);
    }

    if (sem_init(mutex, 1, 1) == -1){
        fprintf(stderr, "Error creating semaphore\n");
        exit(EXIT_FAILURE);
    }
}

void semaphore_cleanup(){
    sem_destroy(mutex);
    munmap(mutex, sizeof(sem_t));
}

void employee(int EmployeeCount, int EmployeeMaxBreak){
    for (int i = 0; i < EmployeeCount; i++){
        pid_t pid = fork();
        if (pid == 0){
            // employee
            int time = rand() % EmployeeMaxBreak;
            usleep(time);
            exit(EXIT_SUCCESS);
        }
    }
}

struct Customer{
    int CustomerID;
    int CustomerMaxWait;
    int CustomerDemand;
};

int main(int argc, char *argv[]) {


    semaphore_init();
    // checks if arguments are in correct format
    if (argcheck(argc, argv) == EXIT_FAILURE){
        return EXIT_FAILURE;
    }
    int CustomerCount = atoi(argv[1]);
    int EmployeeCount = atoi(argv[2]);
    int CustomerMaxWait = atoi(argv[3]);
    int EmployeeMaxBreak = atoi(argv[4]);
    int MaxTime = atoi(argv[5]);

    // sets buffer to NULL
    setbuf(file, NULL);

    // prints arguments to file
    if (write_to_file(CustomerCount, EmployeeCount, CustomerMaxWait, EmployeeMaxBreak, MaxTime) == EXIT_FAILURE){
        return EXIT_FAILURE;
    }
    

    while( wait(NULL) > 0);
    semaphore_cleanup();

    return EXIT_SUCCESS;
}